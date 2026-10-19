/* MIT license. Integration with the exact, unmodified H700 v1.22 binary.
 * Its existing SDL3 mapping and ndsp master-volume APIs are called in process.
 * All symbol offsets are gated by GNU build ID and segment permissions.
 * No executable instructions are patched and no other process is accessed.
 */
#define _GNU_SOURCE
#include "rgds_runtime.h"
#include <link.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <time.h>

static const unsigned char expected_id[8] = {0x4e,0xbf,0xbd,0xc1,0x5c,0x1c,0xc0,0x2b};
static uintptr_t executable_base;
static int enabled, initialized;
static int level = 2;
static float audio_gain = 0.35f;
static int input_fds[16], input_count;
static const char *volume_path = "/sys/class/anbernic_misc/openbor_volume";
static uint64_t next_poll;

typedef struct { unsigned char data[16]; } Guid;
static uint32_t *(*get_joysticks)(int *);
static const char *(*joystick_name)(uint32_t);
static Guid (*joystick_guid)(uint32_t);
static void (*guid_string)(Guid, char *, int);
static int (*add_mapping)(const char *);
static void (*sdl_free)(void *);
static void (*master_volume)(float);

static int segment_contains(const struct dl_phdr_info *info, uintptr_t offset, size_t size, int flags) {
    for (unsigned i=0; i<info->dlpi_phnum; ++i) {
        const ElfW(Phdr) *p=&info->dlpi_phdr[i];
        if (p->p_type==PT_LOAD && (p->p_flags & flags)==(unsigned)flags &&
            offset>=p->p_vaddr && offset+size<=p->p_vaddr+p->p_memsz) return 1;
    }
    return 0;
}

static int find_executable(struct dl_phdr_info *info, size_t size, void *data) {
    (void)size; (void)data;
    for (unsigned i=0; i<info->dlpi_phnum; ++i) {
        const ElfW(Phdr) *p=&info->dlpi_phdr[i];
        if (p->p_type!=PT_NOTE) continue;
        const unsigned char *at=(const unsigned char *)(info->dlpi_addr+p->p_vaddr);
        const unsigned char *end=at+p->p_memsz;
        while ((size_t)(end-at)>=sizeof(ElfW(Nhdr))) {
            const ElfW(Nhdr) *note=(const ElfW(Nhdr) *)at;
            size_t namesize=(note->n_namesz+3u)&~3u, descsize=(note->n_descsz+3u)&~3u;
            if (namesize>(size_t)(end-at) || descsize>(size_t)(end-at) ||
                sizeof(*note)+namesize+descsize>(size_t)(end-at)) break;
            const unsigned char *name=at+sizeof(*note), *desc=name+namesize;
            if (note->n_type==NT_GNU_BUILD_ID && note->n_namesz==4 &&
                !memcmp(name,"GNU",4) && note->n_descsz==sizeof(expected_id) &&
                !memcmp(desc,expected_id,sizeof(expected_id))) {
                const uintptr_t functions[]={0x432ec0,0x432dc0,0x432d70,0x432320,0x431720,0x435740,0x423130};
                for (unsigned f=0; f<sizeof(functions)/sizeof(functions[0]); ++f)
                    if (!segment_contains(info,functions[f],16,PF_R|PF_X)) return 0;
                executable_base=info->dlpi_addr;
                enabled=1; return 1;
            }
            at+=sizeof(*note)+namesize+descsize;
        }
    }
    return 0;
}

static int read_level(int *value) {
    FILE *f=fopen(volume_path,"r"); if (!f) return 0;
    int v=-1, ok=fscanf(f,"%d",&v)==1; fclose(f);
    if (!ok || v<0 || v>10) return 0;
    *value=v; return 1;
}
static void apply_level(int v, int persist) {
    if (v<0) v=0;
    if (v>10) v=10;
    if (persist) {
        FILE *f=fopen(volume_path,"w");
        if (f) { fprintf(f,"%d\n",v); fclose(f); }
    }
    level=v;
    float fraction=(float)level/10.0f;
    float gain=audio_gain*fraction*fraction;
    master_volume(gain);
    fprintf(stderr,"[rgds] volume=%d/10 gain=%.5f (quadratic, ceiling=%.2f)\n",level,gain,audio_gain);
}
static void open_volume_keys(void) {
    for (int i=0; i<64 && input_count<16; ++i) {
        char path[64],name[256]="";
        snprintf(path,sizeof path,"/dev/input/event%d",i);
        int fd=open(path,O_RDONLY|O_NONBLOCK|O_CLOEXEC);
        if (fd<0) continue;
        unsigned char bits[(KEY_MAX+8)/8]={0};
        ioctl(fd,EVIOCGNAME(sizeof name),name);
        if (ioctl(fd,EVIOCGBIT(EV_KEY,sizeof bits),bits)>=0 &&
            ((bits[KEY_VOLUMEUP/8]>>(KEY_VOLUMEUP%8))&1) &&
            ((bits[KEY_VOLUMEDOWN/8]>>(KEY_VOLUMEDOWN%8))&1)) {
            input_fds[input_count++]=fd;
            fprintf(stderr,"[rgds] volume keys %s (%s), no exclusive grab\n",path,name);
        } else close(fd);
    }
}
static void install_mapping(void) {
    int count=0;
    uint32_t *ids=get_joysticks(&count);
    for (int i=0; ids && i<count; ++i) {
        const char *name=joystick_name(ids[i]);
        if (!name || !strstr(name,"ANBERNIC-rk3568-keys")) continue;
        char guid[33], mapping[1024];
        guid_string(joystick_guid(ids[i]),guid,sizeof guid);
        /* RK3568 Stock raw indices from the device owner's original mapping.
         * Nintendo positions: SDL south=B, east=A, north=X, west=Y. */
        snprintf(mapping,sizeof mapping,
            "%s,ANBERNIC-rk3568-keys,a:b1,b:b0,x:b3,y:b2,back:b8,guide:b6,start:b7,"
            "leftstick:b9,rightstick:b12,leftshoulder:b4,rightshoulder:b5,"
            "dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,"
            "leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b10,righttrigger:b11,platform:Linux,",guid);
        int result=add_mapping(mapping);
        fprintf(stderr,"[rgds] Nintendo controller mapping result=%d GUID=%s\n",result,guid);
        fprintf(stderr,"[rgds] mapping: %s\n",mapping);
    }
    if (ids) sdl_free(ids);
}

void rgds_runtime_init(void) {
    if (initialized) return;
    initialized=1;
    dl_iterate_phdr(find_executable,NULL);
    if (!enabled) { fprintf(stderr,"[rgds] binary build ID differs; game hooks disabled (probe is unaffected)\n"); return; }
    get_joysticks=(void *)(executable_base+0x432ec0);
    joystick_name=(void *)(executable_base+0x432dc0);
    joystick_guid=(void *)(executable_base+0x432d70);
    guid_string=(void *)(executable_base+0x432320);
    add_mapping=(void *)(executable_base+0x431720);
    sdl_free=(void *)(executable_base+0x435740);
    master_volume=(void *)(executable_base+0x423130);
    fprintf(stderr,"[rgds] verified H700 v1.22 build ID; using existing SDL3 and ndsp APIs\n");
    install_mapping();
    const char *gain_setting=getenv("MKW_AUDIO_GAIN");
    if (gain_setting && *gain_setting) {
        char *end=NULL;
        float requested=strtof(gain_setting,&end);
        if (end && !*end && requested>=0.01f && requested<=1.0f) audio_gain=requested;
        else fprintf(stderr,"[rgds] invalid MKW_AUDIO_GAIN; using 0.35\n");
    }
    const char *override=getenv("MKW_VOLUME_PATH");
    if (override && *override) volume_path=override;
    int initial=2;
    if (!read_level(&initial)) fprintf(stderr,"[rgds] Stock volume unavailable; starting at 2/10\n");
    apply_level(initial,0);
    open_volume_keys();
}
void rgds_runtime_tick(void) {
    if (!enabled) return;
    for (int i=0; i<input_count; ++i) {
        struct input_event events[32]; ssize_t bytes;
        while ((bytes=read(input_fds[i],events,sizeof events))>0) {
            for (int n=0; n<bytes/(ssize_t)sizeof(events[0]); ++n) {
                const struct input_event *e=&events[n];
                if (e->type==EV_KEY && e->value>0 &&
                    (e->code==KEY_VOLUMEUP || e->code==KEY_VOLUMEDOWN)) {
                    int observed;
                    /* If firmware already changed the level, follow it once
                     * instead of applying a second increment for the same key. */
                    if (read_level(&observed) && observed!=level) apply_level(observed,0);
                    else apply_level(level+(e->code==KEY_VOLUMEUP ? 1 : -1),1);
                }
            }
        }
    }
    struct timespec now; clock_gettime(CLOCK_MONOTONIC,&now);
    uint64_t ms=(uint64_t)now.tv_sec*1000+now.tv_nsec/1000000;
    if (ms>=next_poll) {
        int observed;
        if (read_level(&observed) && observed!=level) apply_level(observed,0);
        next_poll=ms+200;
    }
}

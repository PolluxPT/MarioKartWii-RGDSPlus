#define _GNU_SOURCE
#include "rgds_runtime.c"
#include <assert.h>
#include <math.h>

static float last_gain;
static int mappings;
static void test_master(float v) { last_gain=v; }
static uint32_t *test_ids(int *n) { *n=1; uint32_t *ids=malloc(sizeof(*ids)); ids[0]=42; return ids; }
static const char *test_name(uint32_t id) { assert(id==42); return "ANBERNIC-rk3568-keys"; }
static Guid test_guid(uint32_t id) { assert(id==42); Guid g={{0}}; return g; }
static void test_guidstr(Guid g, char *out, int size) { (void)g; assert(size==33); strcpy(out,"1900b655010000000100000000010000"); }
static int test_map(const char *mapping) {
    assert(strstr(mapping,"a:b1,b:b0,x:b3,y:b2,"));
    assert(strstr(mapping,"back:b8,guide:b6,start:b7,"));
    assert(strstr(mapping,"leftshoulder:b4,rightshoulder:b5,"));
    assert(strstr(mapping,"lefttrigger:b10,righttrigger:b11,"));
    mappings++; return 1;
}
static void emit_key(int fd, int code, int value) {
    struct input_event e={0}; e.type=EV_KEY; e.code=code; e.value=value;
    assert(write(fd,&e,sizeof e)==sizeof e);
}
int main(void) {
    master_volume=test_master;
    get_joysticks=test_ids; joystick_name=test_name; joystick_guid=test_guid;
    guid_string=test_guidstr; add_mapping=test_map; sdl_free=free;
    install_mapping(); assert(mappings==1);
    char path[]="/tmp/mkw-volume-test-XXXXXX";
    int file=mkstemp(path); assert(file>=0); assert(write(file,"0\n",2)==2); close(file);
    volume_path=path; enabled=1;
    int v; assert(read_level(&v) && v==0); apply_level(v,0); assert(last_gain==0);
    int fds[2]; assert(pipe(fds)==0); fcntl(fds[0],F_SETFL,O_NONBLOCK);
    input_fds[0]=fds[0]; input_count=1;
    emit_key(fds[1],KEY_VOLUMEUP,1); rgds_runtime_tick(); assert(level==1 && fabsf(last_gain-.0035f)<.00001f);
    emit_key(fds[1],KEY_VOLUMEUP,0); rgds_runtime_tick(); assert(level==1);
    emit_key(fds[1],KEY_VOLUMEDOWN,1); rgds_runtime_tick(); assert(level==0 && last_gain==0);
    emit_key(fds[1],KEY_VOLUMEDOWN,2); rgds_runtime_tick(); assert(level==0);
    apply_level(10,1); emit_key(fds[1],KEY_VOLUMEUP,1); rgds_runtime_tick(); assert(level==10 && fabsf(last_gain-.35f)<.00001f);
    for (int n=0; n<=10; ++n) {
        apply_level(n,1);
        float expected=.35f*(float)n*(float)n/100.f;
        assert(fabsf(last_gain-expected)<.00001f);
    }
    /* Firmware updates first: the same key must not cause a second increment. */
    apply_level(4,1); FILE *f=fopen(path,"w"); fputs("5\n",f); fclose(f);
    emit_key(fds[1],KEY_VOLUMEUP,1); rgds_runtime_tick(); assert(level==5);
    close(fds[0]); close(fds[1]); unlink(path);

    union { max_align_t a; unsigned char bytes[64]; } note_memory={0};
    ElfW(Nhdr) *note=(ElfW(Nhdr) *)note_memory.bytes;
    note->n_namesz=4; note->n_descsz=8; note->n_type=NT_GNU_BUILD_ID;
    memcpy(note_memory.bytes+sizeof(*note),"GNU",4);
    memcpy(note_memory.bytes+sizeof(*note)+4,expected_id,8);
    ElfW(Phdr) ph[2]={{.p_type=PT_NOTE,.p_vaddr=0,.p_memsz=24},
                     {.p_type=PT_LOAD,.p_vaddr=0x400000,.p_memsz=0x100000,.p_flags=PF_R|PF_X}};
    struct dl_phdr_info info={.dlpi_addr=(uintptr_t)note_memory.bytes,.dlpi_phdr=ph,.dlpi_phnum=2};
    enabled=0; assert(find_executable(&info,sizeof info,NULL)==1 && enabled);
    note_memory.bytes[sizeof(*note)+4]^=1;
    enabled=0; assert(find_executable(&info,sizeof info,NULL)==0 && !enabled);
    note_memory.bytes[sizeof(*note)+4]^=1;
    ph[1].p_flags=PF_R; enabled=0;
    assert(find_executable(&info,sizeof info,NULL)==0 && !enabled);
    puts("PASS: GUID mapping, volume keys/mute/bounds/firmware coordination, executable ID and executable-segment guards");
}

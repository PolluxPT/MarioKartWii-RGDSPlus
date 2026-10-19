/* MIT license. Read-only Linux controller capability report. */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <string.h>
#define BIT(a,n) (((a)[(n)/8] >> ((n)%8)) & 1)
int main(void) {
    for (int i=0;i<64;i++) {
        char path[64], name[256]="unknown";
        snprintf(path,sizeof path,"/dev/input/event%d",i);
        int fd=open(path,O_RDONLY|O_NONBLOCK); if(fd<0) continue;
        struct input_id id={0}; unsigned char keys[(KEY_MAX+8)/8]={0}, axes[(ABS_MAX+8)/8]={0};
        ioctl(fd,EVIOCGNAME(sizeof name),name); ioctl(fd,EVIOCGID,&id);
        ioctl(fd,EVIOCGBIT(EV_KEY,sizeof keys),keys); ioctl(fd,EVIOCGBIT(EV_ABS,sizeof axes),axes);
        if(BIT(keys,BTN_GAMEPAD)||BIT(keys,BTN_JOYSTICK)||BIT(keys,BTN_START)||BIT(axes,ABS_X)) {
            printf("[input] %s %s bus=%04x vendor=%04x product=%04x version=%04x\n",path,name,id.bustype,id.vendor,id.product,id.version);
            printf("[input] keys:"); for(int k=0;k<=KEY_MAX;k++) if(BIT(keys,k)) printf(" %d",k); puts("");
            printf("[input] axes:"); for(int k=0;k<=ABS_MAX;k++) if(BIT(axes,k)) printf(" %d",k); puts("");
        }
        close(fd);
    }
    return 0;
}

/* MIT license. A six-second EGL/GLES3 smoke test; no game data required. */
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#define LOAD(name) __typeof__(&name) p_##name = dlsym(lib, #name); if (!p_##name) { fprintf(stderr,"[probe] missing %s\n",#name); return 2; }
int main(void) {
    setvbuf(stdout, NULL, _IOLBF, 0);
    const char *path = getenv("MKW_EGL_LIBRARY");
    if (!path) { fprintf(stderr,"[probe] MKW_EGL_LIBRARY unset\n"); return 2; }
    void *lib = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
    if (!lib) { fprintf(stderr,"[probe] dlopen: %s\n",dlerror()); return 2; }
    LOAD(eglGetDisplay); LOAD(eglInitialize); LOAD(eglChooseConfig);
    LOAD(eglBindAPI); LOAD(eglCreateWindowSurface); LOAD(eglCreateContext);
    LOAD(eglMakeCurrent); LOAD(eglSwapBuffers); LOAD(eglDestroyContext);
    LOAD(eglDestroySurface); LOAD(eglTerminate); LOAD(eglGetError);
    LOAD(eglGetProcAddress); LOAD(eglQueryString);
    EGLDisplay d = p_eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGLint major, minor, n;
    if (d == EGL_NO_DISPLAY || !p_eglInitialize(d, &major, &minor)) goto failed;
    printf("[probe] EGL %d.%d vendor=%s\n",major,minor,p_eglQueryString(d,EGL_VENDOR));
    const EGLint cfg[] = {EGL_SURFACE_TYPE,EGL_WINDOW_BIT,EGL_RENDERABLE_TYPE,0x40,EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_NONE};
    EGLConfig c;
    if (!p_eglChooseConfig(d,cfg,&c,1,&n) || n < 1) goto failed;
    if (!p_eglBindAPI(EGL_OPENGL_ES_API)) goto failed;
    uint16_t size[] = {640,480};
    EGLSurface s = p_eglCreateWindowSurface(d,c,(EGLNativeWindowType)size,NULL);
    if (s == EGL_NO_SURFACE) goto failed;
    const EGLint ca[] = {EGL_CONTEXT_CLIENT_VERSION,3,EGL_NONE};
    EGLContext ctx = p_eglCreateContext(d,c,EGL_NO_CONTEXT,ca);
    if (ctx == EGL_NO_CONTEXT || !p_eglMakeCurrent(d,s,s,ctx)) goto failed;
    __typeof__(&glGetString) get_string = (__typeof__(&glGetString))p_eglGetProcAddress("glGetString");
    __typeof__(&glClearColor) clear_color = (__typeof__(&glClearColor))p_eglGetProcAddress("glClearColor");
    __typeof__(&glClear) clear = (__typeof__(&glClear))p_eglGetProcAddress("glClear");
    if (!get_string || !clear_color || !clear) goto failed;
    printf("[probe] GL vendor=%s renderer=%s version=%s\n",get_string(GL_VENDOR),get_string(GL_RENDERER),get_string(GL_VERSION));
    for (int i=0;i<3;i++) {
        clear_color(i==0?0.8f:0.05f,i==1?0.8f:0.05f,i==2?0.8f:0.05f,1);
        clear(GL_COLOR_BUFFER_BIT);
        if (!p_eglSwapBuffers(d,s)) goto failed;
        sleep(2);
    }
    puts("[probe] SUCCESS: red, green, blue displayed; verify visually on RGDS+");
    p_eglMakeCurrent(d,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
    p_eglDestroyContext(d,ctx); p_eglDestroySurface(d,s); p_eglTerminate(d);
    return 0;
failed:
    fprintf(stderr,"[probe] FAILED: EGL error=0x%x\n",p_eglGetError());
    return 3;
}

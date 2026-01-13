#include <unistd.h>
#include <dlfcn.h>
#include <thread>
#include "log.h"
#include "xdl.h"
#include "dobby.h"
#include "il2cpp-resolver.h"
#include "il2cpp-hook.h"
#include "menu.h"

void hack_start() {
    LOGI("hack started!");
    while (true) {
        auto handle = xdl_open("libil2cpp.so", 0);
        if (handle) {
            il2cpp_api_init(handle);
            il2cpp_hook();
            break;
        }
        usleep(100000);
    }
    auto libEGL = xdl_open("libEGL.so", XDL_TRY_FORCE_LOAD);
    auto eglSwapBuffers = xdl_sym(libEGL, "eglSwapBuffers", nullptr);
    DobbyHook(eglSwapBuffers, (void*)h_eglSwapBuffers, (void**)&o_eglSwapBuffers);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {

    std::thread(hack_start).detach();

    auto handle = dlopen("libmain_real.so", RTLD_LAZY);
    if (!handle) {
        return JNI_VERSION_1_6;
    }

    auto o_JNI_OnLoad = (jint(*)(JavaVM*, void*))dlsym(handle, "JNI_OnLoad");
    if (o_JNI_OnLoad) return o_JNI_OnLoad(vm, reserved);

    return JNI_VERSION_1_6;
}

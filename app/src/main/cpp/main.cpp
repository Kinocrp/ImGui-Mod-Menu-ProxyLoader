#include <unistd.h>
#include <dlfcn.h>
#include <thread>
#include <string>
#include <jni.h>
#include "log.h"
#include "xdl.h"
#include "dobby.h"
#include "il2cpp-resolver.h"
#include "il2cpp-dumper.h"
#include "il2cpp-hook.h"
#include "menu.h"

std::string GetAppDataDir(JavaVM *vm) {
    JNIEnv *env = nullptr;
    vm->AttachCurrentThread(&env, nullptr);
    jclass activity_thread_clz = env->FindClass("android/app/ActivityThread");
    if (activity_thread_clz != nullptr) {
        jmethodID currentApplicationId = env->GetStaticMethodID(activity_thread_clz, "currentApplication", "()Landroid/app/Application;");
        if (currentApplicationId) {
            jobject application = env->CallStaticObjectMethod(activity_thread_clz, currentApplicationId);
            if (application) {
                jclass application_clazz = env->GetObjectClass(application);
                jmethodID getExternalFilesDir = env->GetMethodID(application_clazz, "getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;");
                if (getExternalFilesDir) {
                    jobject fileObj = env->CallObjectMethod(application, getExternalFilesDir, nullptr);
                    if (fileObj) {
                        jclass fileClass = env->GetObjectClass(fileObj);
                        jmethodID getAbsolutePathMethod = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
                        jstring pathString = (jstring)env->CallObjectMethod(fileObj, getAbsolutePathMethod);
                        const char *pathChars = env->GetStringUTFChars(pathString, nullptr);
                        std::string result(pathChars);
                        LOGI("data dir: %s", result.c_str());
                        env->ReleaseStringUTFChars(pathString, pathChars);
                        return result;
                    } else {
                        LOGE("getExternalFilesDir returned null");
                    }
                } else {
                    LOGE("getExternalFilesDir method not found");
                }
            } else {
                LOGE("currentApplication returned null");
            }
        } else {
            LOGE("currentApplication static method not found");
        }
    } else {
        LOGE("ActivityThread class not found");
    }
    return {};
}

void hack_start(JavaVM *vm) {
    LOGI("hack started!");
    while (true) {
        auto handle = xdl_open("libil2cpp.so", 0);
        if (handle) {
            il2cpp_api_init(handle);
            il2cpp_dump(GetAppDataDir(vm).c_str());
            il2cpp_hook();
            break;
        }
        usleep(100000);
    }
    auto libEGL = xdl_open("libEGL.so", 0);
    auto eglSwapBuffers = xdl_sym(libEGL, "eglSwapBuffers", nullptr);
    DobbyHook(eglSwapBuffers, (void*)h_eglSwapBuffers, (void**)&o_eglSwapBuffers);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {

    std::thread hack_thread(hack_start, vm);
    hack_thread.detach();

    auto handle = dlopen("libmain_real.so", RTLD_NOW | RTLD_GLOBAL);
    if (!handle) return JNI_VERSION_1_6;

    auto o_JNI_OnLoad = (jint(*)(JavaVM*, void*))dlsym(handle, "JNI_OnLoad");
    if (o_JNI_OnLoad) return o_JNI_OnLoad(vm, reserved);

    return JNI_VERSION_1_6;
}

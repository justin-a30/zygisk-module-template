#include <jni.h>
#include <android/log.h>
#include "zygisk.hpp"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "WaveCharge", __VA_ARGS__)

class WaveCharge : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;

        // Hook the methods
        hookSupportWaveChargeAnimation();
        hookUpdateWaveHeight();
    }

private:
    Api *api;
    JNIEnv *env;

    void hookSupportWaveChargeAnimation() {
        JNINativeMethod methods[] = {
                {"supportWaveChargeAnimation", "()Z", (void *) supportWaveChargeAnimation}
        };
        api->hookJniNativeMethods(env, "com/android/keyguard/charge/ChargeUtils", methods, 1);
    }

    void hookUpdateWaveHeight() {
        JNINativeMethod methods[] = {
                {"updateWaveHeight", "()V", (void *) updateWaveHeight}
        };
        api->hookJniNativeMethods(env, "com/android/keyguard/charge/wave/WaveView", methods, 1);
    }

    static jboolean supportWaveChargeAnimation(JNIEnv *env, jobject thiz) {
        jclass cls = env->FindClass("java/lang/Throwable");
        jobject throwable = env->NewObject(cls, env->GetMethodID(cls, "<init>", "()V"));
        jobjectArray stackTrace = (jobjectArray) env->CallObjectMethod(throwable, env->GetMethodID(cls, "getStackTrace", "()[Ljava/lang/StackTraceElement;"));

        jsize length = env->GetArrayLength(stackTrace);
        for (jsize i = 0; i < length; ++i) {
            jobject element = env->GetObjectArrayElement(stackTrace, i);
            jclass elementClass = env->GetObjectClass(element);
            jmethodID getClassName = env->GetMethodID(elementClass, "getClassName", "()Ljava/lang/String;");
            jstring className = (jstring) env->CallObjectMethod(element, getClassName);

            const char *classNameCStr = env->GetStringUTFChars(className, nullptr);
            if (strcmp(classNameCStr, "com.android.keyguard.charge.ChargeUtils") == 0 ||
                strcmp(classNameCStr, "com.android.keyguard.charge.container.MiuiChargeContainerView") == 0) {
                env->ReleaseStringUTFChars(className, classNameCStr);
                return JNI_TRUE;
            }
            env->ReleaseStringUTFChars(className, classNameCStr);
        }
        return JNI_FALSE;
    }

    static void updateWaveHeight(JNIEnv *env, jobject thiz) {
        jclass cls = env->GetObjectClass(thiz);
        jfieldID fieldID = env->GetFieldID(cls, "mWaveXOffset", "I");
        env->SetIntField(thiz, fieldID, 0);
    }
};

REGISTER_ZYGISK_MODULE(WaveCharge)
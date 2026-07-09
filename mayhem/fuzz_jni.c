#include <jni.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static JavaVM *g_vm;
static jclass g_cls;
static jmethodID g_mid;

static int init_jvm(void) {
    if (g_vm) {
        return 0;
    }
    JavaVMInitArgs args;
    JavaVMOption opts[5];
    char ld_path[512];
    char jvm_args[256];
    char classpath[1024];

    const char *tmpdir = getenv("TMPDIR");
    if (!tmpdir || !*tmpdir) {
        tmpdir = "/tmp";
    }
    snprintf(jvm_args, sizeof(jvm_args), "-Djava.io.tmpdir=%s", tmpdir);
    snprintf(classpath, sizeof(classpath),
             "-Djava.class.path=/mayhem/xmlpull-app.jar:/mayhem/kxml2.jar:/mayhem");
    opts[0].optionString = jvm_args;
    opts[1].optionString = classpath;
    opts[2].optionString = "-Xmx2048m";
    opts[3].optionString = "-Djdk.attach.allowAttachSelf=true";
    opts[4].optionString = (char *)"-XX:+IgnoreUnrecognizedVMOptions";

    args.version = JNI_VERSION_1_8;
    args.nOptions = 5;
    args.options = opts;
    args.ignoreUnrecognized = JNI_TRUE;

    snprintf(ld_path, sizeof(ld_path),
             "LD_LIBRARY_PATH=/usr/lib/jvm/java-21-openjdk-amd64/lib/server:/mayhem");
    putenv(ld_path);

    JNIEnv *env = NULL;
    if (JNI_CreateJavaVM(&g_vm, (void **)&env, &args) != 0) {
        return -1;
    }

    jclass local = (*env)->FindClass(env, "PullParserFactoryFuzzer");
    if (!local) {
        return -1;
    }
    g_cls = (jclass)(*env)->NewGlobalRef(env, local);
    (*env)->DeleteLocalRef(env, local);
    g_mid = (*env)->GetStaticMethodID(env, g_cls, "fuzzerTestOneInput", "([B)V");
    return g_mid ? 0 : -1;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > (size_t)INT_MAX) {
        return 0;
    }
    if (init_jvm() != 0) {
        return 0;
    }

    JNIEnv *env = NULL;
    if ((*g_vm)->AttachCurrentThread(g_vm, (void **)&env, NULL) != 0) {
        return 0;
    }

    jbyteArray arr = (*env)->NewByteArray(env, (jsize)size);
    if (!arr) {
        return 0;
    }
    (*env)->SetByteArrayRegion(env, arr, 0, (jsize)size, (const jbyte *)data);
    (*env)->CallStaticVoidMethod(env, g_cls, g_mid, arr);
    (*env)->DeleteLocalRef(env, arr);

    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
    }
    return 0;
}

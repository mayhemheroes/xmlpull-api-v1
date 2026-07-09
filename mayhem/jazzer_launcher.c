#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef TARGET_CLASS
#define TARGET_CLASS "PullParserFactoryFuzzer"
#endif

int main(int argc, char **argv) {
    const char *driver = "/mayhem/jazzer_driver";
    const char *agent = "--agent_path=/mayhem/jazzer_agent_deploy.jar";
    const char *cp = "--cp=/mayhem/xmlpull-app.jar:/mayhem/kxml2.jar:/mayhem";
    char target_class[128];
    snprintf(target_class, sizeof(target_class), "--target_class=%s", TARGET_CLASS);
    /* Mayhem coverage runs with a read-only rootfs; only /dev/shm is writable.
       Jazzer/ByteBuddy needs a temp dir for agent attach — point JVM + TMPDIR there. */
    const char *jvm_args =
        "--jvm_args=-Xmx2048m:-Djava.io.tmpdir=/dev/shm:-Djdk.attach.allowAttachSelf=true";

    char ld_path[512];
    snprintf(ld_path, sizeof(ld_path),
             "LD_LIBRARY_PATH=/usr/lib/jvm/java-21-openjdk-amd64/lib/server:/mayhem");
    putenv(ld_path);
    putenv((char *)"TMPDIR=/dev/shm");
    putenv((char *)"HOME=/dev/shm");
    putenv((char *)"JAVA_TOOL_OPTIONS=-Djava.io.tmpdir=/dev/shm");

    char **nv = calloc((size_t)argc + 8, sizeof(char *));
    if (!nv) {
        return 1;
    }
    int i = 0;
    nv[i++] = (char *)driver;
    nv[i++] = (char *)agent;
    nv[i++] = (char *)cp;
    nv[i++] = target_class;
    nv[i++] = (char *)jvm_args;
    for (int j = 1; j < argc; j++) {
        nv[i++] = argv[j];
    }
    nv[i] = NULL;
    execv(driver, nv);
    perror("execv jazzer_driver");
    free(nv);
    return 127;
}

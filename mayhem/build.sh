#!/usr/bin/env bash
#
# mayhem/build.sh — build xmlpull-api-v1 JNI libFuzzer harness + API jar.
set -euo pipefail

[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH

: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}"
: "${MAYHEM_JOBS:=$(nproc)}"
export SANITIZER_FLAGS DEBUG_FLAGS CC CXX MAYHEM_JOBS

cd "$SRC"
OUT="$SRC"
JNI_INC="$JAVA_HOME/include"
JNI_LINUX="$JAVA_HOME/include/linux"
JVM_LIB="$JAVA_HOME/lib/server"

build_fuzz_project() {
  echo "=== building xmlpull jar for fuzzing ==="
  if [ ! -f "$OUT/xmlpull-app.jar" ]; then
    mkdir -p build/classes build/meta/META-INF/services
    javac -source 8 -target 8 -encoding ISO-8859-1 -d build/classes \
      $(find src/java/api -name '*.java')
    printf '%s\n' 'org.kxml2.io.KXmlParser,org.kxml2.io.KXmlSerializer' \
      > build/meta/META-INF/services/org.xmlpull.v1.XmlPullParserFactory
    jar cf "$OUT/xmlpull-app.jar" -C build/classes . -C build/meta META-INF
  fi
  if [ ! -f "$OUT/kxml2.jar" ]; then
    cp /opt/toolchains/kxml2/kxml2.jar "$OUT/kxml2.jar"
  fi
}

build_fuzzers() {
  echo "=== building JNI libFuzzer harnesses ==="
  local build_cp="$OUT/xmlpull-app.jar:$OUT/kxml2.jar"
  local jvm_link="-L$JVM_LIB -ljvm -Wl,-rpath,$JVM_LIB"
  local jni_flags="-I$JNI_INC -I$JNI_LINUX"
  local src_c="$SRC/mayhem/fuzz_jni.c $SRC/mayhem/asan_defaults.c"

  for fuzzer in "$SRC"/mayhem/*Fuzzer.java; do
    [ -f "$fuzzer" ] || continue
    local fuzzer_basename
    fuzzer_basename=$(basename -s .java "$fuzzer")
    local out_bin="$OUT/${fuzzer_basename}"
    local out_standalone="$OUT/${fuzzer_basename}-standalone"
    local standalone_o="$OUT/${fuzzer_basename}_standalone_driver.o"

    if [ ! -f "$OUT/${fuzzer_basename}.class" ]; then
      javac -cp "$build_cp" -d "$OUT" "$fuzzer"
    fi

    if [ ! -f "$standalone_o" ]; then
      $CC -c "$STANDALONE_FUZZ_MAIN" -o "$standalone_o"
    fi

    if [ ! -x "$out_bin" ]; then
      # shellcheck disable=SC2086
      $CC $SANITIZER_FLAGS $DEBUG_FLAGS -O1 $jni_flags \
        $src_c -o "$out_bin" \
        $LIB_FUZZING_ENGINE $jvm_link
    fi

    if [ ! -x "$out_standalone" ]; then
      # shellcheck disable=SC2086
      $CC $SANITIZER_FLAGS $DEBUG_FLAGS -O1 $jni_flags \
        $src_c "$standalone_o" \
        -o "$out_standalone" $jvm_link
    fi
  done
}

build_fuzz_project
build_fuzzers

echo "build.sh complete"

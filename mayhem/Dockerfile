# syntax=docker/dockerfile:1.7
FROM ghcr.io/mayhemheroes/base:20260621

ARG COMMIT
ARG REPO_URL
ARG SOURCE_DATE_EPOCH
ARG SANITIZER_FLAGS
ARG COVERAGE_FLAGS=
ARG MAYHEM_JOBS=

ARG MAVEN_VERSION=3.9.9
ARG JAZZER_COMMIT=86378b7a20f08165e72a45582d6a9a5091212318
ARG BAZELISK_VERSION=1.25.0

USER root
RUN apt-get update && apt-get install -yq --no-install-recommends \
      openjdk-21-jdk-headless ca-certificates curl git unzip \
    && rm -rf /var/lib/apt/lists/*

RUN install -d /opt/toolchains/m2/repository /opt/toolchains/jazzer /opt/toolchains/kxml2 \
 && curl -fsSL -o /opt/toolchains/kxml2/kxml2.jar \
      https://repo1.maven.org/maven2/net/sf/kxml/kxml2/2.3.0/kxml2-2.3.0.jar \
 && curl -fsSL "https://archive.apache.org/dist/maven/maven-3/${MAVEN_VERSION}/binaries/apache-maven-${MAVEN_VERSION}-bin.tar.gz" \
    | tar -xz -C /opt/toolchains \
 && ln -sfn "/opt/toolchains/apache-maven-${MAVEN_VERSION}" /opt/toolchains/maven \
 && curl -fsSL -o /usr/local/bin/bazelisk \
      "https://github.com/bazelbuild/bazelisk/releases/download/v${BAZELISK_VERSION}/bazelisk-linux-amd64" \
 && chmod +x /usr/local/bin/bazelisk \
 && git clone https://github.com/CodeIntelligenceTesting/jazzer /opt/toolchains/jazzer-src \
 && cd /opt/toolchains/jazzer-src \
 && git checkout "${JAZZER_COMMIT}" \
 && echo 'build --java_runtime_version=local_jdk_21' >> .bazelrc \
 && echo 'build --cxxopt=-stdlib=libc++' >> .bazelrc \
 && echo 'build --linkopt=-lc++' >> .bazelrc \
 && USE_BAZEL_VERSION=7.4.0 bazelisk build \
      //src/main/java/com/code_intelligence/jazzer:jazzer_standalone_deploy.jar \
      //deploy:jazzer-api \
      //launcher:jazzer \
 && cp "$(USE_BAZEL_VERSION=7.4.0 bazelisk cquery --output=files //launcher:jazzer)" /opt/toolchains/jazzer/jazzer_driver \
 && cp "$(USE_BAZEL_VERSION=7.4.0 bazelisk cquery --output=files //src/main/java/com/code_intelligence/jazzer:jazzer_standalone_deploy.jar)" /opt/toolchains/jazzer/jazzer_agent_deploy.jar \
 && cp "$(USE_BAZEL_VERSION=7.4.0 bazelisk cquery --output=files //deploy:jazzer-api)" /opt/toolchains/jazzer/jazzer_api_deploy.jar \
 && chmod +x /opt/toolchains/jazzer/jazzer_driver \
 && chown -R 2000:2000 /opt/toolchains/m2 /opt/toolchains/jazzer /opt/toolchains/kxml2 /opt/toolchains/maven

ENV JAVA_HOME=/usr/lib/jvm/java-21-openjdk-amd64 \
    JVM_LD_LIBRARY_PATH=/usr/lib/jvm/java-21-openjdk-amd64/lib/server \
    JAZZER_API_PATH=/opt/toolchains/jazzer/jazzer_api_deploy.jar \
    MVN=/opt/toolchains/maven/bin/mvn \
    M2_HOME=/opt/toolchains/maven \
    MAVEN_OPTS=-Dmaven.repo.local=/opt/toolchains/m2/repository \
    PATH=/usr/lib/jvm/java-21-openjdk-amd64/bin:/opt/toolchains/maven/bin:${PATH}

USER mayhem
WORKDIR /mayhem
COPY --chown=2000:2000 . /mayhem

RUN bash mayhem/build.sh

RUN if [ -f mayhem/test.sh ]; then bash mayhem/test.sh; else echo "no mayhem/test.sh — repo ships no functional tests"; fi

LABEL org.opencontainers.image.source="${REPO_URL}" \
      org.opencontainers.image.revision="${COMMIT}" \
      rlenv.tier="commit" \
      rlenv.source_date_epoch="${SOURCE_DATE_EPOCH}" \
      rlenv.user="mayhem"

ENV TMPDIR=/dev/shm \
    HOME=/dev/shm \
    JAVA_TOOL_OPTIONS=-Djava.io.tmpdir=/dev/shm
USER mayhem
WORKDIR /mayhem

# =============================================================================
# diametercodec multi-stage Dockerfile
# =============================================================================
# Single file replacing the inherited chain: ubuntu -> diametercodec_builder
# All dependency versions are declared as ARGs here (single source of truth).
#
# Stages:
#   deps      - All third-party libraries compiled and installed
#   build     - Project compilation (library + examples)
#   unit-test - Lightweight image for running unit tests
#
# Usage:
#   docker build --target deps      -t diametercodec_builder .
#   docker build --target build     -t diametercodec_build .
#   docker build --target unit-test -t diametercodec_ut .
#
# The build.sh script handles all of this automatically.
# =============================================================================

FROM ubuntu:24.04 AS deps
LABEL maintainer="testillano"
LABEL testillano.diametercodec_builder.description="Docker image with all dependencies to build ert_diametercodec library"

WORKDIR /code/build

# ---------------------------------------------------------------------------
# Dependency versions (single source of truth)
# ---------------------------------------------------------------------------
ARG make_procs=4
ARG build_type=Release

ARG ert_logger_ver=v1.1.1
ARG nlohmann_json_ver=v3.12.0
ARG pboettch_jsonschemavalidator_ver=2.4.0
ARG jupp0r_prometheuscpp_ver=v1.3.0
ARG civetweb_civetweb_ver=v1.16
ARG ert_metrics_ver=v1.3.0
ARG google_test_ver=v1.11.0

# ---------------------------------------------------------------------------
# System packages
# ---------------------------------------------------------------------------
RUN apt-get update && apt-get install -y \
    wget tar \
    make cmake g++ \
    libssl-dev zlib1g-dev libcurl4-openssl-dev \
    doxygen graphviz \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# ===========================================================================
# ERT_LOGGER
# ===========================================================================
RUN set -x && \
    wget https://github.com/testillano/logger/archive/${ert_logger_ver}.tar.gz && \
    tar xvf ${ert_logger_ver}.tar.gz && cd logger-*/ && \
    cmake -DERT_LOGGER_BuildExamples=OFF -DCMAKE_BUILD_TYPE=${build_type} . && \
    make -j${make_procs} && make install && \
    cd .. && rm -rf * && \
    set +x

# ===========================================================================
# NLOHMANN JSON
# ===========================================================================
RUN set -x && \
    wget https://github.com/nlohmann/json/archive/refs/tags/${nlohmann_json_ver}.tar.gz && \
    tar xvf ${nlohmann_json_ver}.tar.gz && cd json-*/ && mkdir build && cd build && \
    cmake -DJSON_BuildTests=OFF -DCMAKE_POLICY_VERSION_MINIMUM=3.5 .. && \
    make -j${make_procs} install && \
    cd ../.. && rm -rf * && \
    set +x

# ===========================================================================
# PBOETTCH JSON-SCHEMA-VALIDATOR
# ===========================================================================
RUN set -x && \
    wget https://github.com/pboettch/json-schema-validator/archive/${pboettch_jsonschemavalidator_ver}.tar.gz && \
    tar xvf ${pboettch_jsonschemavalidator_ver}.tar.gz && cd json-schema-validator*/ && mkdir build && cd build && \
    cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 .. && \
    make -j${make_procs} && make install && \
    cd ../.. && rm -rf * && \
    set +x

# ===========================================================================
# PROMETHEUS-CPP + CIVETWEB
# ===========================================================================
RUN set -x && \
    wget https://github.com/jupp0r/prometheus-cpp/archive/refs/tags/${jupp0r_prometheuscpp_ver}.tar.gz && \
    tar xvf ${jupp0r_prometheuscpp_ver}.tar.gz && cd prometheus-cpp*/3rdparty && \
    wget https://github.com/civetweb/civetweb/archive/refs/tags/${civetweb_civetweb_ver}.tar.gz && \
    tar xvf ${civetweb_civetweb_ver}.tar.gz && mv civetweb-*/* civetweb && cd .. && \
    mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=${build_type} -DENABLE_TESTING=OFF .. && \
    make -j${make_procs} && make install && \
    cd ../.. && rm -rf * && \
    set +x

# ===========================================================================
# ERT_METRICS
# ===========================================================================
RUN set -x && \
    wget https://github.com/testillano/metrics/archive/${ert_metrics_ver}.tar.gz && \
    tar xvf ${ert_metrics_ver}.tar.gz && cd metrics-*/ && \
    cmake -DERT_METRICS_BuildExamples=OFF -DCMAKE_BUILD_TYPE=${build_type} . && \
    make -j${make_procs} && make install && \
    cd .. && rm -rf * && \
    set +x

# ===========================================================================
# GOOGLE TEST FRAMEWORK
# ===========================================================================
RUN set -x && \
    wget https://github.com/google/googletest/archive/refs/tags/release-$(echo ${google_test_ver} | cut -c2-).tar.gz && \
    tar xvf release-$(echo ${google_test_ver} | cut -c2-).tar.gz && cd googletest-release*/ && \
    cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 . && make -j${make_procs} install && \
    cd .. && rm -rf * && \
    set +x

# ---------------------------------------------------------------------------
# Builder entrypoint (cmake + make wrapper)
# ---------------------------------------------------------------------------
COPY deps/build.sh /var/build.sh
RUN chmod a+x /var/build.sh

ENTRYPOINT ["/var/build.sh"]
CMD []

# =============================================================================
# Stage: build (compile diametercodec project)
# =============================================================================
FROM deps AS build

ARG make_procs=4
ARG build_type=Release

COPY . /code
WORKDIR /code

RUN cmake -DCMAKE_BUILD_TYPE=${build_type} . && make -j${make_procs}

# =============================================================================
# Stage: unit-test (lightweight image for running tests)
# =============================================================================
FROM ubuntu:24.04 AS unit-test

ARG build_type=Release

COPY --from=build /code/build/${build_type}/bin/unit-test /opt/unit-test

ENTRYPOINT ["/opt/unit-test"]
CMD []

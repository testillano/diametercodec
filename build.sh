#!/bin/bash
# =============================================================================
# diametercodec build script (flat multi-stage model)
# =============================================================================
# All dependency versions are declared in Dockerfile as ARGs.
# This script reads them as defaults and exposes them as --build-arg overrides.
#
# Commands:
#   --builder:   Build deps stage only (builder image with all libraries)
#   --image:     Full build: deps + compile + unit-test image
#   (no args):   builds everything (--image).
#
# Environment variables (override defaults):
#   All ARG names from Dockerfile can be set as env vars, e.g.:
#     nlohmann_json_ver=v3.12.0 ./build.sh --image
#
# Other variables:
#   DBUILD_XTRA_OPTS: extra docker build options (e.g., --no-cache)
# =============================================================================

set -e

#############
# VARIABLES #
#############
SCR="$(readlink -f "$0")"
SCR_DIR="$(dirname "${SCR}")"
cd "${SCR_DIR}"

DOCKERFILE=Dockerfile
registry=ghcr.io/testillano

# Parse version defaults from Dockerfile (single source of truth)
parse_arg() {
  grep "^ARG ${1}=" "${DOCKERFILE}" | head -1 | cut -d= -f2
}

# Defaults from Dockerfile
make_procs__dflt=$(grep processor /proc/cpuinfo -c)
build_type__dflt=$(parse_arg build_type)
ert_logger_ver__dflt=$(parse_arg ert_logger_ver)
nlohmann_json_ver__dflt=$(parse_arg nlohmann_json_ver)
pboettch_jsonschemavalidator_ver__dflt=$(parse_arg pboettch_jsonschemavalidator_ver)
google_test_ver__dflt=$(parse_arg google_test_ver)

image_tag__dflt=latest

#############
# FUNCTIONS #
#############
usage() {
  cat << EOF

  Usage: $0 [--builder|--image]

         (no args):   builds everything (--image).
         --builder:   builds deps stage (builder image with all libraries).
         --image:     full build: deps + compile + unit-test image.

         Environment variables (override any version):

           image_tag, make_procs, build_type, ert_logger_ver,
           nlohmann_json_ver, pboettch_jsonschemavalidator_ver,
           google_test_ver

         Other variables:

           DBUILD_XTRA_OPTS: extra docker build options (e.g., --no-cache)

         Examples:

           $0
           nlohmann_json_ver=v3.12.0 $0 --image
           DBUILD_XTRA_OPTS=--no-cache $0

EOF
}

# Resolve variable: use env value if set, otherwise use __dflt
resolve() {
  local var=$1
  local val="${!var}"
  if [ -z "${val}" ]; then
    val="$(eval echo \$${var}__dflt)"
  fi
  echo "${val}"
}

build_builder() {
  echo
  echo "=== Build diametercodec_builder (deps stage) ==="
  echo

  local tag=$(resolve image_tag)
  local bargs=""
  bargs+=" --build-arg make_procs=$(resolve make_procs)"
  bargs+=" --build-arg build_type=$(resolve build_type)"
  bargs+=" --build-arg ert_logger_ver=$(resolve ert_logger_ver)"
  bargs+=" --build-arg nlohmann_json_ver=$(resolve nlohmann_json_ver)"
  bargs+=" --build-arg pboettch_jsonschemavalidator_ver=$(resolve pboettch_jsonschemavalidator_ver)"
  bargs+=" --build-arg google_test_ver=$(resolve google_test_ver)"

  docker build --target deps \
    -t ${registry}/diametercodec_builder:${tag} \
    ${bargs} ${DBUILD_XTRA_OPTS} .

  echo
  echo "Built: ${registry}/diametercodec_builder:${tag}"
}

build_image() {
  echo
  echo "=== Build diametercodec_ut (unit-test image) ==="
  echo

  local tag=$(resolve image_tag)
  local bargs=""
  bargs+=" --build-arg make_procs=$(resolve make_procs)"
  bargs+=" --build-arg build_type=$(resolve build_type)"
  bargs+=" --build-arg ert_logger_ver=$(resolve ert_logger_ver)"
  bargs+=" --build-arg nlohmann_json_ver=$(resolve nlohmann_json_ver)"
  bargs+=" --build-arg pboettch_jsonschemavalidator_ver=$(resolve pboettch_jsonschemavalidator_ver)"
  bargs+=" --build-arg google_test_ver=$(resolve google_test_ver)"

  docker build --target unit-test \
    -t ${registry}/diametercodec_ut:${tag} \
    ${bargs} ${DBUILD_XTRA_OPTS} .

  echo
  echo "Built: ${registry}/diametercodec_ut:${tag}"
}

########
# MAIN #
########
case "$1" in
  --builder)
    build_builder
    ;;
  --image)
    build_image
    ;;
  -h|--help)
    usage
    exit 0
    ;;
  "")
    build_image
    ;;
  *)
    echo "Unknown option: $1"
    usage
    exit 1
    ;;
esac

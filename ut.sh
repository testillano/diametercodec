#!/bin/bash

# Narrow test selection by mean '--gtest_filter', i.e.:
#
# $ ./ut.sh --gtest_list_tests # to list the available tests
# $ ./ut.sh --gtest_filter=CodecAvp_test.* # to filter and run 1 specific suite

DIAMETERCODEC_UT_IMAGE=${DIAMETERCODEC_UT_IMAGE:-ghcr.io/testillano/diametercodec_ut:latest}

# Build unit-test image if not available:
if ! docker image inspect ${DIAMETERCODEC_UT_IMAGE} &>/dev/null; then
  echo "Building unit-test image..."
  docker build --target unit-test -t ${DIAMETERCODEC_UT_IMAGE} .
fi

docker run --rm -it ${DIAMETERCODEC_UT_IMAGE} $@

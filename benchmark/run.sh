#!/bin/bash

# Run the diametercodec micro-benchmark from its Docker image.
#
# Arguments are passed straight to the benchmark binary, e.g.:
#
# $ ./benchmark/run.sh                      # defaults (100000 iters x 5 repeats)
# $ ./benchmark/run.sh 1000000 --repeats 7  # custom iterations/repeats
# $ ./benchmark/run.sh --jsonl              # one JSON line to stdout (redirect it yourself)
#
# Nothing is written to disk: capture stdout if you want to keep the results.

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

DIAMETERCODEC_BENCHMARK_IMAGE=${DIAMETERCODEC_BENCHMARK_IMAGE:-ghcr.io/testillano/diametercodec_benchmark:latest}

# Build benchmark image if not available:
if ! docker image inspect ${DIAMETERCODEC_BENCHMARK_IMAGE} &>/dev/null; then
  echo "Building benchmark image..."
  docker build --target benchmark -t ${DIAMETERCODEC_BENCHMARK_IMAGE} "${PROJECT_DIR}"
fi

# Interactive TTY only when attached to a terminal (keeps piping/redirects clean).
TTY=""
[ -t 0 ] && [ -t 1 ] && TTY="-it"

docker run --rm ${TTY} ${DIAMETERCODEC_BENCHMARK_IMAGE} "$@"

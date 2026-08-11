# diametercodec benchmark

Reference micro-benchmark for the **codec hot path**: per-message `encode` and
`decode` (plus the JSON conversions `toJson` / `fromJson` and a full
`roundtrip`).

It is a throughput/latency probe, not a correctness test (correctness lives in
the unit-test suite under `ut/`).

## What it measures (and what it does not)

The dictionary is **parsed once at startup and kept in memory**, exactly as the
consuming applications do (e.g. `h2diagent` loads every stack at boot and, for
each Diameter message of a given application-id, just retrieves the already
built dictionary by reference). Re-parsing per message would be extremely
penalizing on multistack setups, so it is never on the hot path.

Accordingly, the dictionary is loaded **once, outside** the timed loops, and
every timed operation reuses it by const reference. The reported numbers are
pure `encode`/`decode` (and JSON) cost.

The sample message is a base-protocol `Capabilities-Exchange-Request` (command
257), covering the common AVP formats: `DiameterIdentity`, `Address`,
`Unsigned32` and `UTF8String`.

## Run

The benchmark ships as its own Docker image (built from the `benchmark` stage,
just like the unit-test image). `run.sh` builds it on first use and runs it,
passing any arguments straight to the binary:

```bash
./benchmark/run.sh                      # defaults: 100000 iterations x 5 repeats
./benchmark/run.sh 1000000 --repeats 7  # custom iterations / repeats
./benchmark/run.sh --jsonl              # emit one JSON line to stdout
```

Build the image explicitly (or to refresh it after code changes) with the
project build script:

```bash
./build.sh --benchmark                  # builds ghcr.io/testillano/diametercodec_benchmark
```

Override the image with the `DIAMETERCODEC_BENCHMARK_IMAGE` env var. The
underlying build (deps, native compilation, etc.) is the standard project build
described in the top-level `README.md`; the benchmark is compiled by the same
multi-stage `Dockerfile`.

Human-readable output:

```
ert_diametercodec micro-benchmark
  build      : Release
  iterations : 100000 x 5 repeats
  message    : Capabilities-Exchange-Request (9 AVPs, 188 wire bytes)

  operation  | median ns/op |    min ns/op |    throughput
  -----------+--------------+--------------+---------------
  encode     |       ...    |       ...    |       ... ops/s
  decode     |       ...    |       ...    |       ... ops/s
  ...
```

For each operation the tool reports the **median** and **minimum** ns/op across
the repeats (the minimum is usually the most stable figure; the median resists
occasional scheduler/turbo outliers).

## JSONL output (optional)

With `--jsonl` the tool prints a single machine-readable line to **stdout** (it
never writes any file, so nothing gets trapped inside the container). Redirect
it yourself if you want to keep a record, and tag it with the commit as you see
fit:

```bash
./benchmark/run.sh 1000000 --repeats 7 --jsonl >> my-results.jsonl
```

The line includes `build_type`, `cpu_model`, `nproc`, run parameters and, per
operation, `{median_ns, min_ns, ops_per_s}`. Raw ns/op are only comparable on
the same hardware and build type.

## Notes for stable numbers

- Use `Release` builds (`Debug` enables `-O0` and coverage instrumentation).
- Keep the machine otherwise idle; pin to a core with `taskset -c 0` if needed.
- Logging level is forced to `Warning` so tracing does not skew the codec cost.

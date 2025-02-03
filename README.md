# C++ diametercodec library

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Documentation](https://codedocs.xyz/testillano/diametercodec.svg)](https://codedocs.xyz/testillano/diametercodec/index.html)
[![Coverage Status](https://img.shields.io/endpoint?url=https://testillano.github.io/diametercodec/badge.json)](https://testillano.github.io/diametercodec)
[![Ask Me Anything !](https://img.shields.io/badge/Ask%20me-anything-1abc9c.svg)](https://github.com/testillano)
[![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://github.com/testillano/diametercodec/graphs/commit-activity)
[![Main project workflow](https://github.com/testillano/diametercodec/actions/workflows/ci.yml/badge.svg)](https://github.com/testillano/diametercodec/actions/workflows/ci.yml)

This library encodes/decodes diameter messages based in Diameter Protocol (RFC 6733).

## Project image

This image is already available at `github container registry` and `docker hub` for every repository `tag`, and also for master as `latest`:

```bash
$ docker pull ghcr.io/testillano/diametercodec:<tag>
```

You could also build it using the script `./build.sh` located at project root:


```bash
$ ./build.sh --project-image
```

This image is built with `./Dockerfile`.

## Usage

To run compilation over this image, just run with `docker`. The `entrypoint` (check it at `./deps/build.sh`) will fall back from `cmake` (looking for `CMakeLists.txt` file at project root, i.e. mounted on working directory `/code` to generate makefiles) to `make`, in order to build your source code. There are two available environment variables used by the builder script of this image: `BUILD_TYPE` (for `cmake`) and `MAKE_PROCS` (for `make`):

```bash
$ envs="-e MAKE_PROCS=$(grep processor /proc/cpuinfo -c) -e BUILD_TYPE=Release"
$ docker run --rm -it -u $(id -u):$(id -g) ${envs} -v ${PWD}:/code -w /code \
         ghcr.io/testillano/diametercodec:<tag>
```

## Build project with docker

### Builder image

This image is already available at `github container registry` and `docker hub` for every repository `tag`, and also for master as `latest`:

```bash
$ docker pull ghcr.io/testillano/diametercodec_builder:<tag>
```

You could also build it using the script `./build.sh` located at project root:


```bash
$ ./build.sh --builder-image
```

This image is built with `./Dockerfile.build`.

### Usage

Builder image is used to build the project library. To run compilation over this image, again, just run with `docker`:

```bash
$ envs="-e MAKE_PROCS=$(grep processor /proc/cpuinfo -c) -e BUILD_TYPE=Release"
$ docker run --rm -it -u $(id -u):$(id -g) ${envs} -v ${PWD}:/code -w /code \
         ghcr.io/testillano/diametercodec_builder:<tag>
```

You could generate documentation passing extra arguments to the [entry point](https://github.com/testillano/diametercodec/blob/master/deps/build.sh) behind:

```bash
$ docker run --rm -it -u $(id -u):$(id -g) ${envs} -v ${PWD}:/code -w /code \
         ghcr.io/testillano/diametercodec_builder::<tag>-build "" doc
```

You could also build the library using the script `./build.sh` located at project root:


```bash
$ ./build.sh --project
```

## Build project natively

This is a cmake-based building library, so you may install cmake:

```bash
$ sudo apt-get install cmake
```

And then generate the makefiles from project root directory:

```bash
$ cmake .
```

You could specify type of build, 'Debug' or 'Release', for example:

```bash
$ cmake -DCMAKE_BUILD_TYPE=Debug .
$ cmake -DCMAKE_BUILD_TYPE=Release .
```

You could also change the compilers used:

```bash
$ cmake -DCMAKE_CXX_COMPILER=/usr/bin/g++     -DCMAKE_C_COMPILER=/usr/bin/gcc
```
or

```bash
$ cmake -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_C_COMPILER=/usr/bin/clang
```

### Build

```bash
$ make
```

### Clean

```bash
$ make clean
```

### Documentation

```bash
$ make doc
```

```bash
$ cd docs/doxygen
$ tree -L 1
     .
     ├── Doxyfile
     ├── html
     ├── latex
     └── man
```

### Install

```bash
$ sudo make install
```

Optionally you could specify another prefix for installation:

```bash
$ cmake -DMY_OWN_INSTALL_PREFIX=$HOME/mylibs/ert_diametercodec
$ make install
```

### Uninstall

```bash
$ cat install_manifest.txt | sudo xargs rm
```

## Testing

### Unit test

Check the badge above to know the current coverage level.
You can execute it after project building, for example for `Release` target:

```bash
$> build/Release/bin/unit-test # native executable
- or -
$> docker run -it --rm -v ${PWD}/build/Release/bin/unit-test:/ut --entrypoint "/ut" ghcr.io/testillano/diametercodec:latest # docker
```

To shortcut docker run execution, `./ut.sh` script at root directory can also be used.
You may provide extra arguments to Google test executable, for example:

```bash
$> ./ut.sh --gtest_list_tests # to list the available tests
$> ./ut.sh --gtest_filter=Avp_test.getId # to filter and run 1 specific test
$> ./ut.sh --gtest_filter=Avp_test.* # to filter and run 1 specific suite
etc.
```

#### Coverage

Unit test coverage could be easily calculated executing the script `./tools/coverage.sh`. This script builds and runs an image based in `./Dockerfile.coverage` which uses the `lcov` utility behind. Finally, a `firefox` instance is launched showing the coverage report where you could navigate the source tree to check the current status of the project. This stage is also executed as part of `h2agent` continuous integration (`github workflow`).

Both `ubuntu` and `alpine` base images are supported, but the official image uploaded is the one based in `ubuntu`.
If you want to work with alpine-based images, you may build everything from scratch, including all docker base images which are project dependencies.

## Integration

### CMake

#### Embedded

##### Replication

To embed the library directly into an existing CMake project, place the entire source tree in a subdirectory and call `add_subdirectory()` in your `CMakeLists.txt` file:

```cmake
add_subdirectory(ert_diametercodec)
...
add_library(foo ...)
...
target_link_libraries(foo PRIVATE ert_diametercodec::ert_diametercodec)
```

##### FetchContent

Since CMake v3.11,
[FetchContent](https://cmake.org/cmake/help/v3.11/module/FetchContent.html) can be used to automatically download the repository as a dependency at configure type.

Example:

```cmake
include(FetchContent)

FetchContent_Declare(ert_diametercodec
  GIT_REPOSITORY https://github.com/testillano/diametercodec.git
  GIT_TAG vx.y.z)

FetchContent_GetProperties(ert_diametercodec)
if(NOT ert_json_POPULATED)
  FetchContent_Populate(ert_diametercodec)
  add_subdirectory(${ert_diametercodec_SOURCE_DIR} ${ert_diametercodec_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

target_link_libraries(foo PRIVATE ert_diametercodec::ert_diametercodec)
```

## Examples

Examples are packaged on project image, so you could run by mean:

```bash
$> docker run --rm -it  --entrypoint "/opt/printHardcodedStacks" ghcr.io/testillano/diametercodec:latest
-or-
...
```

## Contributing

Please, execute `astyle` formatting (using [frankwolf image](https://hub.docker.com/r/frankwolf/astyle)) before any pull request:

```bash
$ sources=$(find . -name "*.hpp" -o -name "*.cpp")
$ docker run -i --rm -v $PWD:/data frankwolf/astyle ${sources}
```


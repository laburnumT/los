# LOS (Laburnum Operating System)

## Build

### Prerequisites

- cmake
- Cross compiler toolchain for target architecture that supports c23 at minimum

One must pass the specific toolchain file to cmake using `-DCMAKE_TOOLCHAIN_FILE=`

An optimised version can be built by passing the `-DCMAKE_BUILD_TYPE=OPTIMISED`
option to `cmake`.

Example workflow to create an x86 optimised build:

```sh
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake_tc/i386.cmake -DCMAKE_BUILD_TYPE=OPTIMISED ..
make
```

## Running

The easiest way to run los is to use the script `qemu.sh` in the scripts
directory.

If an ISO is desired, `iso.sh` can be found in the same directory.

## Supported platforms

### X86

Currently the only supported platform is x86.

It has been tested to run on:

- Thinkpad T43
- Thinkpad T60
- qemu-system-i386

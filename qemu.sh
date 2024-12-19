#!/bin/bash

set -euo pipefail

DEBUG_FLAGS=""

main() {
        . ./build.sh
        "qemu-system-$(./target-triplet-to-arch.sh ${HOST})" \
                -kernel kernel/myos.kernel \
                ${DEBUG_FLAGS}
}

while [[ "${#}" -gt 0 ]]; do
        case "${1}" in
                -d | --debug)
                        shift
                        DEBUG_FLAGS="-s -S"
                        ;;
        esac
done

main

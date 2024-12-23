#!/bin/bash

set -euo pipefail

EXTRA_FLAGS=""
DEBUG_FLAGS=""
MODE="-kernel kernel/los.kernel"

main() {
        . ./build.sh
        "qemu-system-$(./target-triplet-to-arch.sh ${HOST})" \
                ${MODE} \
                ${EXTRA_FLAGS} \
                ${DEBUG_FLAGS}
}

while [[ "${#}" -gt 0 ]]; do
        case "${1}" in
                -d | --debug)
                        shift
                        DEBUG_FLAGS="${DEBUG_FLAGS} -s -S"
                        ;;
                --iso)
                        shift
                        . ./iso.sh
                        MODE="-cdrom los.iso"
                        ;;
                --int)
                        shift
                        DEBUG_FLAGS="${DEBUG_FLAGS} -d int"
                        ;;
                -m)
                        shift
                        EXTRA_FLAGS="${EXTRA_FLAGS} -m ${1}"
                        shift
                        ;;
                *)
                        echo "Unknown flag" >&2
                        exit 1
        esac
done

main

#!/bin/bash

set -euo pipefail

EXTRA_FLAGS=""
DEBUG_FLAGS=""
MODE="-kernel kernel/los.kernel"

main() {
        "qemu-system-i386" \
                ${MODE} \
                ${EXTRA_FLAGS} \
                ${DEBUG_FLAGS}
}

help_func() {
        cat <<EOF
options:
  -d, --debug   Freeze cpu at startup and open debug port on 1234
  --iso         Make and use iso for LOS
  --int         Log interrupts
  -m            Set available memory
EOF
}

while [[ "${#}" -gt 0 ]]; do
        case "${1}" in
                -d | --debug)
                        shift
                        DEBUG_FLAGS="${DEBUG_FLAGS} -s -S"
                        ;;
                --iso)
                        shift
                        scripts/iso.sh
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
                -h | --help)
                        help_func
                        exit 1
                        ;;
                *)
                        echo "Unknown flag" >&2
                        exit 1
                        ;;
        esac
done

main

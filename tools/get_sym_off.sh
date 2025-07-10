#!/usr/bin/bash
set -euo pipefail

if [[ $# -ne 2 ]]; then
    echo "$0 <executable> <symbol>" >&2
    exit 1
fi

EX="$1"
SYM="$2"

TEXT_ADDR_HEX=$(readelf -W -S "$EX" | awk '$3 == ".text" {print "0x"$5; exit}')
[[ -n $TEXT_ADDR_HEX ]] || { echo "error: failed to find .text section" >&2; exit 1; }

SYM_ADDR_HEX=$(readelf -W -s "$EX" | awk '$8 == "'"$SYM"'" {print "0x"$2; exit}')
[[ -n $SYM_ADDR_HEX ]] || { echo "error: failed to find symbol" >&2; exit 1; }

TEXT_ADDR=$((TEXT_ADDR_HEX))
SYM_ADDR=$((SYM_ADDR_HEX))
OFF=$((SYM_ADDR - TEXT_ADDR))

echo "$OFF"

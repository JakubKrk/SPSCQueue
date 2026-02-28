#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

shopt -s nullglob
dirs=(*build*/)

if [ ${#dirs[@]} -eq 0 ]; then
    echo "No build directories found."
    exit 0
fi

echo "Removing:"
for d in "${dirs[@]}"; do
    echo "  ${d}"
    rm -rf "${d}"
done

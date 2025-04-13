#!/bin/bash

# Clean build
make clean && make build

exec 2> napaka.txt  # Redirect stderr to napaka.txt

# Stop on first command error (also write to napaka.txt automatically)
set -e

# Check binaries
[[ -x elf_changer && -x hello ]] || { echo "Missing binaries" >&2; exit 1; }

# Test 1: Check header output
output=$(./elf_changer -h hello)
if echo "$output" | grep -q -e "Error opening file" \
                            -e "Error getting file stats" \
                            -e "File too small to be a valid ELF file" \
                            -e "Error mapping file" \
                            -e "Not a valid ELF file"; then
    echo "Test 1 failed: Detected an error when parsing ELF file" >&2
    exit 1
else
    echo "Test 1 passed"
fi

# Test 2: List ELF layout
output=$(./elf_changer -l hello)
if echo "$output" | grep -q -e "Error opening file" \
                            -e "Error getting file stats" \
                            -e "File too small to be a valid ELF file" \
                            -e "Error mapping file" \
                            -e "Not a valid ELF file"; then
    echo "Test 2 failed: Detected an error when parsing ELF file" >&2
    exit 1
else if echo $output | grep -q -e "Symbol table not found in the ELF file" \
                               -e "String table not found in the ELF file"; then
    echo "Test 2 failed: Unable to find symbol/string table" >&2
    exit 1
else
    echo "Test 2 passed"
fi

# Test 3: Modify ELF
output=$(./elf_changer -c hello g1 g2)
if echo "$output" | grep -q -e "Error opening file" \
                            -e "Error getting file stats" \
                            -e "File too small to be a valid ELF file" \
                            -e "Error mapping file" \
                            -e "Not a valid ELF file"; then
    echo "Test 3 failed: Detected an error when parsing ELF file" >&2
    exit 1
else if echo $output | grep -q -e ".data section not found" \
                               -e "Variable not found:"\
                               -e "is not a variable"\
                               -e "Unsupported variable size"; then
    echo "Test 3 failed: .data or variable error" >&2
    exit 1
else
    echo "Test 3 passed"
fi

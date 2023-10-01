#!/bin/bash
# run FROM PROJECT ROOT!

find '.' -type f '(' -name '*.h' -o -name '*.c' ')' -not -path "*/libs/*" | xargs clang-format -i --verbose

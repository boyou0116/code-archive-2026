#!/usr/bin/env bash
#
# Print the n-th Fibonacci number
#
# Uasge:
#   ./fib.sh <non-negative integer>
#
# Definition:
#   F(0) = 0
#   F(1) = 1
#   F(n) = F(n-1) + F(n-2)

n=$1

if [ -z "$n" ] || [ "$n" -lt 0 ]; then
    echo "Usage: $0 <non-negative integer>"
    exit 1
fi

a=0
b=1

if [ "$n" -le 1 ]; then
    echo "$n"
    exit 0
fi

for ((i=1; i<n; i++))
do
    temp=$b
    b=$((a + b))
    a=$temp
done

echo $b

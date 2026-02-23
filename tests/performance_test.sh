#!/usr/bin/env sh

PROGRAM="$@"
RUNS=5

if [ -z "$PROGRAM" ]; then
  echo "Usage: $0 ./program [arguments]"
  exit 1
fi

TOTAL=0

i=1
while [ $i -le $RUNS ]; do
  TIME=$(/usr/bin/time -f "%e" qemu-arm -L /usr/arm-linux-gnueabi $PROGRAM 2>&1 >/dev/null)
  TOTAL=$(echo "$TOTAL + $TIME" | bc -l)
  echo "Iterating over run: $i ($TIME)"
  i=$((i + 1))
done

AVG=$(echo "scale=6; $TOTAL / $RUNS" | bc -l)
echo "Average time ($RUNS iterations): $AVG s"

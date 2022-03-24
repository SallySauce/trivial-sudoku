#!/bin/bash

_err() { echo -e "$0: " "$@" >&2 ; exit 1; }

N=50

ITER=10
TIMEOUT=9999999

while [[ $# -gt 0 ]]; do case "$1" in
    -iter=*)       ITER=${1#*=}; shift ;;
    -timeout=*)    TIMEOUT=${1#*=}; shift ;;
    -N=*)          N=${1#*=}; shift ;;
    -*)            _err "未知选项: $1" ;;
esac; done

IN=tests/test$N
OUT=tests/answer$N

TMP=$(mktemp)
TOTAL_TIME=0.0

for i in $(seq $ITER); do

    WRONG_LINES=$({ time timeout $TIMEOUT diff -ab $OUT <(echo $IN | ./sudoku_solve) | wc -l; } 2>$TMP)
    r=$?
    [ $r -eq 124 ] && echo "Timeout" && exit
    [ $r -eq 2 ]   && echo "Not exists" && exit
    [ $r -ne 0 ]   && echo "sudoku_solve exited with non-zero code" && exit
    [ $WRONG_LINES -ne 0 ] && echo "answer is wrong" && exit

    TIME=$(cat $TMP | head -2 | tail -1 | awk -F' ' '{print $2}' | awk 'match($0, /([0-9]+)m([.0-9]+)s/, a) {print 60 * a[1] + a[2]}')

    TOTAL_TIME=$(echo $TOTAL_TIME $TIME | awk '{print $1 + $2}')

    echo -n '.'

done;

echo ''

AVG_TIME=$(echo $TOTAL_TIME $ITER | awk '{print 1000 * $1 / $2}')

rm $TMP

echo -e "\033[1;32m通过测试!!! 平均耗时 $AVG_TIME ms\033[0m"

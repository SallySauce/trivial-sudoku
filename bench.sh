#!/bin/bash

_err() { echo -e "$0: " "$@" >&2 ; exit 1; }

N=50
ITER=10
TIMEOUT=9999999

while [[ $# -gt 0 ]]; do case "$1" in
    -iter=*)       ITER=${1#*=};    shift ;;
    -timeout=*)    TIMEOUT=${1#*=}; shift ;;
    -N=*)          N=${1#*=};       shift ;;
    -*)            _err "未知选项: $1" ;;
esac; done

IN=tests/test$N
OUT=tests/answer$N

_run_benchmark() {
    local ITER=$1
    local IN=$2
    local OUT=$3

    TMP=$(mktemp)
    TOTAL_TIME=0.0

    for i in $(seq $ITER); do

        WRONG_LINES=$({ time diff -ab $OUT <(echo $IN | ./sudoku_solve) | wc -l; } 2>$TMP)
        r=$?
        [ $r -eq 2 ]   && echo "Not exists" && exit
        [ $r -ne 0 ]   && echo "sudoku_solve exited with non-zero code" && exit
        [ $WRONG_LINES -ne 0 ] && echo "answer is wrong" && exit

        TIME=$(cat $TMP | head -2 | tail -1 | gawk -F' ' '{print $2}' | gawk 'match($0, /([0-9]+)m([.0-9]+)s/, a) {print 60 * a[1] + a[2]}')

        TOTAL_TIME=$(echo $TOTAL_TIME $TIME | gawk '{print $1 + $2}')

        echo -n '.'

    done;

    echo ''

    AVG_TIME=$(echo $TOTAL_TIME $ITER | gawk '{print 1000 * $1 / $2}')

    rm $TMP

    echo -e "\033[1;32m通过测试!!! 平均耗时 $AVG_TIME ms\033[0m"
}


export -f _run_benchmark
timeout $TIMEOUT bash -c "_run_benchmark $ITER $IN $OUT"
[ $? -eq 124 ] && echo "" && _err "运行时间超过 $TIMEOUT 秒，自动退出"

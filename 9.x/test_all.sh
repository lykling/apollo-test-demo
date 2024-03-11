#!/bin/bash
# This script is a wrapper for test.sh. It runs test.sh with all cases

collect_cpu_stat() {
  tar -zcf cpu_stats.tar.gz cpu_stats
  rm -rf cpu_stats
  exit 0
}

dump_cpu_stats() {
  n=$1
  trap "collect_cpu_stat" SIGINT SIGTERM
  mkdir -p cpu_stats
  for ((i = 0; i < $n; ++i)); do
    cat /proc/stat >> cpu_stats/cpu_stat_$i
    sleep 1
  done
}

for freq in 20 50 100; do
  for size_mb in 1 4 16; do
    n=$(($freq * 60))
    size=$((1024 * 1024 * $size_mb))
    echo "Running test with n=$n, freq=$freq, size=$size using bytes"
    dump_cpu_stats 120 &
    dump_pid=$!
    { time ./test.sh $n $freq $size 1 > /dev/null; } 2> time.out
    kill -2 $dump_pid
    wait
    tar -czf case_${n}_${freq}_${freq}_${size_mb}M_1.tar.gz time.out cpu_stats.tar.gz n1.out n1.err n2.out n2.err n3.out n3.err
    echo "Running test with n=$n, freq=$freq, size=$size using repeated uint32"
    dump_cpu_stats 120 &
    dump_pid=$!
    { time ./test.sh $n $freq $size 0 > /dev/null; } 2> time.out
    kill -2 $dump_pid
    wait
    tar -czf case_${n}_${freq}_${freq}_${size_mb}M_0.tar.gz time.out cpu_stats.tar.gz n1.out n1.err n2.out n2.err n3.out n3.err
  done
done

tar -zcf result.tar.gz case_*.tar.gz

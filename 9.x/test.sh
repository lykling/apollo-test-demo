#!/bin/bash

n3 1> n3.out 2> n3.err &
pid_n3=$!

n2 1> n2.out 2> n2.err &
pid_n2=$!

n1 $@ 1> n1.out 2> n1.err

sleep 3
kill -2 $pid_n2 $pid_n3

wait $pid_n2
wait $pid_n3

cat n3.out

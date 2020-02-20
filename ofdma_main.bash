#!/bin/bash

./waf

for (( i=$1 ; i<=$2; i++ ))
do
    for (( j=0 ; j<$3 ; j++ ))
    do
        rng=$((i*$3+j))
        echo "$rng start"
        bash ofdma_loop.bash $rng 0 9 &
    done
    wait
done

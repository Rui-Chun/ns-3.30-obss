#!/bin/bash

./waf

for i in {0..3}
do
    for (( j=$1 ; j<=$2 ; j++ ))
    do
        bash ofdma_loopc.bash $i $j &
        bash ofdma_loopo.bash $i $j &
        bash ofdma_loopac.bash $i $j &
        bash ofdma_loopao.bash $i $j &
    done
    wait
    echo finished
done

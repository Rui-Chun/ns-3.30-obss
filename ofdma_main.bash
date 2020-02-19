#!/bin/bash

./waf

for i in {0..0}
do
    echo apsta
    for j in {$1..$2}
    do
        bash ofdma_loopc.bash $i $j &
        bash ofdma_loopo.bash $i $j &
        bash ofdma_loopac.bash $i $j &
        bash ofdma_loopao.bash $i $j
    done
done

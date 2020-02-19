#!/bin/bash

./waf

for i in {0..5}
do
    echo apsta
    for j in {0..14}
    do
        echo $((i*15+j))
        bash ofdma_loop.bash $((i*15+j)) &
    done
    wait

    echo adhoc
    for j in {0..14}
    do
        echo $((i*15+j))
        bash ofdma_loop1.bash $((i*15+j)) &
    done
    wait

done

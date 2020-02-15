#!/bin/bash

for i in {0..1}
do
    for j in {0..15}
    do
        echo $((i*16+j))
        bash ofdma_loop.bash $((i*16+j)) &
    done
    wait
done
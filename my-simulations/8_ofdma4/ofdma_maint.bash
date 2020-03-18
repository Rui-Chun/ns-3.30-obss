#!/bin/bash

for (( i=0 ; i<=$1 ; i=i+1))
do
    sleep 10
    bash ofdma_loopt.bash $2 $3 &
done

wait

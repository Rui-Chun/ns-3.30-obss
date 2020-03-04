#!/bin/bash

for (( i=0 ; i<=$1 ; i=i+1))
do
    bash ofdma_loopt.bash $2 $3 &
done

wait
#!/bin/bash

for (( i=$1 ; i<=$2 ; i=i+1))
do
    bash ofdma_loopt.bash $3 $4
done
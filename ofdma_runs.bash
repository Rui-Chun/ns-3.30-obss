#!/bin/bash

for (( i=1; i<=$1; i++))
do
    sleep 10
    bash ofdma_run.bash &
done
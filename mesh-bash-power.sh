#!/bin/bash

./waf

# udpr
for ((cnt=$1;cnt<=$2;cnt++))
do
    for ((pow=15;pow<=30;pow=pow+3))
    do
        if [ -f ./mesh-bash-power-udpr/${pow}-${cnt}.txt ]; then
            echo exist
        else
            ./waf --run "mesh-loc-jw-ac --RngSeed=${cnt} --txPower=${pow}" | tee ./mesh-bash-power-udpr/${pow}-${cnt}.txt
        fi
    done
done

rm minstrel-*.txt
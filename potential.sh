#!/bin/bash

./waf

route=$3
for ((cnt=$1;cnt<=$2;cnt++))
do
  for ((pow=3;pow<=5;pow=pow+1))
  do
    filename="./ece598hpn/${route}-${pow}-1-${cnt}.txt"
    if [ -f ${filename} ]; then
        echo exist
    else
        ./waf --run "simple_wifi --RngSeed=${cnt} --apNum=$((pow*pow))" | tee ${filename}
    fi
    filename="./ece598hpn/${route}-${pow}-2-${cnt}.txt"
    if [ -f ${filename} ]; then
        echo exist
    else
        ./waf --run "simple_wifi --RngSeed=${cnt} --apNum=$((pow*pow)) --gateways=0+$((pow*pow-1))" | tee ${filename}
    fi
  done
done
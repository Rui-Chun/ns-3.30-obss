#!/bin/bash

./waf

route=$3
for ((cnt=$1;cnt<=$2;cnt++))
do
  filename="./ece598hpn/gateway/${route}-2-${cnt}.txt"
  if [ -f ${filename} ]; then
      echo exist
  else
      ./waf --run "simple_wifi --RngSeed=${cnt} --apXSize=6 --apNum=36 --route=${route} --gateways=8+27" | tee ${filename}
  fi
  filename="./ece598hpn/gateway/${route}-3-${cnt}.txt"
  if [ -f ${filename} ]; then
      echo exist
  else
      ./waf --run "simple_wifi --RngSeed=${cnt} --apXSize=6 --apNum=36 --route=${route} --gateways=7+16+26" | tee ${filename}
  fi
  filename="./ece598hpn/gateway/${route}-4-${cnt}.txt"
  if [ -f ${filename} ]; then
      echo exist
  else
      ./waf --run "simple_wifi --RngSeed=${cnt} --apXSize=6 --apNum=36 --route=${route} --gateways=7+10+25+28" | tee ${filename}
  fi
  filename="./ece598hpn/gateway/${route}-5-${cnt}.txt"
  if [ -f ${filename} ]; then
      echo exist
  else
      ./waf --run "simple_wifi --RngSeed=${cnt} --apXSize=6 --apNum=36 --route=${route} --gateways=7+9+19+22+27" | tee ${filename}
  fi
  filename="./ece598hpn/gateway/${route}-6-${cnt}.txt"
  if [ -f ${filename} ]; then
      echo exist
  else
      ./waf --run "simple_wifi --RngSeed=${cnt} --apXSize=6 --apNum=36 --route=${route} --gateways=0+9+13+22+26+35" | tee ${filename}
  fi
done
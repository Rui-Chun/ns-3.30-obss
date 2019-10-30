#!/bin/bash

# ./waf

# udpr
for ((cnt=$1;cnt<=$2;cnt++))
do
  for ((grd=3;grd<=7;grd=grd+2))
  do
    apn=$((grd*grd))
    gtn=$((apn-1))
    gtn=$((gtn/2))
    echo $gtn
    # if [ -f ./mesh-bash-route/$3-$4-${grd}-${cnt}.txt ]; then
    #     echo exist
    # else
    #     ./waf --run "mesh-loc-jw-ac --route=$3 --app=$4 --RngSeed=${cnt} --gridSize=${grd} --apNum=${apn} --gateways=${gtn}" | tee ./mesh-bash-route/$3-$4-${grd}-${cnt}.txt
    # fi
  done
done

# rm minstrel-*.txt

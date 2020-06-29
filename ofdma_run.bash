#!/bin/bash

./waf

for k in {1..50..1}
do
for mcs in {3..5..1}
do
for note in "1" "2" "4"
do
for packetsize in "4e3"
do
    locationfile="locationtxt/locationFile${note}.txt"
    routingfile="locationtxt/routingFile${note}.txt"
    appfile="locationtxt/appFile.txt"
    folder="my-simulations/8_ofdma8/basic/topo${note}-mcs${mcs}-size${packetsize}"
    if [ ! -d ${folder} ]; then
    mkdir -p ${folder}
    fi
for rate in "5e5" "10e5" "15e5" "20e5" "25e5" "30e5"
do
for tcp in 0
do
    outfile="${folder}/ofdma-topo${note}-mcs${mcs}-rate${rate}-size${packetsize}-tcp${tcp}-${k}.txt"
    if [ ! -f ${outfile} ]; then
    touch ${outfile}
    echo ${outfile}
    ./waf --run "simple-ofdma-adhoc5 --ofdmaEnabled=true --startTime=60 --totalTime=120 --ratio=0.375 --locationFile=${locationfile} --routingFile=${routingfile} --appFile=${appfile} --randomTime=true --dataMode=HeMcs$mcs --datarate=$rate --packetSize=${packetsize} --tcp=${tcp} --RngRun=$k" &> ${outfile}
    fi

    outfile="${folder}/csma-topo${note}-mcs${mcs}-rate${rate}-size${packetsize}-tcp${tcp}-${k}.txt"
    if [ ! -f ${outfile} ]; then
    touch ${outfile}
    echo ${outfile}
    ./waf --run "simple-ofdma-adhoc5 --ofdmaEnabled=false --rtscts=1 --startTime=60 --totalTime=120 --ratio=0.375 --locationFile=${locationfile} --routingFile=${routingfile} --appFile=${appfile} --randomTime=true --dataMode=HeMcs$mcs --datarate=$rate --packetSize=${packetsize} --tcp=${tcp} --RngRun=$k" &> ${outfile}
    fi
done
done
done
done
done
done

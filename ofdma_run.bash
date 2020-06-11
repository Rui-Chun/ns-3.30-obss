#!/bin/bash

./waf

for k in {1..25..1}
do
for mcs in {3..5..1}
do
for note in "4"
do
    locationfile="locationtxt/locationFile${note}.txt"
    routingfile="locationtxt/routingFile${note}.txt"
    appfile="locationtxt/appFile.txt"
for rate in "5e5" "10e5" "15e5" "20e5" "25e5" "30e5"
do
for packetsize in "1e2" "2e2" "5e2" "10e2"
do
for tcp in 0
do
    outfile="my-simulations/8_ofdma8/ofdma-topo${note}-mcs${mcs}-rate${rate}-size${packetsize}-tcp${tcp}-${k}.txt"
    if [ ! -f ${outfile} ]; then
    touch ${outfile}
    echo ${outfile}
    ./waf --run "simple-ofdma-adhoc5 --ofdmaEnabled=true --startTime=60 --totalTime=120 --ratio=0.375 --locationFile=${locationfile} --routingFile=${routingfile} --appFile=${appfile} --randomTime=true --dataMode=HeMcs$mcs --datarate=$rate --packetSize=${packetsize} --tcp=${tcp} --RngRun=$k" &> ${outfile}
    fi

    outfile="my-simulations/8_ofdma8/csma-topo${note}-mcs${mcs}-rate${rate}-size${packetsize}-tcp${tcp}-${k}.txt"
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
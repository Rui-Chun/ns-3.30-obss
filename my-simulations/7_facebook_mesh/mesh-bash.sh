#!/bin/bash

./waf

# # udp
# for ((cnt=$1;cnt<=$2;cnt++))
# do
#     ./waf --run "mesh-loc-2 --apNum=26 --datarate=3e5 --app=udp --rndSeed=${cnt} --route=olsr" | tee ./mesh-bash-udp/${cnt}.txt
# done

# # udp static
# for ((cnt=$1;cnt<=$2;cnt++))
# do
#     ./waf --run "mesh-loc-2 --apNum=26 --datarate=3e5 --app=udp --rndSeed=${cnt} --route=static" | tee ./mesh-bash-udps/${cnt}.txt
# done

# udp
for ((cnt=$1;cnt<=$2;cnt++))
do
    ./waf --run "mesh-loc-2 --apNum=26 --datarate=10e5 --app=udpr --rndSeed=${cnt} --route=olsr" | tee ./mesh-bash-udpr/${cnt}.txt
done

# udp static
for ((cnt=$1;cnt<=$2;cnt++))
do
    ./waf --run "mesh-loc-2 --apNum=26 --datarate=10e5 --app=udpr --rndSeed=${cnt} --route=static" | tee ./mesh-bash-udprs/${cnt}.txt
done

# tcp
for ((cnt=$1;cnt<=$2;cnt++))
do
    ./waf --run "mesh-loc-2 --apNum=26 --datarate=6e5 --app=tcpr --rndSeed=${cnt} --route=olsr" | tee ./mesh-bash-tcpr/${cnt}.txt
done

# tcp static
for ((cnt=$1;cnt<=$2;cnt++))
do
    ./waf --run "mesh-loc-2 --apNum=26 --datarate=6e5 --app=tcpr --rndSeed=${cnt} --route=static" | tee ./mesh-bash-tcprs/${cnt}.txt
done

# # udp up/down link
# for ((cnt=$1;cnt<=$2;cnt++))
# do
#     ./waf --run "mesh-loc-2b --apNum=26 --datarate=2e3 --app=udp --rndSeed=${cnt} --route=olsr" | tee ./mesh-bash-udpb/${cnt}.txt
# done

# # udp up/down link
# for ((cnt=$1;cnt<=$2;cnt++))
# do
#     ./waf --run "mesh-loc-2b --apNum=26 --datarate=4e3 --app=udp --rndSeed=${cnt} --route=olsr" | tee ./mesh-bash-udpb-1/${cnt}.txt
# done

# # udp up/down link
# for ((cnt=$1;cnt<=$2;cnt++))
# do
#     ./waf --run "mesh-loc-2b --apNum=26 --datarate=20e5 --app=udp --rndSeed=${cnt} --route=olsr --locationFile=./my-simulations/3_real_topology/input/location2b-1.txt" | tee ./mesh-bash-udpb-2/${cnt}.txt
# done

rm minstrel-*.txt
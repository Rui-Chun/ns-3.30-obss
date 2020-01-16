#include <iostream>
#include <vector>

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/flow-monitor-helper.h"

using namespace ns3;

/// throughput monitor
std::vector<uint64_t> lastTotalRx;
std::vector<double> throughput;
std::vector<Ptr<PacketSink>> packetSink;

double
CalculateSingleStreamThroughput (Ptr<PacketSink> sink, uint64_t &lastTotalRx, double monitorInterval)
{
  double thr = (sink->GetTotalRx () - lastTotalRx) * (double) 8/1e3 / monitorInterval;
  lastTotalRx = sink->GetTotalRx ();
  return thr;
}

void
CalculateThroughput (double monitorInterval)
{
  std::cout << Simulator::Now ().GetSeconds ();
  for (uint32_t i = 0; i < packetSink.size (); ++i)
    {
      if (packetSink[i] == NULL)
        throughput[i] = -1;
      else
        throughput[i] = CalculateSingleStreamThroughput (packetSink[i], lastTotalRx[i], monitorInterval);
      std::cout << '\t' << throughput[i];
    }
  std::cout << std::endl;
  Simulator::Schedule (Seconds (monitorInterval), &CalculateThroughput, monitorInterval);
}

void
PrintThroughputTitle (uint32_t clNum)
{
  std::cout << "-------------------------------------------------\n";
  std::cout << "Time[s]";
  for (uint32_t i = 0; i < clNum; ++i)
      std::cout << '\t' << "cl-" << i;
  std::cout << std::endl;
}

int main (int argc, char **argv)
{
  CommandLine cmd;

  bool ofdmaEnabled = false;
  uint32_t clNum = 1;
  double startTime = 10.0;
  double totalTime = 20.0;
  double monitorInterval = 1.0;

  cmd.AddValue ("ofdmaEnabled", "", ofdmaEnabled);
  cmd.AddValue ("clNum", "", clNum);
  cmd.AddValue ("startTime", "Application start time, s.", startTime);
  cmd.AddValue ("totalTime", "Simulation time, s.", totalTime);
  cmd.AddValue ("monitorInterval", "Monitor interval, s.", monitorInterval);
  cmd.Parse (argc, argv);

  // Create nodes and set mobility
  NodeContainer apNodes;
  apNodes.Create (1);
  NodeContainer clNodes;
  clNodes.Create (clNum);

  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (apNodes);
  mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
                                 "X", DoubleValue (0.0),
                                 "Y", DoubleValue (0.0),
                                 "Rho", StringValue ("ns3::ConstantRandomVariable[Constant=10]"),
                                 "Theta", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=6.2830]"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (clNodes);

  // Install mac and phy
  NetDeviceContainer apDevices;
  NetDeviceContainer clDevices;

  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper chan = YansWifiChannelHelper::Default ();
  phy.SetChannel (chan.Create ());
  phy.Set ("ChannelNumber", UintegerValue (36));

  WifiHelper wifi;
  wifi.SetStandard (ns3::WIFI_PHY_STANDARD_80211ax_5GHZ);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "ControlMode", StringValue ("HeMcs0"),
                                "DataMode", StringValue ("HeMcs7"),
                                "RtsCtsThreshold", UintegerValue (65535));

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ofdma");
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "QosSupported", BooleanValue (false),
               "OfdmaSupported", BooleanValue (ofdmaEnabled));
  apDevices = wifi.Install (phy, mac, apNodes);
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "QosSupported", BooleanValue (false),
               "OfdmaSupported", BooleanValue (ofdmaEnabled));
  clDevices = wifi.Install (phy, mac, clNodes);

  // Install Internet stack
  InternetStackHelper stack;
  stack.Install (apNodes);
  stack.Install (clNodes);

  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer apInterfaces = address.Assign (apDevices);
  Ipv4InterfaceContainer clInterfaces = address.Assign (clDevices);

  // Install application
  ApplicationContainer apApplications;
  ApplicationContainer clApplications;
  for (uint32_t i = 0; i < clNum; i++)
    {
      uint16_t port = 5000+i;
      Address localAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
      PacketSinkHelper server ("ns3::UdpSocketFactory", localAddress);
      clApplications.Add (server.Install (clNodes.Get (i)));
      packetSink.push_back (StaticCast<PacketSink> (clApplications.Get (i)));
      lastTotalRx.push_back (0);
      throughput.push_back (0);

      OnOffHelper client ("ns3::UdpSocketFactory", Address ());
      client.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      client.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      client.SetAttribute ("DataRate", DataRateValue (DataRate ((uint64_t) (8e3))));
      client.SetAttribute ("PacketSize", UintegerValue (1000));
      client.SetAttribute ("MaxBytes", UintegerValue (0));
      AddressValue remoteAddress (InetSocketAddress (clInterfaces.GetAddress (i), port));
      client.SetAttribute ("Remote", remoteAddress);
      apApplications.Add (client.Install (apNodes));
    }
  apApplications.Start (Seconds (startTime));
  apApplications.Stop (Seconds (totalTime));
  clApplications.Start (Seconds (0.0));
  clApplications.Stop (Seconds (totalTime + 0.1));

  Simulator::Schedule (Seconds (startTime), &PrintThroughputTitle, clNum);
  Simulator::Schedule (Seconds (startTime+monitorInterval), &CalculateThroughput, monitorInterval);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (totalTime + 0.1));
  Simulator::Run ();
  Simulator::Destroy ();

  return true;
}
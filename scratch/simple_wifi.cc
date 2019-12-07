#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <algorithm>

#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/config-store.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"
#include "ns3/olsr-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/potential-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mesh-module.h"
#include "ns3/v4ping-helper.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"

using namespace std;
using namespace ns3;

std::vector<uint64_t> lastTotalRx;
std::vector<double> throughput;
std::vector<Ptr<PacketSink>> packetSink;

double
CalculateSingleStreamThroughput (Ptr<PacketSink> sink, uint64_t &lastTotalRx, double monitorInterval)
{
  double thr = (sink->GetTotalRx () - lastTotalRx) * (double) 8/1e6 / monitorInterval;
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
        {
          throughput[i] = -1;
          continue;
        }
      else
        throughput[i] = CalculateSingleStreamThroughput (packetSink[i], lastTotalRx[i], monitorInterval);
      std::cout << '\t' << throughput[i];
    }
  std::cout << std::endl;
  Simulator::Schedule (Seconds (monitorInterval), &CalculateThroughput, monitorInterval);
}

void
PrintThroughputTitle (uint32_t apNum)
{
  std::cout << "-------------------------------------------------\n";
  std::cout << "Time[s]";
  for (uint32_t i = 0; i < apNum; ++i)
      std::cout << '\t' << "ap-" << i;
  std::cout << std::endl;
}

int main(int argc, char** argv)
{
  std::cout << fixed << setprecision(3);

  uint32_t apNum = 2;
  double apStep = 50;
  std::string route ("aodv");
  std::string gateways ("0");
  double datarate = 2e6;
  double startTime = 60;
  double totalTime = 120;
  double monitorInterval = 1;
  bool printRoutes = false;

	CommandLine cmd;
  cmd.AddValue ("apNum", "number of mesh nodes", apNum);
  cmd.AddValue ("apStep", "distance between mesh neighbors", apStep);
  cmd.AddValue ("route", "routing protocol", route);
  cmd.AddValue ("gateways", "index of gateways in mesh", gateways);
  cmd.AddValue ("datarate", "tested application datarate", datarate);
  cmd.AddValue ("startTime", "tested application start time, s", startTime);
  cmd.AddValue ("totalTime", "simulation time, s", totalTime);
  cmd.AddValue ("monitorInterval", "monitor interval, s", monitorInterval);
  cmd.AddValue ("printRoutes", "print routing table of all nodes", printRoutes);
	cmd.Parse (argc, argv);

  lastTotalRx = std::vector<uint64_t> (apNum, 0);
  throughput = std::vector<double> (apNum, 0);
  packetSink = std::vector<Ptr<PacketSink>> (apNum, 0);

  std::vector<uint32_t> gatewayIndex;
  std::replace (gateways.begin (), gateways.end (), '+', ' ');
  std::stringstream gatewayss(gateways);
  uint32_t temp;
  while (gatewayss >> temp)
    gatewayIndex.push_back (temp);

  // create nodes
	NodeContainer meshNodes;
	meshNodes.Create (apNum);
  NodeContainer serverNodes;
  serverNodes.Create (1);

  // classify nodes
  NodeContainer meshGateways;
  for (auto it : gatewayIndex)
    {
      meshGateways.Add (meshNodes.Get (it));
    }
  NodeContainer meshRelays;
  for (uint32_t it = 0; it < meshNodes.GetN (); it++)
    {
      if (std::find (gatewayIndex.begin (), gatewayIndex.end (), it) == gatewayIndex.end ())
        {
          meshRelays.Add (meshNodes.Get (it));
        }
    }
  NodeContainer csmaNodes;
  csmaNodes.Add (serverNodes.Get (0));
  csmaNodes.Add (meshGateways);
  NodeContainer allNodes (meshNodes);
  allNodes.Add (serverNodes);

  // mobility
	MobilityHelper mobility;
	mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0),
                                 "MinY", DoubleValue (0),
                                 "DeltaX", DoubleValue (apStep),
                                 "DeltaY", DoubleValue (0),
                                 "GridWidth", UintegerValue (1000));
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (meshNodes);
	mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0),
                                 "MinY", DoubleValue (0),
                                 "DeltaX", DoubleValue (10),
                                 "DeltaY", DoubleValue (0),
                                 "GridWidth", UintegerValue (1000));
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (serverNodes);

  // mesh devices
	YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
	YansWifiChannelHelper chan = YansWifiChannelHelper::Default ();
	phy.SetChannel (chan.Create ());
	phy.Set ("ChannelNumber", UintegerValue (36));
	phy.Set ("Antennas", UintegerValue (1));
	phy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (1));
	phy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (1));
	WifiMacHelper mac;
	mac.SetType("ns3::AdhocWifiMac");
	WifiHelper wifi;
	wifi.SetStandard (ns3::WIFI_PHY_STANDARD_80211n_2_4GHZ);
	wifi.SetRemoteStationManager ("ns3::MinstrelHtWifiManager",
                                "RtsCtsThreshold", UintegerValue (1));
	NetDeviceContainer meshDevices;
  meshDevices = wifi.Install (phy, mac, meshNodes);

  // csma devices
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("1024Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

	// internet stack
  InternetStackHelper internet;
	Ipv4ListRoutingHelper list;
  if (route == std::string ("aodv"))
    {
      AodvHelper aodv;
      list.Add (aodv, 10);
      internet.SetRoutingHelper (aodv);
      internet.Install (allNodes);
    }
  else if (route == std::string ("olsr"))
    {
      OlsrHelper olsr;
      list.Add (olsr, 10);
      internet.SetRoutingHelper (olsr);
      internet.Install (allNodes);
    }
  else if (route == std::string ("potential"))
    {
      PotentialHelper serverPotential;
      serverPotential.Set ("FixedPotential", BooleanValue (true));
      serverPotential.Set ("Potential", UintegerValue (1001));
      internet.SetRoutingHelper (serverPotential);
      internet.Install (serverNodes);

      PotentialHelper gatewayPotential;
      gatewayPotential.Set ("FixedPotential", BooleanValue (true));
      gatewayPotential.Set ("Potential", UintegerValue (1000));
      internet.SetRoutingHelper (gatewayPotential);
      internet.Install (meshGateways);

      PotentialHelper relayPotential;
      relayPotential.Set ("FixedPotential", BooleanValue (false));
      relayPotential.Set ("Potential", UintegerValue (0));
      internet.SetRoutingHelper (relayPotential);
      internet.Install (meshRelays);

      if (printRoutes)
        {
          Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);
          serverPotential.PrintRoutingTableAllEvery (Seconds (60), routingStream);
        }
    }
  else
    {
      std::cout << "Wrong routing protocol." << std::endl;
      return (0);
    }

  // interfaces
	Ipv4AddressHelper addrHelper;
	addrHelper.SetBase("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer meshIfaces = addrHelper.Assign (meshDevices);
  addrHelper.SetBase("10.1.2.0", "255.255.255.0");
	Ipv4InterfaceContainer csmaIfaces = addrHelper.Assign (csmaDevices);

  // prime applications
  uint16_t port = 9999;
  UdpEchoServerHelper echoServer (port);
  ApplicationContainer echoS = echoServer.Install (serverNodes);
  echoS.Start (Seconds (30));
  echoS.Stop (Seconds (totalTime + 1));
  UdpEchoClientHelper echoClient (csmaIfaces.GetAddress (0), port);
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (100));
  ApplicationContainer echoC = echoClient.Install (meshNodes);
  echoC.Start (Seconds (30));
  echoC.Stop (Seconds (startTime -1));

  // test applications
  ApplicationContainer udpUS;
  ApplicationContainer udpUC;
  for (uint32_t i = 0; i < meshNodes.GetN (); i++)
    {
      uint16_t port = 10000+i;
      Address localAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
      PacketSinkHelper sinkUl ("ns3::UdpSocketFactory", localAddress);
      udpUS.Add (sinkUl.Install (serverNodes.Get (0)));
      packetSink[i] = StaticCast<PacketSink> (udpUS.Get (i));

      OnOffHelper sendUl ("ns3::UdpSocketFactory", Address ());
      sendUl.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      sendUl.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      // sendUl.SetAttribute ("OnTime", StringValue ("ns3::ExponentialRandomVariable[Bound=1|Mean=0.5]"));
      // sendUl.SetAttribute ("OffTime", StringValue ("ns3::ExponentialRandomVariable[Bound=1|Mean=0.5]"));
      sendUl.SetAttribute ("PacketSize", UintegerValue (1472));
      sendUl.SetAttribute ("DataRate", DataRateValue (DataRate ((uint64_t) (datarate))));
      sendUl.SetAttribute ("MaxBytes", UintegerValue (0));
      AddressValue remoteAddress (InetSocketAddress (csmaIfaces.GetAddress (0), port)); //
      sendUl.SetAttribute ("Remote", remoteAddress);
      udpUC.Add (sendUl.Install (meshNodes.Get (i)));
    }
  udpUS.Start (Seconds (startTime));
  udpUS.Stop (Seconds (totalTime + 1));
  udpUC.Start (Seconds (startTime));
  udpUC.Stop (Seconds (totalTime + 1));

	// global routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Schedule (Seconds (startTime), &PrintThroughputTitle, apNum);
  Simulator::Schedule (Seconds (startTime+monitorInterval), &CalculateThroughput, monitorInterval);

  // run
	Simulator::Stop (Seconds (totalTime));
	Simulator::Run ();
	Simulator::Destroy ();

	return 0;
}

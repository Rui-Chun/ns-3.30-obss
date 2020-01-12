<<<<<<< HEAD
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
=======
#include <cstdio>

#include <ns3/aodv-helper.h>
#include <ns3/boolean.h>
#include <ns3/bulk-send-application.h>
#include <ns3/bulk-send-helper.h>
#include <ns3/command-line.h>
#include <ns3/config.h>
#include <ns3/config-store.h>
#include <ns3/double.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/ipv4-address-helper.h>
#include <ns3/ipv4-interface-address.h>
#include <ns3/ipv4-interface.h>
#include <ns3/ipv4-list-routing-helper.h>
#include <ns3/ipv4-static-routing-helper.h>
#include <ns3/log.h>
#include <ns3/mesh-helper.h>
#include <ns3/mobility-helper.h>
#include <ns3/olsr-helper.h>
#include <ns3/onoff-application.h>
#include <ns3/packet-sink.h>
#include <ns3/packet-sink-helper.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/simulator.h>
#include <ns3/ssid.h>
#include <ns3/string.h>
#include <ns3/system-wall-clock-ms.h>
#include <ns3/three-gpp-http-client.h>
#include <ns3/three-gpp-http-server.h>
#include <ns3/udp-echo-client.h>
#include <ns3/udp-echo-helper.h>
#include <ns3/udp-echo-server.h>
#include <ns3/uinteger.h>
#include <ns3/wifi-helper.h>
#include <ns3/yans-wifi-helper.h>
>>>>>>> 31a61935ffccfa6b8adcd80d3406921f8d9917cf

using namespace std;
using namespace ns3;

<<<<<<< HEAD
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
=======
long received_bytes = 0;

void rxCounter(Ptr<const Packet> packet, const Address &address)
{
	received_bytes += packet->GetSize();
>>>>>>> 31a61935ffccfa6b8adcd80d3406921f8d9917cf
}

int main(int argc, char** argv)
{
<<<<<<< HEAD
  std::cout << fixed << setprecision(3);
  Config::SetDefault ("ns3::WifiMacQueue::MaxQueueSize", StringValue ("1000p"));
  Config::SetDefault ("ns3::WifiMacQueue::MaxDelay", TimeValue (Seconds (0.2)));
  // Config::SetDefault ("ns3::WifiMacQueue::DropPolicy", EnumValue (WifiMacQueue::DROP_OLDEST));

  uint32_t apNum = 2;
  double apXStep = 50;
  double apYStep = 50;
  uint32_t apXSize = 100;
  std::string route ("aodv");
  std::string gateways ("0");
  double datarate = 1e6;
  double startTime = 60;
  double totalTime = 120;
  double monitorInterval = 1;
  bool printRoutes = false;

	CommandLine cmd;
  cmd.AddValue ("apNum", "number of mesh nodes", apNum);
  cmd.AddValue ("apXStep", "X distance between mesh neighbors", apXStep);
  cmd.AddValue ("apYStep", "Y distance between mesh neighbors", apYStep);
  cmd.AddValue ("apXSize", "X size of mesh grid", apXSize);
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
                                 "DeltaX", DoubleValue (apXStep),
                                 "DeltaY", DoubleValue (apYStep),
                                 "GridWidth", UintegerValue (apXSize));
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (meshNodes);
	mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0),
                                 "MinY", DoubleValue (0));
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
      serverPotential.Set ("Potential", UintegerValue (65536));
      internet.SetRoutingHelper (serverPotential);
      internet.Install (serverNodes);

      PotentialHelper gatewayPotential;
      gatewayPotential.Set ("FixedPotential", BooleanValue (true));
      gatewayPotential.Set ("Potential", UintegerValue (65535));
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
  echoS.Start (Seconds (startTime / 2));
  echoS.Stop (Seconds (totalTime + 1));
  UdpEchoClientHelper echoClient (csmaIfaces.GetAddress (0), port);
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (100));
  ApplicationContainer echoC = echoClient.Install (meshNodes);
  echoC.Start (Seconds (startTime / 2));
  echoC.Stop (Seconds (startTime - 1));

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
	Simulator::Stop (Seconds (totalTime + 0.1));
	Simulator::Run ();
	Simulator::Destroy ();
=======
	int n_antennas = 1;
	int n_streams = 1;
	int n_apsta = 0;
	int useg = 0;
	Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1446));

	CommandLine cmd;
	cmd.AddValue("antennas", "Nbr of antennas in WiFi devices", n_antennas);
	cmd.AddValue("streams", "Nbr of spatial streams supported in Wifi Devices",
			n_streams);
	cmd.AddValue("apsta", "Use AP <-> STA instead of mesh", n_apsta);
	cmd.AddValue("useg", "Use 802.11g instead of 802.11n", useg);
	cmd.Parse(argc, argv);

	YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
	YansWifiChannelHelper chan = YansWifiChannelHelper::Default();
	phy.SetChannel(chan.Create());
        phy.Set("ChannelNumber", UintegerValue(36));
	phy.Set("Antennas", UintegerValue(n_antennas));
	phy.Set("MaxSupportedTxSpatialStreams", UintegerValue(n_streams));
	phy.Set("MaxSupportedRxSpatialStreams", UintegerValue(n_streams));

	NodeContainer nodes;
	nodes.Create(2);

	MobilityHelper mobility;
	mobility.SetPositionAllocator("ns3::GridPositionAllocator",
		"MinX", DoubleValue(0),
		"MinY", DoubleValue(0),
		"DeltaX", DoubleValue(10),
		"DeltaY", DoubleValue(0),
		"GridWidth", UintegerValue(1000));
	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility.Install(nodes);

	NetDeviceContainer devices;
	if (n_apsta == 1) {
		WifiHelper wifi;
		wifi.SetStandard(useg ? ns3::WIFI_PHY_STANDARD_80211g :
					ns3::WIFI_PHY_STANDARD_80211n_5GHZ);
		wifi.SetRemoteStationManager("ns3::MinstrelHtWifiManager");

		WifiMacHelper mac;
		Ssid ssid = Ssid("ns-3-ssid");
		mac.SetType("ns3::StaWifiMac",
			"Ssid", SsidValue(ssid),
			"ActiveProbing", BooleanValue(false));
		NetDeviceContainer staDevice;
		NodeContainer sta(nodes.Get(1));
		staDevice = wifi.Install(phy, mac, sta);

		mac.SetType("ns3::ApWifiMac",
			"Ssid", SsidValue(ssid));
		NodeContainer ap(nodes.Get(0));
		NetDeviceContainer apDevice = wifi.Install(phy, mac, ap);

		devices.Add(apDevice);
		devices.Add(staDevice);
	} else if (n_apsta == 0) {
		MeshHelper mesh = MeshHelper::Default();
		mesh.SetStandard(useg ? ns3::WIFI_PHY_STANDARD_80211g :
					ns3::WIFI_PHY_STANDARD_80211n_5GHZ);
		mesh.SetRemoteStationManager("ns3::MinstrelHtWifiManager");

		mesh.SetStackInstaller("ns3::Dot11sStack");
		mesh.SetSpreadInterfaceChannels(MeshHelper::ZERO_CHANNEL);
		mesh.SetMacType("RandomStart", TimeValue(Seconds(0.1)));
		mesh.SetNumberOfInterfaces(1);

		devices = mesh.Install(phy, nodes);
	} else {
                WifiHelper wifi;
		wifi.SetStandard(useg ? ns3::WIFI_PHY_STANDARD_80211g :
					ns3::WIFI_PHY_STANDARD_80211n_5GHZ);
		wifi.SetRemoteStationManager("ns3::MinstrelHtWifiManager");

                WifiMacHelper mac;
                mac.SetType("ns3::AdhocWifiMac");
                
		devices = wifi.Install(phy, mac, nodes);
        }

	// Now create the interfaces
	Ipv4StaticRoutingHelper staticRouting;
	Ipv4ListRoutingHelper list;
	list.Add (staticRouting, 0);
	OlsrHelper olsr;
	list.Add (olsr, 10);

	InternetStackHelper internet;
	internet.SetRoutingHelper(list);
	internet.Install(nodes);

	Ipv4AddressHelper addrHelper;
	addrHelper.SetBase("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer ifaces = addrHelper.Assign(devices);

	// Install Applications
	Ptr<Application> udpEchoServer = CreateObject<UdpEchoServer>();
	udpEchoServer->SetAttribute("Port", UintegerValue(1000));
	udpEchoServer->SetAttribute("StartTime", StringValue("0s"));
	Ptr<Application> udpEchoClient = CreateObject<UdpEchoClient>();
	udpEchoClient->SetAttribute("RemotePort", UintegerValue(1000));
	udpEchoClient->SetAttribute("RemoteAddress", AddressValue(Ipv4Address("10.1.1.1")));
	udpEchoClient->SetAttribute("StartTime", StringValue("0s"));

	nodes.Get(0)->AddApplication(udpEchoServer);
	nodes.Get(1)->AddApplication(udpEchoClient);

	// Ptr<Application> fileServer = CreateObject<BulkSendApplication>();
	Ptr<Application> fileServer = CreateObject<OnOffApplication>();
	InetSocketAddress sockaddr_send(Ipv4Address("10.1.1.2"), 1001);
	fileServer->SetAttribute("Remote", AddressValue(sockaddr_send));
	fileServer->SetAttribute("StartTime", StringValue("20s"));
	fileServer->SetAttribute("Protocol", StringValue("ns3::TcpSocketFactory"));
	// fileServer->SetAttribute("SendSize", UintegerValue(65536));
        fileServer->SetAttribute("DataRate", DataRateValue(50e6));
        fileServer->SetAttribute("PacketSize", UintegerValue(65536));
	Ptr<Application> fileClient = CreateObject<PacketSink>();
	InetSocketAddress sockaddr_recv(Ipv4Address::GetAny(), 1001);
	fileClient->SetAttribute("Local", AddressValue(sockaddr_recv));
	fileClient->SetAttribute("StartTime", StringValue("20"));
	fileClient->SetAttribute("Protocol", StringValue("ns3::TcpSocketFactory"));

	nodes.Get(0)->AddApplication(fileServer);
	nodes.Get(1)->AddApplication(fileClient);

	fileClient->TraceConnectWithoutContext("Rx", MakeCallback(&rxCounter));

	// Enable logging
	//LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);

	// Now run the sim
	Simulator::Stop(Seconds(60));
	Simulator::Run();
	Simulator::Destroy();

	printf("Number of bytes total received: %ld\n", received_bytes);
	printf("Rate: %g Mbps\n", (received_bytes * 8.0) / (40. * 1.e6));
>>>>>>> 31a61935ffccfa6b8adcd80d3406921f8d9917cf

	return 0;
}

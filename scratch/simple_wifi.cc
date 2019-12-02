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

using namespace std;
using namespace ns3;

long received_bytes = 0;

void rxCounter(Ptr<const Packet> packet, const Address &address)
{
	received_bytes += packet->GetSize();
}

int main(int argc, char** argv)
{
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

	return 0;
}

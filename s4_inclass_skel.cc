#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("s4-inclass");

class MyApp : public Application
{
public:

	MyApp();
	virtual ~MyApp();

	void Setup(Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
	virtual void StartApplication(void);
	virtual void StopApplication(void);

	void ScheduleTx(void);
	void SendPacket(void);

	Ptr<Socket>     m_socket;
	Address         m_peer;
	uint32_t        m_packetSize;
	uint32_t        m_nPackets;
	DataRate        m_dataRate;
	EventId         m_sendEvent;
	bool            m_running;
	uint32_t        m_packetsSent;
};

MyApp::MyApp()
	: m_socket(0),
	m_peer(),
	m_packetSize(0),
	m_nPackets(0),
	m_dataRate(0),
	m_sendEvent(),
	m_running(false),
	m_packetsSent(0)
{
}

MyApp::~MyApp()
{
	m_socket = 0;
}

void
MyApp::Setup(Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
	m_socket = socket;
	m_peer = address;
	m_packetSize = packetSize;
	m_nPackets = nPackets;
	m_dataRate = dataRate;
}

void
MyApp::StartApplication(void)
{
	m_running = true;
	m_packetsSent = 0;
	m_socket->Bind();
	m_socket->Connect(m_peer);
	SendPacket();
}

void
MyApp::StopApplication(void)
{
	m_running = false;

	if (m_sendEvent.IsRunning())
	{
		Simulator::Cancel(m_sendEvent);
	}

	if (m_socket)
	{
		m_socket->Close();
	}
}

void
MyApp::SendPacket(void)
{
	Ptr<Packet> packet = Create<Packet>(m_packetSize);
	m_socket->Send(packet);

	if (++m_packetsSent < m_nPackets)
	{
		ScheduleTx();
	}
}

void
MyApp::ScheduleTx(void)
{
	if (m_running)
	{
		Time tNext(Seconds(m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate())));
		m_sendEvent = Simulator::Schedule(tNext, &MyApp::SendPacket, this);
	}
}


static void
CwndChange(uint32_t oldCwnd, uint32_t newCwnd)
{
	std::cout << Simulator::Now().GetSeconds() << "\t" << newCwnd << std::endl;
}

int
main(int argc, char *argv[])
{
	double udpRateMbps = 0.5; // UDP source rate in Mb/s, default: 0.5 Mb/s

	CommandLine cmd;
	cmd.AddValue("udpRateMbps", "Datarate of UDP source in bps", udpRateMbps);
	cmd.Parse(argc, argv);

	int udpRate = udpRateMbps * 1000 * 1000; // UDP source rate in b/s

  // Create nodes
	NS_LOG_INFO("Create nodes.");
	Ptr<Node> nSrc1 = CreateObject<Node>();
	Ptr<Node> nSrc2 = CreateObject<Node>();
	Ptr<Node> nRtr = CreateObject<Node>();
	Ptr<Node> nDst = CreateObject<Node>();

	NodeContainer nodes = NodeContainer(nSrc1, nSrc2, nRtr, nDst);

	NodeContainer nSrc1nRtr = NodeContainer(nSrc1, nRtr);
	NodeContainer nSrc2nRtr = NodeContainer(nSrc2, nRtr);
	NodeContainer nRtrnDst = NodeContainer(nRtr, nDst);

	InternetStackHelper stack;
	stack.Install(nodes);

	// Create P2P channels
	PointToPointHelper p2p;
	p2p.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
	p2p.SetChannelAttribute("Delay", StringValue("2ms"));

	NetDeviceContainer dSrc1dRtr = p2p.Install(nSrc1nRtr);
	NetDeviceContainer dSrc2dRtr = p2p.Install(nSrc2nRtr);
	NetDeviceContainer dRtrdDst = p2p.Install(nRtrnDst);

	// Add IP addresses
	NS_LOG_INFO("Assign IP Addresses.");
	Ipv4AddressHelper ipv4;
	ipv4.SetBase("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer iSrc1iRtr = ipv4.Assign(dSrc1dRtr);
	ipv4.SetBase("10.1.2.0", "255.255.255.0");
	Ipv4InterfaceContainer iSrc2iRtr = ipv4.Assign(dSrc2dRtr);
	ipv4.SetBase("10.1.3.0", "255.255.255.0");
	Ipv4InterfaceContainer iRtriDst = ipv4.Assign(dRtrdDst);

	// Set up the routing tables
	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	// Implement TCP & UDP sinks to the destinations
	uint16_t sinkPortTcp = 8080;
	uint16_t sinkPortUdp = 9090;
	Address sinkAddressTcp(InetSocketAddress(iRtriDst.GetAddress(1), sinkPortTcp));
	Address sinkAddressUdp(InetSocketAddress(iRtriDst.GetAddress(1), sinkPortUdp));

	//==========================================================================================
	/* ToDo: Install packet sinks to the destinations
		  Hint: Need to install packet sinks for both TCP and UDP traffic */
	PacketSinkHelper packetSinkHelperTcp("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sinkPortTcp));
	ApplicationContainer sinkAppTcp = packetSinkHelperTcp.Install(nDst);
	PacketSinkHelper packetSinkHelperUdp("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sinkPortUdp));
	ApplicationContainer sinkAppUdp = packetSinkHelperUdp.Install(nDst);
	//==========================================================================================

	sinkAppTcp.Start(Seconds(0.));
	sinkAppTcp.Stop(Seconds(30.));
	sinkAppUdp.Start(Seconds(0.));
	sinkAppUdp.Stop(Seconds(30.));

	// OLD: Implement TCP application
	//OnOffHelper onoffTcp("ns3::TcpSocketFactory", sinkAddressTcp);
	//onoffTcp.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
	//onoffTcp.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
	//onoffTcp.SetAttribute("DataRate", DataRateValue(500000));
	//ApplicationContainer sourceAppTcp = onoffTcp.Install(nSrc1);
	//sourceAppTcp.Start(Seconds(5.));
	//sourceAppTcp.Stop(Seconds(20.));


	//==========================================================================================
	/* ToDo: Connect the trace source and the trace sink
		 Hint: Refer to s4_ex2.cc */
	Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(nSrc1, TcpSocketFactory::GetTypeId());
	ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&CwndChange));

	//nSrc1->GetApplication(0)->GetObject<OnOffApplication>()->SetSocket(ns3TcpSocket);
	//==========================================================================================
	
	// Implement TCP application
	Ptr<MyApp> app = CreateObject<MyApp>();
	app->Setup(ns3TcpSocket, sinkAddressTcp, 1040, 1000, DataRate("0.5Mbps"));
	nSrc1->AddApplication(app);
	app->SetStartTime(Seconds(5.));
	app->SetStopTime(Seconds(20.));

	//==========================================================================================
	/* ToDo: Implement UDP application
		 Hint: Refer to the TCP app implementation procedure above
		 Warning: UDP app turns on and off every 1s and use variable "udpRate" for DataRate */
	OnOffHelper onoffUdp("ns3::UdpSocketFactory", sinkAddressUdp);
	onoffUdp.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
	onoffUdp.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
	onoffUdp.SetAttribute("DataRate", DataRateValue(udpRate));
	ApplicationContainer sourceAppUdp = onoffUdp.Install(nSrc2);
	sourceAppUdp.Start(Seconds(1.));
	sourceAppUdp.Stop(Seconds(30.));
	//==========================================================================================

	Simulator::Stop(Seconds(30));
	Simulator::Run();
	Simulator::Destroy();



}

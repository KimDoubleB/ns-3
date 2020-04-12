#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("s4_ex2");

// Trace sinks Cwnd(Congestion Window) Change
static void
CwndChange(uint32_t oldCwnd, uint32_t newCwnd)
{
    NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "\t" << newCwnd);
}

// Trace sinks RxDrop (Receive drop)
static void
RxDrop(Ptr<const Packet> p)
{
    NS_LOG_UNCOND("RxDrop at" << Simluator::Now().GetSeconds());
}

int
main(int argc, char *argv[]){
    // Create network topology
    NodeContainer nodes;
    nodes.Create(2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    // Define an error model
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    em->SetAttribute("ErrorRate", DoubleValue(0.00001));
    devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

    // Install Internet stack and assign IP addresses
    InternetStackHelper stack;
    stack.Install(nodes);  
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.252");
    Ipv4InterfaceContainer interfaces = address.Assign(devices); // IP 할당

    // Implement TCP sink application
    uint16_t sinkPort = 8080;
    Address sinkAddress(InetSocketAddress(interfaces.GetAddress(1), sinkPort));
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
    ApplicationContainer sinkApp = packetSinkHelper.Install(nodes.Get(1));
    sinkApp.Start(Seconds(0.));
    sinkApp.Stop(Seconds(10.));

    // Connect RxDrop trace source and sink
    devices.Get(1)->TraceConnectWithoutContext("phyRxDrop", MakeCallback(&RxDrop));

    // Implement TCP source application
    OnOffHelper onoff("ns3::TcpSocketFactory", sinkAddress);
    onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    onoff.SetAttribute("DataRate", DataRateValue(1000000));
    ApplicationContainer sourceApp = onoff.Install(nodes.Get(0));
    sourceApp.Start(Seconds(1.));
    sourceApp.Stop(Seconds(10.));

    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(nodees.Get(0), TcpSocketFactory::GetTypeId());
    // Connect CwndChange trace source and sink
    ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&CwndChange));
    nodes.Get(0)->GetApplication(0)->GetObject<OnOffApplication>()->SetSocket(ns3TcpSocket);

    Simulator::Stop(Seconds(10));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

int main(int argc, char *argv[])
{
    // Command Line input
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Create a node container and 2 nodes for p2p link
    NodeContainer terminals;
    terminals.Create(2);

    // Create a netdevice container and p2p link
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetDeviceAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(terminals);

    // Install Internet stack on the nodes
    InternetStackHelper internet;
    internet.Install(terminals.Get(0));
    internet.Install(terminals.Get(1));

    // Allocate IP address
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    ipv4.Assign(p2pDevices);

    // Setup OnOff application
    unit16_t port = 9;
    OnOffHelper onoff("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address("10.1.1.2"), port)));
    onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
    onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
    onoff.SetAttribute("DataRate", DataRateValue(5000000));

    // Install OnOff application sender on a node
    ApplicationContainer app = onoff.Install(terminals.Get(0));
    app.Start(Seconds(1.0));
    app.Stop(Seconds(10.0));

    // Create a packet sink to receive packet and install it on a node
    PacketSinkHelper sink("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    app = sink.Install(terminals.Get(1));
    app.Start(Seconds(1.0));

    // Running simulator
    Simulator::Stop(Seconds(15));
    Simulator::Run();
    Simulator::Destory();
}
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 10.1.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("s5_ex1");

int
main(int argc, char* argv[]){

    bool verbose = true;
    uint32_t nCsma = 3;

    // Define commandLine input
    CommandLine cmd;
    cmd.AddValue("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    cmd.Parse(argc, argv);

    if(verbose){
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    nCsma = nCsma == 0 ? 1 : nCsma;

    // Create nodes (p2p/csma)
    // p2p - client // csma - Server
    NodeContainer p2pNodes;
    p2pNodes.Create(2);

    NodeContainer csmaNodes;
    csmaNodes.Add(p2pNodes.Get(1)); // Connect one nodes which is connected by p2p link
    csmaNodes.Create(nCsma);

    // p2p link
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer p2pDevices;
    p2pDevices = p2p.Install(p2pNodes);

    // csma link
    CsmaHelper csma;
    csma.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    SimpleNetDeviceHelper csmaDevices;
    csmaDevices = csma.Install(csmaNodes);

    // Install internet stack and assign IP address
    InternetStackHelper stack;
    stack.Install(p2pNodes.Get(0));
    stack.Install(csmaNodes);

    // set p2p IP address
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);

    // set csma IP address
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign(csmaDevices);


    // UDP Server - csmaNodes(nCsma)
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(csmaNodes.Get(nCsma));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));


    // UDP client - p2pNodes(0)
    UdpEchoClientHelper echoClient(csmaInterfaces.GetAddress(nCsma), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(p2pNodes(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));


    // activate Routing table
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();


    // Enable Pcap (p2p/csma)
    p2p.EnablePcapAll("s5_ex1");
    csma.EnablePcap("s5_ex1", csmaDevices.Get(1), true);

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
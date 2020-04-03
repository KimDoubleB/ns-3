#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("UdpEchoExample");

int main(int argc, char *argv[])
{
    // To determine whether the logging componenets are enabled or not
    bool verbose = true;

    // command line arguement for variable "verbose"
    CommandLine cmd;
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    cmd.Parse(argc, argv);

    // If verbose "true", log components are enabled
    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    // Create a node container and 2 nodes
    NodeContainer p2pNodes;
    p2pNodes.Create(2);

    // p2p link generation and install on the p2p nodes
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    // Connection
    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes); // NodeContainer

    // Install Internet stack on the nodes
    InternetStackHelper stack;
    stack.Install(p2pNodes.Get(0));
    stack.Install(p2pNodes.Get(1));

    // Allocate IP address
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0","255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices); // NetDeviceContainer

    // Setup echoServer and Install it on node 1
    UdpEchoServerHelper echoServer(9); // port number
    ApplicationContainer serverApps = echoServer.Install(p2pNodes.Get(1));
    // Application::SetStartTime(Time time) --> (virtual) Application::StartApplication()
    // Application::SetStopTime(Time time) --> (virtual) Application::StopApplication()
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    // Setup echoClient
    UdpEchoClientHelper echoClient(p2pInterfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(100));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    // Install echoClient on node 0
    ApplicationContainer clientApps = echoClient.Install(p2pNodes.Get(0));
    // Application::SetStartTime(Time time) --> (virtual) Application::StartApplication()
    // Application::SetStopTime(Time time) --> (virtual) Application::StopApplication()
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    // Enable pcap tracing
    pointToPoint.EnablePcapAll("second");

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}

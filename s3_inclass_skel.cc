#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("S3_INCLASS_EXERCISE");

int main(int argc, char* argv[]){

    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

    /* Create a node container */
    NodeContainer nodes;
    nodes.Create(2);

    /* Create a point-to-point link */
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("10us"));

    /* Create a netdevice container */
    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    /* Install a protocol stack on nodes */
    InternetStackHelper stack;
    // stack.Install(nodes);
    stack.Install(nodes.Get(0));
    stack.Install(nodes.Get(1));

    /* Allocate IP addresses */
    Ipv4AddressHelper address;
    Ipv4InterfaceContainer interfaces;

    /* Todo 4: allocate IP addresses on netdevicecontainer
    1) set the base address of IP address to be allocated using "SetBase" method of Ipv4AddressHelper 'ipv4', base address "10.1.1.0", and subnet mask "255.255.255.0")
    2) allocate IP address on netdevicecontainer using "Assign" method of Ipv4AddressHelper and Ipv4InterfaceContainer */
    address.SetBase("10.1.1.0", "255.255.255.0");
    interfaces = address.Assign(devices);


    /* Create udpServer application */
    uint16_t port = 9;
    UdpEchoServerHelper echoServer(port);
    ApplicationContainer serverApps;
    /* Todo 5: 
    1) Install UdpServerHelper 'server' on "node0" using ApplicationContainer 'apps', and "install" method of UdpServerHelper 'server'
    2) start the application at 1 sec and stop at 10 sec.*/
    serverApps = echoServer.Install(nodes.Get(1));
    serverApps.Start(Seconds(1));
    serverApps.Stop(Seconds(10));

    /* Create a UdpClient application to send UDP datagrams from node0 to node1 */
    /* Todo 6: Create a UdpClient using "UdpClientHelper" class 
    1) UdpClientHelper client (server IP address, server port number); 
    2) Get server IP from Ipv4InterfaceContainer "p2pInterfaces"*/
    UdpEchoClientHelper echoClient(interfaces.GetAddress(1), port);

    /* Todo 7: 
    1) Set packet interval as 'Seconds (0.01)' and packetsize as 100 using "SetAttribute" method
    2) Install the UdpClient on node0 using ApplicationContainer "apps" and Install method of UdpClientHelper */
    uint32_t maxPacketCount = 100000;
    Time packetInterval = Seconds(0.01);
    uint32_t PacketSize = 100;
    echoClient.SetAttribute("MaxPackets", UintegerValue (maxPacketCount));
    echoClient.SetAttribute("Interval", TimeValue(packetInterval));
    echoClient.SetAttribute("PacketSize", UintegerValue(PacketSize));
    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));

    /* Todo 8:  Start the client application from 1 to 10 sec */
    clientApps.Start(Seconds(1));
    clientApps.Stop(Seconds(10));

    /* Todo 9:  Enable pcap tracing */
    pointToPoint.EnablePcapAll("s3_inclass");
    /*
    Go ahead and insert this line of code after the ASCII tracing code we just added to scratch/s3_inclass.cc. Notice that we only passed the string “s3_inclass,” and not “s3_inclass.pcap” or something similar. This is because the parameter is a prefix, not a complete file name. The helper will actually create a trace file for every point-to-point device in the simulation. The file names will be built using the prefix, the node number, the device number and a “.pcap” suffix.

    In our example script, we will eventually see files named “s3_inclass-0-0.pcap” and “s3_inclass-1-0.pcap” which are the pcap traces for node 0-device 0 and node 1-device 0, respectively.
    */

    /* Running a simulator */
    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}

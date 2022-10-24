/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1
//    point-to-point
//

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FirstScriptExample"); // Definition of the logging component

int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    // Log information
    Time::SetResolution(Time::NS); // This is the defining of the Time the application is going to run
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO); // These two files are defining logging parameters and what level of logging to use
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);


    // Define the amount of nodes
    NodeContainer nodes;
    nodes.Create(2); 


    // Choose technology to communicate
    PointToPointHelper pointToPoint; 
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps")); 
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms")); 

    // Install the technology on the nodes 
    NetDeviceContainer devices; 
    devices = pointToPoint.Install(nodes);

    // This is used to make the notes follow internet rules, basically making them act like real protocols i believe
    InternetStackHelper stack;
    stack.Install(nodes);

    // Assign ip to use
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // Create a x type of server on port 9
    UdpEchoServerHelper echoServer(9);

    // Install server on a node then Start and stop server
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    // Create x type of client and set its attributes
    UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));


    // Install the client node then start and stop client.
    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));


    //Run the simulation:
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}

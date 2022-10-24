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
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 10.1.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SecondScriptExample");

int
main(int argc, char* argv[])
{
	// Default cmd Values
    bool verbose = true;
    uint32_t nCsma = 3;

    CommandLine cmd(__FILE__);
	// 
    cmd.AddValue("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);

    cmd.Parse(argc, argv);


	// If verbose is true, activate logging
    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

	// If nCsma is set to 0, set it to 1
    nCsma = nCsma == 0 ? 1 : nCsma;

	// Initialize p2pNodes NodeContainer
    NodeContainer p2pNodes;
    p2pNodes.Create(2);

	// Initialize nCsma Node container here it also adds one of the p2pNodes to the csma Container
    NodeContainer csmaNodes;
    csmaNodes.Add(p2pNodes.Get(1));
    csmaNodes.Create(nCsma);

	// Configure Technology P2P
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

	// Install technology on the p2pNodes
    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes);

	// Configure csma technology
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

	// Install technology on the csmaNodes
    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install(csmaNodes);

	// This is used to make the notes follow internet rules
    InternetStackHelper stack;
    stack.Install(p2pNodes.Get(0)); // Only one of the p2pNodes is installed here since the other one is contained within the csmaNodes
    stack.Install(csmaNodes);

	// Initilize the ips for p2pDevices
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);


	// Initialize the ips for the csmaDevices
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign(csmaDevices);

	// Create a x type of server on port 9
    UdpEchoServerHelper echoServer(9);

	// Initialize application container for server side nodes (the nCsma)
    ApplicationContainer serverApps = echoServer.Install(csmaNodes.Get(nCsma));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

	// Initialize the client side, here the ip's for which the client can send to is defined.
    UdpEchoClientHelper echoClient(csmaInterfaces.GetAddress(nCsma), 9);

	// Set some attributes for the client
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

	// Start the client side application
    ApplicationContainer clientApps = echoClient.Install(p2pNodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

	// Hmm idk
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	// Enable pcap
    pointToPoint.EnablePcapAll("second");
    // Enable pcap for only one device
	csma.EnablePcap("second", csmaDevices.Get(1), true);

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}

/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Yufei Cheng
 *
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
 *
 * Author: Yufei Cheng   <yfcheng@ittc.ku.edu>
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  https://resilinets.org/
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */

#ifndef WDSR_OPTION_H
#define WDSR_OPTION_H

#include "wdsr-gratuitous-reply-table.h"
#include "wdsr-maintain-buff.h"
#include "wdsr-option-header.h"
#include "wdsr-rcache.h"
#include "wdsr-routing.h"
#include "wdsr-rsendbuff.h"

#include "ns3/buffer.h"
#include "ns3/callback.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv4.h"
#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/timer.h"
#include "ns3/traced-callback.h"
#include "ns3/udp-l4-protocol.h"

#include <list>
#include <map>

namespace ns3
{

class Packet;
class NetDevice;
class Node;
class Ipv4Address;
class Ipv4Interface;
class Ipv4Route;
class Ipv4;
class Time;

namespace wdsr
{

class WDsrOptions : public Object
{
  public:
    /**
     * \brief Get the type identificator.
     * \return type identificator
     */
    static TypeId GetTypeId();
    /**
     * \brief Constructor.
     */
    WDsrOptions();
    /**
     * \brief Destructor.
     */
    ~WDsrOptions() override;
    /**
     * \brief Get the option number.
     * \return option number
     */
    virtual uint8_t GetOptionNumber() const = 0;
    /**
     * \brief Set the node.
     * \param node the node to set
     */
    void SetNode(Ptr<Node> node);
    /**
     * \brief Get the node.
     * \return the node
     */
    Ptr<Node> GetNode() const;
    /**
     * \brief Search for the ipv4 address in the node list.
     *
     * \param ipv4Address IPv4 address to search for
     * \param destAddress IPv4 address in the list that we begin the search
     * \param nodeList List of IPv4 addresses
     * \return true if contain ip address
     */
    bool ContainAddressAfter(Ipv4Address ipv4Address,
                             Ipv4Address destAddress,
                             std::vector<Ipv4Address>& nodeList);
    /**
     * \brief Cut the route from ipv4Address to the end of the route vector
     *
     * \param ipv4Address the address to begin cutting
     * \param nodeList List of IPv4 addresses
     * \return the vector after the route cut
     */
    std::vector<Ipv4Address> CutRoute(Ipv4Address ipv4Address, std::vector<Ipv4Address>& nodeList);
    /**
     * \brief Set the route to use for data packets,
     *        used by the option headers when sending data/control packets
     *
     * \param nextHop IPv4 address of the next hop
     * \param srcAddress IPv4 address of the source
     * \return the route
     */
    virtual Ptr<Ipv4Route> SetRoute(Ipv4Address nextHop, Ipv4Address srcAddress);
    /**
     * \brief Reverse the routes.
     *
     * \param vec List of IPv4 addresses
     * \return true if successfully reversed
     */
    bool ReverseRoutes(std::vector<Ipv4Address>& vec);
    /**
     * \brief Search for the next hop in the route
     *
     * \param ipv4Address the IPv4 address of the node we are looking for its next hop address
     * \param vec List of IPv4 addresses
     * \return the next hop address if found
     */
    Ipv4Address SearchNextHop(Ipv4Address ipv4Address, std::vector<Ipv4Address>& vec);
    /**
     * \brief Reverse search for the next hop in the route
     *
     * \param ipv4Address the IPv4 address of the node we are looking for its next hop address
     * \param vec List of IPv4 addresses
     * \return the previous next hop address if found
     */
    Ipv4Address ReverseSearchNextHop(Ipv4Address ipv4Address, std::vector<Ipv4Address>& vec);
    /**
     * \brief Reverse search for the next two hop in the route
     *
     * \param ipv4Address the IPv4 address of the node we are looking for its next two hop address
     * \param vec List of IPv4 addresses
     * \return the previous next two hop address if found
     */
    Ipv4Address ReverseSearchNextTwoHop(Ipv4Address ipv4Address, std::vector<Ipv4Address>& vec);
    /**
     * \brief Print out the elements in the route vector
     * \param vec The route vector to print.
     */
    void PrintVector(std::vector<Ipv4Address>& vec);
    /**
     * \brief Check if the two vectors contain duplicate or not
     *
     * \param vec the first list of IPv4 addresses
     * \param vec2 the second list of IPv4 addresses
     * \return true if contains duplicate
     */
    bool IfDuplicates(std::vector<Ipv4Address>& vec, std::vector<Ipv4Address>& vec2);
    /**
     * \brief Check if the route already contains the node ip address
     *
     * \param ipv4Address the IPv4 address that we are looking for
     * \param vec List of IPv4 addresses
     * \return true if it already exists
     */
    bool CheckDuplicates(Ipv4Address ipv4Address, std::vector<Ipv4Address>& vec);
    /**
     * \brief Remove the duplicates from the route
     *
     * \param [in,out] vec List of IPv4 addresses to clean
     */
    void RemoveDuplicates(std::vector<Ipv4Address>& vec);
    /**
     * \brief Schedule the intermediate node route request broadcast
     * \param packet the original packet
     * \param nodeList The list of IPv4 addresses
     * \param source address
     * \param destination address
     */
    void ScheduleReply(Ptr<Packet>& packet,
                       std::vector<Ipv4Address>& nodeList,
                       Ipv4Address& source,
                       Ipv4Address& destination);
    /**
     * \brief Get the node id with Ipv4Address
     *
     * \param address IPv4 address to look for ID
     * \return the id of the node
     */
    uint32_t GetIDfromIP(Ipv4Address address);
    /**
     * \brief Get the node object with Ipv4Address
     *
     * \param ipv4Address IPv4 address of the node
     * \return the object of the node
     */
    Ptr<Node> GetNodeWithAddress(Ipv4Address ipv4Address);
    /**
     * \brief Process method
     *
     * Called from WDsrRouting::Receive.
     * \param packet the packet
     * \param wdsrP  the clean packet with payload
     * \param ipv4Address the IPv4 address
     * \param source IPv4 address of the source
     * \param ipv4Header the IPv4 header of packet received
     * \param protocol the protocol number of the up layer
     * \param isPromisc if the packet must be dropped
     * \param promiscSource IPv4 address
     * \return the processed size
     */
    virtual uint8_t Process(Ptr<Packet> packet,
                            Ptr<Packet> wdsrP,
                            Ipv4Address ipv4Address,
                            Ipv4Address source,
                            const Ipv4Header& ipv4Header,
                            uint8_t protocol,
                            bool& isPromisc,
                            Ipv4Address promiscSource) = 0;

  protected:
    /**
     * \brief Drop trace callback.
     */
    TracedCallback<Ptr<const Packet>> m_dropTrace;
    /**
     * \brief The broadcast IP address.
     */
    Ipv4Address Broadcast;
    /**
     * \brief The route request table.
     */
    Ptr<wdsr::WDsrRreqTable> m_rreqTable;
    /**
     * \brief The route cache table.
     */
    Ptr<wdsr::WDsrRouteCache> m_routeCache;
    /**
     * \brief The ipv4 route.
     */
    Ptr<Ipv4Route> m_ipv4Route;
    /**
     * \brief The ipv4.
     */
    Ptr<Ipv4> m_ipv4;
    /**
     * \brief The vector of Ipv4 address.
     */
    std::vector<Ipv4Address> m_ipv4Address;
    /**
     * \brief The vector of final Ipv4 address.
     */
    std::vector<Ipv4Address> m_finalRoute;
    /**
     * \brief The active route timeout value.
     */
    Time ActiveRouteTimeout;
    /**
     * The receive trace back, only triggered when final destination receive data packet
     */
    TracedCallback<const WDsrOptionSRHeader&> m_rxPacketTrace;

  private:
    Ptr<Node> m_node; ///< the node
};

/**
 * \class WDsrOptionPad1
 * \brief WDsr Option Pad1
 */
class WDsrOptionPad1 : public WDsrOptions
{
  public:
    /**
     * \brief Pad1 option number.
     */
    static const uint8_t OPT_NUMBER = 224;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    WDsrOptionPad1();
    ~WDsrOptionPad1() override;

    uint8_t GetOptionNumber() const override;
    uint8_t Process(Ptr<Packet> packet,
                    Ptr<Packet> wdsrP,
                    Ipv4Address ipv4Address,
                    Ipv4Address source,
                    const Ipv4Header& ipv4Header,
                    uint8_t protocol,
                    bool& isPromisc,
                    Ipv4Address promiscSource) override;
};

/**
 * \class WDsrOptionPadn
 * \brief IPv4 Option Padn
 */
class WDsrOptionPadn : public WDsrOptions
{
  public:
    /**
     * \brief PadN option number.
     */
    static const uint8_t OPT_NUMBER = 0;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    WDsrOptionPadn();
    ~WDsrOptionPadn() override;

    uint8_t GetOptionNumber() const override;
    uint8_t Process(Ptr<Packet> packet,
                    Ptr<Packet> wdsrP,
                    Ipv4Address ipv4Address,
                    Ipv4Address source,
                    const Ipv4Header& ipv4Header,
                    uint8_t protocol,
                    bool& isPromisc,
                    Ipv4Address promiscSource) override;
};

/**
 * \class WDsrOptionRreq
 * \brief WDsr Option Rreq
 */
class WDsrOptionRreq : public WDsrOptions
{
  public:
    /**
     * \brief Rreq option number.
     */
    static const uint8_t OPT_NUMBER = 1;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    /**
     * \brief Get the instance type ID.
     * \return instance type ID
     */
    TypeId GetInstanceTypeId() const override;
    /**
     * \brief Constructor.
     */
    WDsrOptionRreq();
    /**
     * \brief Destructor.
     */
    ~WDsrOptionRreq() override;

    uint8_t GetOptionNumber() const override;
    uint8_t Process(Ptr<Packet> packet,
                    Ptr<Packet> wdsrP,
                    Ipv4Address ipv4Address,
                    Ipv4Address source,
                    const Ipv4Header& ipv4Header,
                    uint8_t protocol,
                    bool& isPromisc,
                    Ipv4Address promiscSource) override;

  private:
    /**
     * \brief The route cache.
     */
    Ptr<wdsr::WDsrRouteCache> m_routeCache;
    /**
     * \brief The ipv4.
     */
    Ptr<Ipv4> m_ipv4;
};

/**
 * \class WDsrOptionRrep
 * \brief WDsr Option Route Reply
 */
class WDsrOptionRrep : public WDsrOptions
{
  public:
    /**
     * \brief Router alert option number.
     */
    static const uint8_t OPT_NUMBER = 2;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    /**
     * \brief Get the instance type ID.
     * \return instance type ID
     */
    TypeId GetInstanceTypeId() const override;

    WDsrOptionRrep();
    ~WDsrOptionRrep() override;

    uint8_t GetOptionNumber() const override;
    uint8_t Process(Ptr<Packet> packet,
                    Ptr<Packet> wdsrP,
                    Ipv4Address ipv4Address,
                    Ipv4Address source,
                    const Ipv4Header& ipv4Header,
                    uint8_t protocol,
                    bool& isPromisc,
                    Ipv4Address promiscSource) override;

  private:
    /**
     * \brief The route cache.
     */
    Ptr<wdsr::WDsrRouteCache> m_routeCache;
    /**
     * \brief The ip layer 3.
     */
    Ptr<Ipv4> m_ipv4;
};

/**
 * \class WDsrOptionSR
 * \brief WDsr Option Source Route
 */
class WDsrOptionSR : public WDsrOptions
{
  public:
    /**
     * \brief Source Route option number.
     */
    static const uint8_t OPT_NUMBER = 96;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    /**
     * \brief Get the instance type ID.
     * \return instance type ID
     */
    TypeId GetInstanceTypeId() const override;

    WDsrOptionSR();
    ~WDsrOptionSR() override;

    uint8_t GetOptionNumber() const override;
    uint8_t Process(Ptr<Packet> packet,
                    Ptr<Packet> wdsrP,
                    Ipv4Address ipv4Address,
                    Ipv4Address source,
                    const Ipv4Header& ipv4Header,
                    uint8_t protocol,
                    bool& isPromisc,
                    Ipv4Address promiscSource) override;

  private:
    /**
     * \brief The ip layer 3.
     */
    Ptr<Ipv4> m_ipv4;
};

/**
 * \class WDsrOptionRerr
 * \brief WDsr Option Route Error
 */
class WDsrOptionRerr : public WDsrOptions
{
  public:
    /**
     * \brief WDsr Route Error option number.
     */
    static const uint8_t OPT_NUMBER = 3;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    /**
     * \brief Get the instance type ID.
     * \return instance type ID
     */
    TypeId GetInstanceTypeId() const override;

    WDsrOptionRerr();
    ~WDsrOptionRerr() override;

    uint8_t GetOptionNumber() const override;
    uint8_t Process(Ptr<Packet> packet,
                    Ptr<Packet> wdsrP,
                    Ipv4Address ipv4Address,
                    Ipv4Address source,
                    const Ipv4Header& ipv4Header,
                    uint8_t protocol,
                    bool& isPromisc,
                    Ipv4Address promiscSource) override;
    /**
     * \brief Do Send error message
     *
     * \param p the packet
     * \param rerr  the WDsrOptionRerrUnreachHeader header
     * \param rerrSize the route error header size
     * \param ipv4Address ipv4 address of our own
     * \param protocol the protocol number of the up layer
     * \return the processed size
     */
    uint8_t DoSendError(Ptr<Packet> p,
                        WDsrOptionRerrUnreachHeader& rerr,
                        uint32_t rerrSize,
                        Ipv4Address ipv4Address,
                        uint8_t protocol);

  private:
    /**
     * \brief The route cache.
     */
    Ptr<wdsr::WDsrRouteCache> m_routeCache;
    /**
     * \brief The ipv4 layer 3.
     */
    Ptr<Ipv4> m_ipv4;
};

/**
 * \class WDsrOptionAckReq
 * \brief WDsr Option
 */
class WDsrOptionAckReq : public WDsrOptions
{
  public:
    /**
     * \brief WDsr ack request option number.
     */
    static const uint8_t OPT_NUMBER = 160;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    /**
     * \brief Get the instance type ID.
     * \return instance type ID
     */
    TypeId GetInstanceTypeId() const override;

    WDsrOptionAckReq();
    ~WDsrOptionAckReq() override;

    uint8_t GetOptionNumber() const override;
    uint8_t Process(Ptr<Packet> packet,
                    Ptr<Packet> wdsrP,
                    Ipv4Address ipv4Address,
                    Ipv4Address source,
                    const Ipv4Header& ipv4Header,
                    uint8_t protocol,
                    bool& isPromisc,
                    Ipv4Address promiscSource) override;

  private:
    /**
     * \brief The route cache.
     */
    Ptr<wdsr::WDsrRouteCache> m_routeCache;
    /**
     * \brief The ipv4 layer 3.
     */
    Ptr<Ipv4> m_ipv4;
};

/**
 * \class WDsrOptionAck
 * \brief WDsr Option Ack
 */
class WDsrOptionAck : public WDsrOptions
{
  public:
    /**
     * \brief The WDsr Ack option number.
     */
    static const uint8_t OPT_NUMBER = 32;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    /**
     * \brief Get the instance type ID.
     * \return instance type ID
     */
    TypeId GetInstanceTypeId() const override;

    WDsrOptionAck();
    ~WDsrOptionAck() override;

    uint8_t GetOptionNumber() const override;
    uint8_t Process(Ptr<Packet> packet,
                    Ptr<Packet> wdsrP,
                    Ipv4Address ipv4Address,
                    Ipv4Address source,
                    const Ipv4Header& ipv4Header,
                    uint8_t protocol,
                    bool& isPromisc,
                    Ipv4Address promiscSource) override;

  private:
    /**
     * \brief The route cache.
     */
    Ptr<wdsr::WDsrRouteCache> m_routeCache;
    /**
     * \brief The ipv4 layer 3.
     */
    Ptr<Ipv4> m_ipv4;
};
} // namespace wdsr
} // Namespace ns3

#endif

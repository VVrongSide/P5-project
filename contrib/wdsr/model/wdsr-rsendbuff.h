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

#ifndef WDSR_SENDBUFF_H
#define WDSR_SENDBUFF_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/simulator.h"

#include <vector>

namespace ns3
{
namespace wdsr
{
/**
 * \ingroup wdsr
 * \brief WDSR Send Buffer Entry
 */
class WDsrSendBuffEntry
{
  public:
    /**
     * Construct WDsrSendBuffEntry with the given parameters.
     *
     * \param pa packet
     * \param d destination address
     * \param exp expiration time
     * \param p protocol number
     */
    WDsrSendBuffEntry(Ptr<const Packet> pa = nullptr,
                     Ipv4Address d = Ipv4Address(),
                     Time exp = Simulator::Now(),
                     uint8_t p = 0)
        : m_packet(pa),
          m_dst(d),
          m_expire(exp + Simulator::Now()),
          m_protocol(p)
    {
    }

    /**
     * Compare send buffer entries
     * \param o another WDsrSendBuffEntry
     * \return true if equal
     */
    bool operator==(const WDsrSendBuffEntry& o) const
    {
        return ((m_packet == o.m_packet) && (m_dst == o.m_dst) && (m_expire == o.m_expire));
    }

    // Fields
    /**
     * Get pointer to entry's packet
     * \returns the current packet
     */
    Ptr<const Packet> GetPacket() const
    {
        return m_packet;
    }

    /**
     * Set pointer to entry's packet
     * \param p the current packet
     */
    void SetPacket(Ptr<const Packet> p)
    {
        m_packet = p;
    }

    /**
     * Get destination address of entry
     * \returns the destination IPv4 address
     */
    Ipv4Address GetDestination() const
    {
        return m_dst;
    }

    /**
     * Set destination address of entry
     * \param d the destination IP address
     */
    void SetDestination(Ipv4Address d)
    {
        m_dst = d;
    }

    /**
     * Set expire time for entry
     * \param exp the expire time
     */
    void SetExpireTime(Time exp)
    {
        m_expire = exp + Simulator::Now();
    }

    /**
     * Get expire time for entry
     * \returns the expire time
     */
    Time GetExpireTime() const
    {
        return m_expire - Simulator::Now();
    }

    /**
     * Set protocol value
     * \param p the protocol
     */
    void SetProtocol(uint8_t p)
    {
        m_protocol = p;
    }

    /**
     * Get protocol value
     * \returns the protocol
     */
    uint8_t GetProtocol() const
    {
        return m_protocol;
    }

  private:
    /// Data packet
    Ptr<const Packet> m_packet;
    /// Destination address
    Ipv4Address m_dst;
    /// Expire time for queue entry
    Time m_expire;
    /// The protocol number
    uint8_t m_protocol;
};

/**
 * \ingroup wdsr
 * \brief WDSR send buffer
 */
/************************************************************************************************************************/
class WDsrSendBuffer
{
  public:
    /**
     * Default constructor
     */
    WDsrSendBuffer()
    {
    }

    /**
     * Push entry in queue, if there is no entry with
     * the same packet and destination address in queue.
     *
     * \param entry WDsrSendBuffEntry to put in the queue
     * \return true if successfully enqueued,
     *         false otherwise
     */
    bool Enqueue(WDsrSendBuffEntry& entry);
    /**
     * Return first found (the earliest) entry for
     * the given destination.
     *
     * \param dst IPv4 address of the destination
     * \param entry pointer to entry to return
     * \return true if successfully dequeued,
     *         false otherwise
     */
    bool Dequeue(Ipv4Address dst, WDsrSendBuffEntry& entry);
    /**
     * Remove all packets with destination IP address dst
     *
     * \param dst IPv4 address of the destination
     */
    void DropPacketWithDst(Ipv4Address dst);
    /**
     * Check if a packet with destination dst exists in the queue
     *
     * \param dst IPv4 address of the destination
     * \return true if found, false otherwise
     */
    bool Find(Ipv4Address dst);
    /**
     * Number of entries
     *
     * \return the number of entries in the queue
     */
    uint32_t GetSize();

    /**
     * Return the maximum queue length
     *
     * \return the maximum queue length
     */
    uint32_t GetMaxQueueLen() const
    {
        return m_maxLen;
    }

    /**
     * Set the maximum queue length
     *
     * \param len the maximum queue length
     */
    void SetMaxQueueLen(uint32_t len)
    {
        m_maxLen = len;
    }

    /**
     * Return the entry lifetime in the queue
     *
     * \return the entry lifetime in the queue
     */
    Time GetSendBufferTimeout() const
    {
        return m_sendBufferTimeout;
    }

    /**
     * Set the entry lifetime in the queue
     *
     * \param t the entry lifetime in the queue
     */
    void SetSendBufferTimeout(Time t)
    {
        m_sendBufferTimeout = t;
    }

    // \}

    /**
     * Return a pointer to the internal queue
     *
     * \return a pointer to the internal queue
     */
    std::vector<WDsrSendBuffEntry>& GetBuffer()
    {
        return m_sendBuffer;
    }

  private:
    std::vector<WDsrSendBuffEntry> m_sendBuffer; ///< The send buffer to cache unsent packet
    void Purge();                               ///< Remove all expired entries

    /// Notify that packet is dropped from queue by timeout
    /// \param en BuffEntry Buffer entry
    /// \param reason Drop reason
    void Drop(WDsrSendBuffEntry en, std::string reason);

    uint32_t
        m_maxLen; ///< The maximum number of packets that we allow a routing protocol to buffer.
    Time m_sendBufferTimeout; ///< The maximum period of time that a routing protocol is allowed to
                              ///< buffer a packet for, seconds.
};

/*******************************************************************************************************************************/
} // namespace wdsr
} // namespace ns3

#endif /* WDSR_SENDBUFF_H */

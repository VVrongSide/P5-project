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

#include "wdsr-option-header.h"

#include "ns3/address-utils.h"
#include "ns3/assert.h"
#include "ns3/enum.h"
#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/log.h"
#include "ns3/packet.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("WDsrOptionHeader");

namespace wdsr
{

NS_OBJECT_ENSURE_REGISTERED(WDsrOptionHeader);

TypeId
WDsrOptionHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::wdsr::WDsrOptionHeader")
                            .AddConstructor<WDsrOptionHeader>()
                            .SetParent<Header>()
                            .SetGroupName("WDsr");
    return tid;
}

TypeId
WDsrOptionHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

WDsrOptionHeader::WDsrOptionHeader()
    : m_type(0),
      m_length(0)
{
}

WDsrOptionHeader::~WDsrOptionHeader()
{
}

void
WDsrOptionHeader::SetType(uint8_t type)
{
    m_type = type;
}

uint8_t
WDsrOptionHeader::GetType() const
{
    return m_type;
}

void
WDsrOptionHeader::SetLength(uint8_t length)
{
    m_length = length;
}

uint8_t
WDsrOptionHeader::GetLength() const
{
    return m_length;
}

void
WDsrOptionHeader::Print(std::ostream& os) const
{
    os << "( type = " << (uint32_t)m_type << " length = " << (uint32_t)m_length << " )";
}

uint32_t
WDsrOptionHeader::GetSerializedSize() const
{
    return m_length + 2;
}

void
WDsrOptionHeader::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    i.WriteU8(m_type);
    i.WriteU8(m_length);
    i.Write(m_data.Begin(), m_data.End());
}

uint32_t
WDsrOptionHeader::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;

    m_type = i.ReadU8();
    m_length = i.ReadU8();

    m_data = Buffer();
    m_data.AddAtEnd(m_length);
    Buffer::Iterator dataStart = i;
    i.Next(m_length);
    Buffer::Iterator dataEnd = i;
    m_data.Begin().Write(dataStart, dataEnd);

    return GetSerializedSize();
}

WDsrOptionHeader::Alignment
WDsrOptionHeader::GetAlignment() const
{
    Alignment retVal = {1, 0};
    return retVal;
}

NS_OBJECT_ENSURE_REGISTERED(WDsrOptionPad1Header);

TypeId
WDsrOptionPad1Header::GetTypeId()
{
    static TypeId tid = TypeId("ns3::wdsr::WDsrOptionPad1Header")
                            .AddConstructor<WDsrOptionPad1Header>()
                            .SetParent<WDsrOptionHeader>()
                            .SetGroupName("WDsr");
    return tid;
}

TypeId
WDsrOptionPad1Header::GetInstanceTypeId() const
{
    return GetTypeId();
}

WDsrOptionPad1Header::WDsrOptionPad1Header()
{
    SetType(224);
}

WDsrOptionPad1Header::~WDsrOptionPad1Header()
{
}

void
WDsrOptionPad1Header::Print(std::ostream& os) const
{
    os << "( type = " << (uint32_t)GetType() << " )";
}

uint32_t
WDsrOptionPad1Header::GetSerializedSize() const
{
    return 1;
}

void
WDsrOptionPad1Header::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    i.WriteU8(GetType());
}

uint32_t
WDsrOptionPad1Header::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;

    SetType(i.ReadU8());

    return GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED(WDsrOptionPadnHeader);

TypeId
WDsrOptionPadnHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::wdsr::WDsrOptionPadnHeader")
                            .AddConstructor<WDsrOptionPadnHeader>()
                            .SetParent<WDsrOptionHeader>()
                            .SetGroupName("WDsr");
    return tid;
}

TypeId
WDsrOptionPadnHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

WDsrOptionPadnHeader::WDsrOptionPadnHeader(uint32_t pad)
{
    SetType(0);
    NS_ASSERT_MSG(pad >= 2, "PadN must be at least 2 bytes long");
    SetLength(pad - 2);
}

WDsrOptionPadnHeader::~WDsrOptionPadnHeader()
{
}

void
WDsrOptionPadnHeader::Print(std::ostream& os) const
{
    os << "( type = " << (uint32_t)GetType() << " length = " << (uint32_t)GetLength() << " )";
}

uint32_t
WDsrOptionPadnHeader::GetSerializedSize() const
{
    return GetLength() + 2;
}

void
WDsrOptionPadnHeader::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    i.WriteU8(GetType());
    i.WriteU8(GetLength());

    for (int padding = 0; padding < GetLength(); padding++)
    {
        i.WriteU8(0);
    }
}

uint32_t
WDsrOptionPadnHeader::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;

    SetType(i.ReadU8());
    SetLength(i.ReadU8());

    return GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED(WDsrOptionRreqHeader);

TypeId
WDsrOptionRreqHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::wdsr::WDsrOptionRreqHeader")
                            .AddConstructor<WDsrOptionRreqHeader>()
                            .SetParent<WDsrOptionHeader>()
                            .SetGroupName("WDsr");
    return tid;
}

TypeId
WDsrOptionRreqHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

WDsrOptionRreqHeader::WDsrOptionRreqHeader()
    : m_ipv4Address(0)
{
    SetType(1);
    SetLength(6 + m_ipv4Address.size() * 4);
}

WDsrOptionRreqHeader::~WDsrOptionRreqHeader()
{
}

void
WDsrOptionRreqHeader::SetNumberAddress(uint8_t n)
{
    m_ipv4Address.clear();
    m_ipv4Address.assign(n, Ipv4Address());
}

Ipv4Address
WDsrOptionRreqHeader::GetTarget()
{
    return m_target;
}

void
WDsrOptionRreqHeader::SetTarget(Ipv4Address target)
{
    m_target = target;
}

void
WDsrOptionRreqHeader::AddNodeAddress(Ipv4Address ipv4)
{
    m_ipv4Address.push_back(ipv4);
    SetLength(6 + m_ipv4Address.size() * 4);
}

void
WDsrOptionRreqHeader::SetNodesAddress(std::vector<Ipv4Address> ipv4Address)
{
    m_ipv4Address = ipv4Address;
    SetLength(6 + m_ipv4Address.size() * 4);
}

std::vector<Ipv4Address>
WDsrOptionRreqHeader::GetNodesAddresses() const
{
    return m_ipv4Address;
}

uint32_t
WDsrOptionRreqHeader::GetNodesNumber() const
{
    return m_ipv4Address.size();
}

void
WDsrOptionRreqHeader::SetNodeAddress(uint8_t index, Ipv4Address addr)
{
    m_ipv4Address.at(index) = addr;
}

Ipv4Address
WDsrOptionRreqHeader::GetNodeAddress(uint8_t index) const
{
    return m_ipv4Address.at(index);
}

void
WDsrOptionRreqHeader::SetId(uint16_t identification)
{
    m_identification = identification;
}

uint16_t
WDsrOptionRreqHeader::GetId() const
{
    return m_identification;
}

void
WDsrOptionRreqHeader::Print(std::ostream& os) const
{
    os << "( type = " << (uint32_t)GetType() << " length = " << (uint32_t)GetLength() << "";

    for (std::vector<Ipv4Address>::const_iterator it = m_ipv4Address.begin();
         it != m_ipv4Address.end();
         it++)
    {
        os << *it << " ";
    }

    os << ")";
}

uint32_t
WDsrOptionRreqHeader::GetSerializedSize() const
{
    return 8 + m_ipv4Address.size() * 4;
}

void
WDsrOptionRreqHeader::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;
    uint8_t buff[4];

    i.WriteU8(GetType());
    i.WriteU8(GetLength());
    i.WriteHtonU16(m_identification);
    WriteTo(i, m_target);

    for (VectorIpv4Address_t::const_iterator it = m_ipv4Address.begin(); it != m_ipv4Address.end();
         it++)
    {
        it->Serialize(buff);
        i.Write(buff, 4);
    }
}

uint32_t
WDsrOptionRreqHeader::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    uint8_t buff[4];

    SetType(i.ReadU8());
    SetLength(i.ReadU8());
    m_identification = i.ReadNtohU16();
    ReadFrom(i, m_target);

    uint8_t index = 0;
    for (std::vector<Ipv4Address>::iterator it = m_ipv4Address.begin(); it != m_ipv4Address.end();
         it++)
    {
        i.Read(buff, 4);
        m_address = it->Deserialize(buff);
        SetNodeAddress(index, m_address);
        ++index;
    }

    return GetSerializedSize();
}

WDsrOptionHeader::Alignment
WDsrOptionRreqHeader::GetAlignment() const
{
    Alignment retVal = {4, 0};
    return retVal;
}

NS_OBJECT_ENSURE_REGISTERED(WDsrOptionRrepHeader);

TypeId
WDsrOptionRrepHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::wdsr::WDsrOptionRrepHeader")
                            .AddConstructor<WDsrOptionRrepHeader>()
                            .SetParent<WDsrOptionHeader>()
                            .SetGroupName("WDsr");
    return tid;
}

TypeId
WDsrOptionRrepHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

WDsrOptionRrepHeader::WDsrOptionRrepHeader()
    : m_ipv4Address(0)
{
    SetType(2);
    SetLength(2 + m_ipv4Address.size() * 4);
}

WDsrOptionRrepHeader::~WDsrOptionRrepHeader()
{
}

void
WDsrOptionRrepHeader::SetNumberAddress(uint8_t n)
{
    m_ipv4Address.clear();
    m_ipv4Address.assign(n, Ipv4Address());
}

void
WDsrOptionRrepHeader::SetNodesAddress(std::vector<Ipv4Address> ipv4Address)
{
    m_ipv4Address = ipv4Address;
    SetLength(2 + m_ipv4Address.size() * 4);
}

std::vector<Ipv4Address>
WDsrOptionRrepHeader::GetNodesAddress() const
{
    return m_ipv4Address;
}

void
WDsrOptionRrepHeader::SetNodeAddress(uint8_t index, Ipv4Address addr)
{
    m_ipv4Address.at(index) = addr;
}

Ipv4Address
WDsrOptionRrepHeader::GetNodeAddress(uint8_t index) const
{
    return m_ipv4Address.at(index);
}

Ipv4Address
WDsrOptionRrepHeader::GetTargetAddress(std::vector<Ipv4Address> ipv4Address) const
{
    return m_ipv4Address.at(ipv4Address.size() - 1);
}

void
WDsrOptionRrepHeader::Print(std::ostream& os) const
{
    os << "( type = " << (uint32_t)GetType() << " length = " << (uint32_t)GetLength() << "";

    for (std::vector<Ipv4Address>::const_iterator it = m_ipv4Address.begin();
         it != m_ipv4Address.end();
         it++)
    {
        os << *it << " ";
    }

    os << ")";
}

uint32_t
WDsrOptionRrepHeader::GetSerializedSize() const
{
    return 4 + m_ipv4Address.size() * 4;
}

void
WDsrOptionRrepHeader::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;
    uint8_t buff[4];

    i.WriteU8(GetType());
    i.WriteU8(GetLength());
    i.WriteU8(0);
    i.WriteU8(0);

    for (VectorIpv4Address_t::const_iterator it = m_ipv4Address.begin(); it != m_ipv4Address.end();
         it++)
    {
        it->Serialize(buff);
        i.Write(buff, 4);
    }
}

uint32_t
WDsrOptionRrepHeader::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    uint8_t buff[4];

    SetType(i.ReadU8());
    SetLength(i.ReadU8());
    i.ReadU8();
    i.ReadU8();

    uint8_t index = 0;
    for (std::vector<Ipv4Address>::iterator it = m_ipv4Address.begin(); it != m_ipv4Address.end();
         it++)
    {
        i.Read(buff, 4);
        m_address = it->Deserialize(buff);
        SetNodeAddress(index, m_address);
        ++index;
    }

    return GetSerializedSize();
}

WDsrOptionHeader::Alignment
WDsrOptionRrepHeader::GetAlignment() const
{
    Alignment retVal = {4, 0};
    return retVal;
}

NS_OBJECT_ENSURE_REGISTERED(WDsrOptionSRHeader);

TypeId
WDsrOptionSRHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::wdsr::WDsrOptionSRHeader")
                            .AddConstructor<WDsrOptionSRHeader>()
                            .SetParent<WDsrOptionHeader>()
                            .SetGroupName("WDsr");
    return tid;
}

TypeId
WDsrOptionSRHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

WDsrOptionSRHeader::WDsrOptionSRHeader()
    : m_segmentsLeft(0),
      m_ipv4Address(0)
{
    SetType(96);
    SetLength(2 + m_ipv4Address.size() * 4);
}

WDsrOptionSRHeader::~WDsrOptionSRHeader()
{
}

void
WDsrOptionSRHeader::SetSegmentsLeft(uint8_t segmentsLeft)
{
    m_segmentsLeft = segmentsLeft;
}

uint8_t
WDsrOptionSRHeader::GetSegmentsLeft() const
{
    return m_segmentsLeft;
}

void
WDsrOptionSRHeader::SetSalvage(uint8_t salvage)
{
    m_salvage = salvage;
}

uint8_t
WDsrOptionSRHeader::GetSalvage() const
{
    return m_salvage;
}

void
WDsrOptionSRHeader::SetNumberAddress(uint8_t n)
{
    m_ipv4Address.clear();
    m_ipv4Address.assign(n, Ipv4Address());
}

void
WDsrOptionSRHeader::SetNodesAddress(std::vector<Ipv4Address> ipv4Address)
{
    m_ipv4Address = ipv4Address;
    SetLength(2 + m_ipv4Address.size() * 4);
}

std::vector<Ipv4Address>
WDsrOptionSRHeader::GetNodesAddress() const
{
    return m_ipv4Address;
}

void
WDsrOptionSRHeader::SetNodeAddress(uint8_t index, Ipv4Address addr)
{
    m_ipv4Address.at(index) = addr;
}

Ipv4Address
WDsrOptionSRHeader::GetNodeAddress(uint8_t index) const
{
    return m_ipv4Address.at(index);
}

uint8_t
WDsrOptionSRHeader::GetNodeListSize() const
{
    return m_ipv4Address.size();
}

void
WDsrOptionSRHeader::Print(std::ostream& os) const
{
    os << "( type = " << (uint32_t)GetType() << " length = " << (uint32_t)GetLength() << "";

    for (std::vector<Ipv4Address>::const_iterator it = m_ipv4Address.begin();
         it != m_ipv4Address.end();
         it++)
    {
        os << *it << " ";
    }

    os << ")";
}

uint32_t
WDsrOptionSRHeader::GetSerializedSize() const
{
    return 4 + m_ipv4Address.size() * 4;
}

void
WDsrOptionSRHeader::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;
    uint8_t buff[4];

    i.WriteU8(GetType());
    i.WriteU8(GetLength());
    i.WriteU8(m_salvage);
    i.WriteU8(m_segmentsLeft);

    for (VectorIpv4Address_t::const_iterator it = m_ipv4Address.begin(); it != m_ipv4Address.end();
         it++)
    {
        it->Serialize(buff);
        i.Write(buff, 4);
    }
}

uint32_t
WDsrOptionSRHeader::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    uint8_t buff[4];

    SetType(i.ReadU8());
    SetLength(i.ReadU8());
    m_salvage = i.ReadU8();
    m_segmentsLeft = i.ReadU8();

    uint8_t index = 0;
    for (std::vector<Ipv4Address>::iterator it = m_ipv4Address.begin(); it != m_ipv4Address.end();
         it++)
    {
        i.Read(buff, 4);
        m_address = it->Deserialize(buff);
        SetNodeAddress(index, m_address);
        ++index;
    }

    return GetSerializedSize();
}

WDsrOptionHeader::Alignment
WDsrOptionSRHeader::GetAlignment() const
{
    Alignment retVal = {4, 0};
    return retVal;
}

NS_OBJECT_ENSURE_REGISTERED(WDsrOptionRerrHeader);

TypeId
WDsrOptionRerrHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::wdsr::WDsrOptionRerrHeader")
                            .AddConstructor<WDsrOptionRerrHeader>()
                            .SetParent<WDsrOptionHeader>()
                            .SetGroupName("WDsr");
    return tid;
}

TypeId
WDsrOptionRerrHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

WDsrOptionRerrHeader::WDsrOptionRerrHeader()
    : m_errorType(0),
      m_salvage(0),
      m_errorLength(4)
{
    SetType(3);
    SetLength(18);
}

WDsrOptionRerrHeader::~WDsrOptionRerrHeader()
{
}

void
WDsrOptionRerrHeader::SetErrorType(uint8_t errorType)
{
    m_errorType = errorType;
}

uint8_t
WDsrOptionRerrHeader::GetErrorType() const
{
    return m_errorType;
}

void
WDsrOptionRerrHeader::SetSalvage(uint8_t salvage)
{
    m_salvage = salvage;
}

uint8_t
WDsrOptionRerrHeader::GetSalvage() const
{
    return m_salvage;
}

void
WDsrOptionRerrHeader::SetErrorSrc(Ipv4Address errorSrcAddress)
{
    m_errorSrcAddress = errorSrcAddress;
}

Ipv4Address
WDsrOptionRerrHeader::GetErrorSrc() const
{
    return m_errorSrcAddress;
}

void
WDsrOptionRerrHeader::SetErrorDst(Ipv4Address errorDstAddress)
{
    m_errorDstAddress = errorDstAddress;
}

Ipv4Address
WDsrOptionRerrHeader::GetErrorDst() const
{
    return m_errorDstAddress;
}

void
WDsrOptionRerrHeader::Print(std::ostream& os) const
{
    os << "( type = " << (uint32_t)GetType() << " length = " << (uint32_t)GetLength()
       << " errorType = " << (uint32_t)m_errorType << " salvage = " << (uint32_t)m_salvage
       << " error source = " << m_errorSrcAddress << " error dst = " << m_errorDstAddress << " )";
}

uint32_t
WDsrOptionRerrHeader::GetSerializedSize() const
{
    return 20;
}

void
WDsrOptionRerrHeader::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    i.WriteU8(GetType());
    i.WriteU8(GetLength());
    i.WriteU8(m_errorType);
    i.WriteU8(m_salvage);
    WriteTo(i, m_errorSrcAddress);
    WriteTo(i, m_errorDstAddress);
    i.Write(m_errorData.Begin(), m_errorData.End());
}

uint32_t
WDsrOptionRerrHeader::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;

    SetType(i.ReadU8());
    SetLength(i.ReadU8());
    m_errorType = i.ReadU8();
    m_salvage = i.ReadU8();
    ReadFrom(i, m_errorSrcAddress);
    ReadFrom(i, m_errorDstAddress);

    m_errorData = Buffer();
    m_errorData.AddAtEnd(m_errorLength);
    Buffer::Iterator dataStart = i;
    i.Next(m_errorLength);
    Buffer::Iterator dataEnd = i;
    m_errorData.Begin().Write(dataStart, dataEnd);

    return GetSerializedSize();
}

WDsrOptionHeader::Alignment
WDsrOptionRerrHeader::GetAlignment() const
{
    Alignment retVal = {4, 0};
    return retVal;
}

NS_OBJECT_ENSURE_REGISTERED(WDsrOptionRerrUnreachHeader);

TypeId
WDsrOptionRerrUnreachHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::wdsr::WDsrOptionRerrUnreachHeader")
                            .AddConstructor<WDsrOptionRerrUnreachHeader>()
                            .SetParent<WDsrOptionRerrHeader>()
                            .SetGroupName("WDsr");
    return tid;
}

TypeId
WDsrOptionRerrUnreachHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

WDsrOptionRerrUnreachHeader::WDsrOptionRerrUnreachHeader()
    : m_salvage(0)
{
    SetType(3);
    SetLength(18);
    SetErrorType(1);
}

WDsrOptionRerrUnreachHeader::~WDsrOptionRerrUnreachHeader()
{
}

void
WDsrOptionRerrUnreachHeader::SetSalvage(uint8_t salvage)
{
    m_salvage = salvage;
}

uint8_t
WDsrOptionRerrUnreachHeader::GetSalvage() const
{
    return m_salvage;
}

void
WDsrOptionRerrUnreachHeader::SetErrorSrc(Ipv4Address errorSrcAddress)
{
    m_errorSrcAddress = errorSrcAddress;
}

Ipv4Address
WDsrOptionRerrUnreachHeader::GetErrorSrc() const
{
    return m_errorSrcAddress;
}

void
WDsrOptionRerrUnreachHeader::SetErrorDst(Ipv4Address errorDstAddress)
{
    m_errorDstAddress = errorDstAddress;
}

Ipv4Address
WDsrOptionRerrUnreachHeader::GetErrorDst() const
{
    return m_errorDstAddress;
}

void
WDsrOptionRerrUnreachHeader::SetUnreachNode(Ipv4Address unreachNode)
{
    m_unreachNode = unreachNode;
}

Ipv4Address
WDsrOptionRerrUnreachHeader::GetUnreachNode() const
{
    return m_unreachNode;
}

void
WDsrOptionRerrUnreachHeader::SetOriginalDst(Ipv4Address originalDst)
{
    m_originalDst = originalDst;
}

Ipv4Address
WDsrOptionRerrUnreachHeader::GetOriginalDst() const
{
    return m_originalDst;
}

void
WDsrOptionRerrUnreachHeader::Print(std::ostream& os) const
{
    os << "( type = " << (uint32_t)GetType() << " length = " << (uint32_t)GetLength()
       << " errorType = " << (uint32_t)m_errorType << " salvage = " << (uint32_t)m_salvage
       << " error source = " << m_errorSrcAddress << " error dst = " << m_errorDstAddress
       << " unreach node = " << m_unreachNode << " )";
}

uint32_t
WDsrOptionRerrUnreachHeader::GetSerializedSize() const
{
    return 20;
}

void
WDsrOptionRerrUnreachHeader::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    i.WriteU8(GetType());
    i.WriteU8(GetLength());
    i.WriteU8(GetErrorType());
    i.WriteU8(m_salvage);
    WriteTo(i, m_errorSrcAddress);
    WriteTo(i, m_errorDstAddress);
    WriteTo(i, m_unreachNode);
    WriteTo(i, m_originalDst);
}

uint32_t
WDsrOptionRerrUnreachHeader::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;

    SetType(i.ReadU8());
    SetLength(i.ReadU8());
    SetErrorType(i.ReadU8());
    m_salvage = i.ReadU8();
    ReadFrom(i, m_errorSrcAddress);
    ReadFrom(i, m_errorDstAddress);
    ReadFrom(i, m_unreachNode);
    ReadFrom(i, m_originalDst);

    return GetSerializedSize();
}

WDsrOptionHeader::Alignment
WDsrOptionRerrUnreachHeader::GetAlignment() const
{
    Alignment retVal = {4, 0};
    return retVal;
}

NS_OBJECT_ENSURE_REGISTERED(WDsrOptionRerrUnsupportHeader);

TypeId
WDsrOptionRerrUnsupportHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::wdsr::WDsrOptionRerrUnsupportHeader")
                            .AddConstructor<WDsrOptionRerrUnsupportHeader>()
                            .SetParent<WDsrOptionRerrHeader>()
                            .SetGroupName("WDsr");
    return tid;
}

TypeId
WDsrOptionRerrUnsupportHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

WDsrOptionRerrUnsupportHeader::WDsrOptionRerrUnsupportHeader()
    : m_salvage(0)
{
    SetType(3);
    SetLength(14);
    SetErrorType(3);
}

WDsrOptionRerrUnsupportHeader::~WDsrOptionRerrUnsupportHeader()
{
}

void
WDsrOptionRerrUnsupportHeader::SetSalvage(uint8_t salvage)
{
    m_salvage = salvage;
}

uint8_t
WDsrOptionRerrUnsupportHeader::GetSalvage() const
{
    return m_salvage;
}

void
WDsrOptionRerrUnsupportHeader::SetErrorSrc(Ipv4Address errorSrcAddress)
{
    m_errorSrcAddress = errorSrcAddress;
}

Ipv4Address
WDsrOptionRerrUnsupportHeader::GetErrorSrc() const
{
    return m_errorSrcAddress;
}

void
WDsrOptionRerrUnsupportHeader::SetErrorDst(Ipv4Address errorDstAddress)
{
    m_errorDstAddress = errorDstAddress;
}

Ipv4Address
WDsrOptionRerrUnsupportHeader::GetErrorDst() const
{
    return m_errorDstAddress;
}

void
WDsrOptionRerrUnsupportHeader::SetUnsupported(uint16_t unsupport)
{
    m_unsupport = unsupport;
}

uint16_t
WDsrOptionRerrUnsupportHeader::GetUnsupported() const
{
    return m_unsupport;
}

void
WDsrOptionRerrUnsupportHeader::Print(std::ostream& os) const
{
    os << "( type = " << (uint32_t)GetType() << " length = " << (uint32_t)GetLength()
       << " errorType = " << (uint32_t)m_errorType << " salvage = " << (uint32_t)m_salvage
       << " error source = " << m_errorSrcAddress << " error dst = " << m_errorDstAddress
       << " unsupported option = " << m_unsupport << " )";
}

uint32_t
WDsrOptionRerrUnsupportHeader::GetSerializedSize() const
{
    return 16;
}

void
WDsrOptionRerrUnsupportHeader::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    i.WriteU8(GetType());
    i.WriteU8(GetLength());
    i.WriteU8(GetErrorType());
    i.WriteU8(m_salvage);
    WriteTo(i, m_errorSrcAddress);
    WriteTo(i, m_errorDstAddress);
    i.WriteU16(m_unsupport);
}

uint32_t
WDsrOptionRerrUnsupportHeader::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;

    SetType(i.ReadU8());
    SetLength(i.ReadU8());
    SetErrorType(i.ReadU8());
    m_salvage = i.ReadU8();
    ReadFrom(i, m_errorSrcAddress);
    ReadFrom(i, m_errorDstAddress);
    m_unsupport = i.ReadU16();

    return GetSerializedSize();
}

WDsrOptionHeader::Alignment
WDsrOptionRerrUnsupportHeader::GetAlignment() const
{
    Alignment retVal = {4, 0};
    return retVal;
}

NS_OBJECT_ENSURE_REGISTERED(WDsrOptionAckReqHeader);

TypeId
WDsrOptionAckReqHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::wdsr::WDsrOptionAckReqHeader")
                            .AddConstructor<WDsrOptionAckReqHeader>()
                            .SetParent<WDsrOptionHeader>()
                            .SetGroupName("WDsr");
    return tid;
}

TypeId
WDsrOptionAckReqHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

WDsrOptionAckReqHeader::WDsrOptionAckReqHeader()
    : m_identification(0)

{
    SetType(160);
    SetLength(2);
}

WDsrOptionAckReqHeader::~WDsrOptionAckReqHeader()
{
}

void
WDsrOptionAckReqHeader::SetAckId(uint16_t identification)
{
    m_identification = identification;
}

uint16_t
WDsrOptionAckReqHeader::GetAckId() const
{
    return m_identification;
}

void
WDsrOptionAckReqHeader::Print(std::ostream& os) const
{
    os << "( type = " << (uint32_t)GetType() << " length = " << (uint32_t)GetLength()
       << " id = " << m_identification << " )";
}

uint32_t
WDsrOptionAckReqHeader::GetSerializedSize() const
{
    return 4;
}

void
WDsrOptionAckReqHeader::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    i.WriteU8(GetType());
    i.WriteU8(GetLength());
    i.WriteU16(m_identification);
}

uint32_t
WDsrOptionAckReqHeader::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;

    SetType(i.ReadU8());
    SetLength(i.ReadU8());
    m_identification = i.ReadU16();

    return GetSerializedSize();
}

WDsrOptionHeader::Alignment
WDsrOptionAckReqHeader::GetAlignment() const
{
    Alignment retVal = {4, 0};
    return retVal;
}

NS_OBJECT_ENSURE_REGISTERED(WDsrOptionAckHeader);

TypeId
WDsrOptionAckHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::wdsr::WDsrOptionAckHeader")
                            .AddConstructor<WDsrOptionAckHeader>()
                            .SetParent<WDsrOptionHeader>()
                            .SetGroupName("WDsr");
    return tid;
}

TypeId
WDsrOptionAckHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

WDsrOptionAckHeader::WDsrOptionAckHeader()
    : m_identification(0)
{
    SetType(32);
    SetLength(10);
}

WDsrOptionAckHeader::~WDsrOptionAckHeader()
{
}

void
WDsrOptionAckHeader::SetAckId(uint16_t identification)
{
    m_identification = identification;
}

uint16_t
WDsrOptionAckHeader::GetAckId() const
{
    return m_identification;
}

void
WDsrOptionAckHeader::SetRealSrc(Ipv4Address realSrcAddress)
{
    m_realSrcAddress = realSrcAddress;
}

Ipv4Address
WDsrOptionAckHeader::GetRealSrc() const
{
    return m_realSrcAddress;
}

void
WDsrOptionAckHeader::SetRealDst(Ipv4Address realDstAddress)
{
    m_realDstAddress = realDstAddress;
}

Ipv4Address
WDsrOptionAckHeader::GetRealDst() const
{
    return m_realDstAddress;
}

void
WDsrOptionAckHeader::Print(std::ostream& os) const
{
    os << "( type = " << (uint32_t)GetType() << " length = " << (uint32_t)GetLength()
       << " id = " << m_identification << " real src = " << m_realSrcAddress
       << " real dst = " << m_realDstAddress << " )";
}

uint32_t
WDsrOptionAckHeader::GetSerializedSize() const
{
    return 12;
}

void
WDsrOptionAckHeader::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    i.WriteU8(GetType());
    i.WriteU8(GetLength());
    i.WriteU16(m_identification);
    WriteTo(i, m_realSrcAddress);
    WriteTo(i, m_realDstAddress);
}

uint32_t
WDsrOptionAckHeader::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;

    SetType(i.ReadU8());
    SetLength(i.ReadU8());
    m_identification = i.ReadU16();
    ReadFrom(i, m_realSrcAddress);
    ReadFrom(i, m_realDstAddress);

    return GetSerializedSize();
}

WDsrOptionHeader::Alignment
WDsrOptionAckHeader::GetAlignment() const
{
    Alignment retVal = {4, 0};
    return retVal;
}
} /* namespace wdsr */
} /* namespace ns3 */

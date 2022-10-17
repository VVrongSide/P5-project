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

#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/wdsr-fs-header.h"
#include "ns3/wdsr-helper.h"
#include "ns3/wdsr-main-helper.h"
#include "ns3/wdsr-option-header.h"
#include "ns3/wdsr-rcache.h"
#include "ns3/wdsr-rreq-table.h"
#include "ns3/wdsr-rsendbuff.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-route.h"
#include "ns3/mesh-helper.h"
#include "ns3/ptr.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"

#include <vector>

using namespace ns3;
using namespace wdsr;

// -----------------------------------------------------------------------------
/**
 * \ingroup wdsr
 * \defgroup wdsr-test WDSR routing module tests
 */

/**
 * \ingroup wdsr-test
 * \ingroup tests
 *
 * \class WDsrFsHeaderTest
 * \brief Unit test for WDSR Fixed Size Header
 */
class WDsrFsHeaderTest : public TestCase
{
  public:
    WDsrFsHeaderTest();
    ~WDsrFsHeaderTest() override;
    void DoRun() override;
};

WDsrFsHeaderTest::WDsrFsHeaderTest()
    : TestCase("WDSR Fixed size Header")
{
}

WDsrFsHeaderTest::~WDsrFsHeaderTest()
{
}

void
WDsrFsHeaderTest::DoRun()
{
    wdsr::WDsrRoutingHeader header;
    wdsr::WDsrOptionRreqHeader rreqHeader;
    header.AddWDsrOption(rreqHeader); // has an alignment of 4n+0

    NS_TEST_EXPECT_MSG_EQ(header.GetSerializedSize() % 2,
                          0,
                          "length of routing header is not a multiple of 4");
    Buffer buf;
    buf.AddAtStart(header.GetSerializedSize());
    header.Serialize(buf.Begin());

    const uint8_t* data = buf.PeekData();
    NS_TEST_EXPECT_MSG_EQ(*(data + 8),
                          rreqHeader.GetType(),
                          "expect the rreqHeader after fixed size header");
}

// -----------------------------------------------------------------------------
/**
 * \ingroup wdsr-test
 * \ingroup tests
 *
 * \class WDsrRreqHeaderTest
 * \brief Unit test for RREQ
 */
class WDsrRreqHeaderTest : public TestCase
{
  public:
    WDsrRreqHeaderTest();
    ~WDsrRreqHeaderTest() override;
    void DoRun() override;
};

WDsrRreqHeaderTest::WDsrRreqHeaderTest()
    : TestCase("WDSR RREQ")
{
}

WDsrRreqHeaderTest::~WDsrRreqHeaderTest()
{
}

void
WDsrRreqHeaderTest::DoRun()
{
    wdsr::WDsrOptionRreqHeader h;

    const std::vector<Ipv4Address> nodeList{
        Ipv4Address("1.1.1.0"),
        Ipv4Address("1.1.1.1"),
        Ipv4Address("1.1.1.2"),
    };

    h.SetTarget(Ipv4Address("1.1.1.3"));
    NS_TEST_EXPECT_MSG_EQ(h.GetTarget(), Ipv4Address("1.1.1.3"), "trivial");
    h.SetNodesAddress(nodeList);
    NS_TEST_EXPECT_MSG_EQ(h.GetNodeAddress(0), Ipv4Address("1.1.1.0"), "trivial");
    NS_TEST_EXPECT_MSG_EQ(h.GetNodeAddress(1), Ipv4Address("1.1.1.1"), "trivial");
    NS_TEST_EXPECT_MSG_EQ(h.GetNodeAddress(2), Ipv4Address("1.1.1.2"), "trivial");
    h.SetId(1);
    NS_TEST_EXPECT_MSG_EQ(h.GetId(), 1, "trivial");

    Ptr<Packet> p = Create<Packet>();
    wdsr::WDsrRoutingHeader header;
    header.AddWDsrOption(h);
    p->AddHeader(header);
    p->RemoveAtStart(8);
    wdsr::WDsrOptionRreqHeader h2;
    h2.SetNumberAddress(3);
    uint32_t bytes = p->RemoveHeader(h2);
    NS_TEST_EXPECT_MSG_EQ(bytes, 20, "Total RREP is 20 bytes long");
}

// -----------------------------------------------------------------------------
/**
 * \ingroup wdsr-test
 * \ingroup tests
 *
 * \class WDsrRrepHeaderTest
 * \brief Unit test for RREP
 */
class WDsrRrepHeaderTest : public TestCase
{
  public:
    WDsrRrepHeaderTest();
    ~WDsrRrepHeaderTest() override;
    void DoRun() override;
};

WDsrRrepHeaderTest::WDsrRrepHeaderTest()
    : TestCase("WDSR RREP")
{
}

WDsrRrepHeaderTest::~WDsrRrepHeaderTest()
{
}

void
WDsrRrepHeaderTest::DoRun()
{
    wdsr::WDsrOptionRrepHeader h;

    const std::vector<Ipv4Address> nodeList{
        Ipv4Address("1.1.1.0"),
        Ipv4Address("1.1.1.1"),
        Ipv4Address("1.1.1.2"),
    };

    h.SetNodesAddress(nodeList);
    NS_TEST_EXPECT_MSG_EQ(h.GetNodeAddress(0), Ipv4Address("1.1.1.0"), "trivial");
    NS_TEST_EXPECT_MSG_EQ(h.GetNodeAddress(1), Ipv4Address("1.1.1.1"), "trivial");
    NS_TEST_EXPECT_MSG_EQ(h.GetNodeAddress(2), Ipv4Address("1.1.1.2"), "trivial");

    Ptr<Packet> p = Create<Packet>();
    wdsr::WDsrRoutingHeader header;
    header.AddWDsrOption(h);
    p->AddHeader(header);
    p->RemoveAtStart(8);
    wdsr::WDsrOptionRrepHeader h2;
    h2.SetNumberAddress(3);
    uint32_t bytes = p->RemoveHeader(h2);
    NS_TEST_EXPECT_MSG_EQ(bytes, 16, "Total RREP is 16 bytes long");
}

// -----------------------------------------------------------------------------
/**
 * \ingroup wdsr-test
 * \ingroup tests
 *
 * \class WDsrSRHeaderTest
 * \brief Unit test for Source Route
 */
class WDsrSRHeaderTest : public TestCase
{
  public:
    WDsrSRHeaderTest();
    ~WDsrSRHeaderTest() override;
    void DoRun() override;
};

WDsrSRHeaderTest::WDsrSRHeaderTest()
    : TestCase("WDSR Source Route")
{
}

WDsrSRHeaderTest::~WDsrSRHeaderTest()
{
}

void
WDsrSRHeaderTest::DoRun()
{
    wdsr::WDsrOptionSRHeader h;

    const std::vector<Ipv4Address> nodeList{
        Ipv4Address("1.1.1.0"),
        Ipv4Address("1.1.1.1"),
        Ipv4Address("1.1.1.2"),
    };

    h.SetNodesAddress(nodeList);
    NS_TEST_EXPECT_MSG_EQ(h.GetNodeAddress(0), Ipv4Address("1.1.1.0"), "trivial");
    NS_TEST_EXPECT_MSG_EQ(h.GetNodeAddress(1), Ipv4Address("1.1.1.1"), "trivial");
    NS_TEST_EXPECT_MSG_EQ(h.GetNodeAddress(2), Ipv4Address("1.1.1.2"), "trivial");

    h.SetSalvage(1);
    NS_TEST_EXPECT_MSG_EQ(h.GetSalvage(), 1, "trivial");
    h.SetSegmentsLeft(2);
    NS_TEST_EXPECT_MSG_EQ(h.GetSegmentsLeft(), 2, "trivial");

    Ptr<Packet> p = Create<Packet>();
    wdsr::WDsrRoutingHeader header;
    header.AddWDsrOption(h);
    p->AddHeader(header);
    p->RemoveAtStart(8);
    wdsr::WDsrOptionSRHeader h2;
    h2.SetNumberAddress(3);
    uint32_t bytes = p->RemoveHeader(h2);
    NS_TEST_EXPECT_MSG_EQ(bytes, 16, "Total RREP is 16 bytes long");
}

// -----------------------------------------------------------------------------
/**
 * \ingroup wdsr-test
 * \ingroup tests
 *
 * \class WDsrRerrHeaderTest
 * \brief Unit test for RERR
 */
class WDsrRerrHeaderTest : public TestCase
{
  public:
    WDsrRerrHeaderTest();
    ~WDsrRerrHeaderTest() override;
    void DoRun() override;
};

WDsrRerrHeaderTest::WDsrRerrHeaderTest()
    : TestCase("WDSR RERR")
{
}

WDsrRerrHeaderTest::~WDsrRerrHeaderTest()
{
}

void
WDsrRerrHeaderTest::DoRun()
{
    wdsr::WDsrOptionRerrUnreachHeader h;
    h.SetErrorSrc(Ipv4Address("1.1.1.0"));
    NS_TEST_EXPECT_MSG_EQ(h.GetErrorSrc(), Ipv4Address("1.1.1.0"), "trivial");
    h.SetErrorDst(Ipv4Address("1.1.1.1"));
    NS_TEST_EXPECT_MSG_EQ(h.GetErrorDst(), Ipv4Address("1.1.1.1"), "trivial");
    h.SetSalvage(1);
    NS_TEST_EXPECT_MSG_EQ(h.GetSalvage(), 1, "trivial");
    h.SetUnreachNode(Ipv4Address("1.1.1.2"));
    NS_TEST_EXPECT_MSG_EQ(h.GetUnreachNode(), Ipv4Address("1.1.1.2"), "trivial");

    Ptr<Packet> p = Create<Packet>();
    wdsr::WDsrRoutingHeader header;
    header.AddWDsrOption(h);
    p->AddHeader(header);
    p->RemoveAtStart(8);
    wdsr::WDsrOptionRerrUnreachHeader h2;
    uint32_t bytes = p->RemoveHeader(h2);
    NS_TEST_EXPECT_MSG_EQ(bytes, 20, "Total RREP is 20 bytes long");
}

// -----------------------------------------------------------------------------
/**
 * \ingroup wdsr-test
 * \ingroup tests
 *
 * \class WDsrAckReqHeaderTest
 * \brief Unit test for ACK-REQ
 */
class WDsrAckReqHeaderTest : public TestCase
{
  public:
    WDsrAckReqHeaderTest();
    ~WDsrAckReqHeaderTest() override;
    void DoRun() override;
};

WDsrAckReqHeaderTest::WDsrAckReqHeaderTest()
    : TestCase("WDSR Ack Req")
{
}

WDsrAckReqHeaderTest::~WDsrAckReqHeaderTest()
{
}

void
WDsrAckReqHeaderTest::DoRun()
{
    wdsr::WDsrOptionAckReqHeader h;

    h.SetAckId(1);
    NS_TEST_EXPECT_MSG_EQ(h.GetAckId(), 1, "trivial");

    Ptr<Packet> p = Create<Packet>();
    wdsr::WDsrRoutingHeader header;
    header.AddWDsrOption(h);
    p->AddHeader(header);
    p->RemoveAtStart(8);
    p->AddHeader(header);
    wdsr::WDsrOptionAckReqHeader h2;
    p->RemoveAtStart(8);
    uint32_t bytes = p->RemoveHeader(h2);
    NS_TEST_EXPECT_MSG_EQ(bytes, 4, "Total RREP is 4 bytes long");
}

// -----------------------------------------------------------------------------
/**
 * \ingroup wdsr-test
 * \ingroup tests
 *
 * \class WDsrAckHeaderTest
 * \brief Unit test for ACK
 */
class WDsrAckHeaderTest : public TestCase
{
  public:
    WDsrAckHeaderTest();
    ~WDsrAckHeaderTest() override;
    void DoRun() override;
};

WDsrAckHeaderTest::WDsrAckHeaderTest()
    : TestCase("WDSR ACK")
{
}

WDsrAckHeaderTest::~WDsrAckHeaderTest()
{
}

void
WDsrAckHeaderTest::DoRun()
{
    wdsr::WDsrOptionAckHeader h;

    h.SetRealSrc(Ipv4Address("1.1.1.0"));
    NS_TEST_EXPECT_MSG_EQ(h.GetRealSrc(), Ipv4Address("1.1.1.0"), "trivial");
    h.SetRealDst(Ipv4Address("1.1.1.1"));
    NS_TEST_EXPECT_MSG_EQ(h.GetRealDst(), Ipv4Address("1.1.1.1"), "trivial");
    h.SetAckId(1);
    NS_TEST_EXPECT_MSG_EQ(h.GetAckId(), 1, "trivial");

    Ptr<Packet> p = Create<Packet>();
    wdsr::WDsrRoutingHeader header;
    header.AddWDsrOption(h);
    p->AddHeader(header);
    p->RemoveAtStart(8);
    p->AddHeader(header);
    wdsr::WDsrOptionAckHeader h2;
    p->RemoveAtStart(8);
    uint32_t bytes = p->RemoveHeader(h2);
    NS_TEST_EXPECT_MSG_EQ(bytes, 12, "Total RREP is 12 bytes long");
}

// -----------------------------------------------------------------------------
/**
 * \ingroup wdsr-test
 * \ingroup tests
 *
 * \class WDsrCacheEntryTest
 * \brief Unit test for WDSR route cache entry
 */
class WDsrCacheEntryTest : public TestCase
{
  public:
    WDsrCacheEntryTest();
    ~WDsrCacheEntryTest() override;
    void DoRun() override;
};

WDsrCacheEntryTest::WDsrCacheEntryTest()
    : TestCase("WDSR ACK")
{
}

WDsrCacheEntryTest::~WDsrCacheEntryTest()
{
}

void
WDsrCacheEntryTest::DoRun()
{
    Ptr<wdsr::WDsrRouteCache> rcache = CreateObject<wdsr::WDsrRouteCache>();

    std::vector<Ipv4Address> ip{
        Ipv4Address("0.0.0.0"),
        Ipv4Address("0.0.0.1"),
    };

    Ipv4Address dst = Ipv4Address("0.0.0.1");
    wdsr::WDsrRouteCacheEntry entry(ip, dst, Seconds(1));
    NS_TEST_EXPECT_MSG_EQ(entry.GetVector().size(), 2, "trivial");
    NS_TEST_EXPECT_MSG_EQ(entry.GetDestination(), Ipv4Address("0.0.0.1"), "trivial");
    NS_TEST_EXPECT_MSG_EQ(entry.GetExpireTime(), Seconds(1), "trivial");

    entry.SetExpireTime(Seconds(3));
    NS_TEST_EXPECT_MSG_EQ(entry.GetExpireTime(), Seconds(3), "trivial");
    entry.SetDestination(Ipv4Address("1.1.1.1"));
    NS_TEST_EXPECT_MSG_EQ(entry.GetDestination(), Ipv4Address("1.1.1.1"), "trivial");
    ip.emplace_back("0.0.0.2");
    entry.SetVector(ip);
    NS_TEST_EXPECT_MSG_EQ(entry.GetVector().size(), 3, "trivial");

    NS_TEST_EXPECT_MSG_EQ(rcache->AddRoute(entry), true, "trivial");

    std::vector<Ipv4Address> ip2{
        Ipv4Address("1.1.1.0"),
        Ipv4Address("1.1.1.1"),
    };

    Ipv4Address dst2 = Ipv4Address("1.1.1.1");
    wdsr::WDsrRouteCacheEntry entry2(ip2, dst2, Seconds(2));
    wdsr::WDsrRouteCacheEntry newEntry;
    NS_TEST_EXPECT_MSG_EQ(rcache->AddRoute(entry2), true, "trivial");
    NS_TEST_EXPECT_MSG_EQ(rcache->LookupRoute(dst2, newEntry), true, "trivial");
    NS_TEST_EXPECT_MSG_EQ(rcache->DeleteRoute(Ipv4Address("2.2.2.2")), false, "trivial");

    NS_TEST_EXPECT_MSG_EQ(rcache->DeleteRoute(Ipv4Address("1.1.1.1")), true, "trivial");
    NS_TEST_EXPECT_MSG_EQ(rcache->DeleteRoute(Ipv4Address("1.1.1.1")), false, "trivial");
}

// -----------------------------------------------------------------------------
/**
 * \ingroup wdsr-test
 * \ingroup tests
 *
 * \class WDsrSendBuffTest
 * \brief Unit test for Send Buffer
 */
class WDsrSendBuffTest : public TestCase
{
  public:
    WDsrSendBuffTest();
    ~WDsrSendBuffTest() override;
    void DoRun() override;
    /// Check size limit function
    void CheckSizeLimit();
    /// Check timeout function
    void CheckTimeout();

    wdsr::WDsrSendBuffer q; ///< send buffer
};

WDsrSendBuffTest::WDsrSendBuffTest()
    : TestCase("WDSR SendBuff"),
      q()
{
}

WDsrSendBuffTest::~WDsrSendBuffTest()
{
}

void
WDsrSendBuffTest::DoRun()
{
    q.SetMaxQueueLen(32);
    NS_TEST_EXPECT_MSG_EQ(q.GetMaxQueueLen(), 32, "trivial");
    q.SetSendBufferTimeout(Seconds(10));
    NS_TEST_EXPECT_MSG_EQ(q.GetSendBufferTimeout(), Seconds(10), "trivial");

    Ptr<const Packet> packet = Create<Packet>();
    Ipv4Address dst1 = Ipv4Address("0.0.0.1");
    wdsr::WDsrSendBuffEntry e1(packet, dst1, Seconds(1));
    q.Enqueue(e1);
    q.Enqueue(e1);
    q.Enqueue(e1);
    NS_TEST_EXPECT_MSG_EQ(q.Find(Ipv4Address("0.0.0.1")), true, "trivial");
    NS_TEST_EXPECT_MSG_EQ(q.Find(Ipv4Address("1.1.1.1")), false, "trivial");
    NS_TEST_EXPECT_MSG_EQ(q.GetSize(), 1, "trivial");
    q.DropPacketWithDst(Ipv4Address("0.0.0.1"));
    NS_TEST_EXPECT_MSG_EQ(q.Find(Ipv4Address("0.0.0.1")), false, "trivial");
    NS_TEST_EXPECT_MSG_EQ(q.GetSize(), 0, "trivial");

    Ipv4Address dst2 = Ipv4Address("0.0.0.2");
    wdsr::WDsrSendBuffEntry e2(packet, dst2, Seconds(1));
    q.Enqueue(e1);
    q.Enqueue(e2);
    Ptr<Packet> packet2 = Create<Packet>();
    wdsr::WDsrSendBuffEntry e3(packet2, dst2, Seconds(1));
    NS_TEST_EXPECT_MSG_EQ(q.Dequeue(Ipv4Address("0.0.0.3"), e3), false, "trivial");
    NS_TEST_EXPECT_MSG_EQ(q.Dequeue(Ipv4Address("0.0.0.2"), e3), true, "trivial");
    NS_TEST_EXPECT_MSG_EQ(q.Find(Ipv4Address("0.0.0.2")), false, "trivial");
    q.Enqueue(e2);
    q.Enqueue(e3);
    NS_TEST_EXPECT_MSG_EQ(q.GetSize(), 2, "trivial");
    Ptr<Packet> packet4 = Create<Packet>();
    Ipv4Address dst4 = Ipv4Address("0.0.0.4");
    wdsr::WDsrSendBuffEntry e4(packet4, dst4, Seconds(20));
    q.Enqueue(e4);
    NS_TEST_EXPECT_MSG_EQ(q.GetSize(), 3, "trivial");
    q.DropPacketWithDst(Ipv4Address("0.0.0.4"));
    NS_TEST_EXPECT_MSG_EQ(q.GetSize(), 2, "trivial");

    CheckSizeLimit();

    Simulator::Schedule(q.GetSendBufferTimeout() + Seconds(1),
                        &WDsrSendBuffTest::CheckTimeout,
                        this);

    Simulator::Run();
    Simulator::Destroy();
}

void
WDsrSendBuffTest::CheckSizeLimit()
{
    Ptr<Packet> packet = Create<Packet>();
    Ipv4Address dst;
    wdsr::WDsrSendBuffEntry e1(packet, dst, Seconds(1));

    for (uint32_t i = 0; i < q.GetMaxQueueLen(); ++i)
    {
        q.Enqueue(e1);
    }
    NS_TEST_EXPECT_MSG_EQ(q.GetSize(), 3, "trivial");

    for (uint32_t i = 0; i < q.GetMaxQueueLen(); ++i)
    {
        q.Enqueue(e1);
    }
    NS_TEST_EXPECT_MSG_EQ(q.GetSize(), 3, "trivial");
}

void
WDsrSendBuffTest::CheckTimeout()
{
    NS_TEST_EXPECT_MSG_EQ(q.GetSize(), 0, "Must be empty now");
}

// -----------------------------------------------------------------------------
/**
 * \ingroup wdsr-test
 * \ingroup tests
 *
 * \class WDsrRreqTableTest
 * \brief Unit test for WDSR routing table entry
 */
class WDsrRreqTableTest : public TestCase
{
  public:
    WDsrRreqTableTest();
    ~WDsrRreqTableTest() override;
    void DoRun() override;
};

WDsrRreqTableTest::WDsrRreqTableTest()
    : TestCase("WDSR RreqTable")
{
}

WDsrRreqTableTest::~WDsrRreqTableTest()
{
}

void
WDsrRreqTableTest::DoRun()
{
    wdsr::RreqTableEntry rt;

    rt.m_reqNo = 2;
    NS_TEST_EXPECT_MSG_EQ(rt.m_reqNo, 2, "trivial");
}

// -----------------------------------------------------------------------------
/**
 * \ingroup wdsr-test
 * \ingroup tests
 *
 * \class WDsrTestSuite
 * \brief WDSR test suite
 */
class WDsrTestSuite : public TestSuite
{
  public:
    WDsrTestSuite()
        : TestSuite("routing-wdsr", UNIT)
    {
        AddTestCase(new WDsrFsHeaderTest, TestCase::QUICK);
        AddTestCase(new WDsrRreqHeaderTest, TestCase::QUICK);
        AddTestCase(new WDsrRrepHeaderTest, TestCase::QUICK);
        AddTestCase(new WDsrSRHeaderTest, TestCase::QUICK);
        AddTestCase(new WDsrRerrHeaderTest, TestCase::QUICK);
        AddTestCase(new WDsrAckReqHeaderTest, TestCase::QUICK);
        AddTestCase(new WDsrAckHeaderTest, TestCase::QUICK);
        AddTestCase(new WDsrCacheEntryTest, TestCase::QUICK);
        AddTestCase(new WDsrSendBuffTest, TestCase::QUICK);
    }
} g_wdsrTestSuite;

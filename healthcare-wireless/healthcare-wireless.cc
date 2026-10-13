#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ipv4-flow-classifier.h"

#include <iostream>
#include <string>

using namespace ns3;

int main(int argc, char *argv[])
{
    std::string protocol = "udp";
    double backgroundMbps = 1.0;

    CommandLine cmd(__FILE__);
    cmd.AddValue("protocol", "Healthcare protocol: udp or tcp", protocol);
    cmd.AddValue("backgroundMbps", "Background traffic rate in Mbps", backgroundMbps);
    cmd.Parse(argc, argv);

    if ((protocol != "udp" && protocol != "tcp") || backgroundMbps <= 0)
    {
        std::cerr << "Use protocol=udp or tcp and a positive backgroundMbps value."
                  << std::endl;
        return 1;
    }

    NodeContainer nodes;
    nodes.Create(3);

    NodeContainer stations;
    stations.Add(nodes.Get(0));
    stations.Add(nodes.Get(2));

    NodeContainer accessPoint;
    accessPoint.Add(nodes.Get(1));

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211g);
    wifi.SetRemoteStationManager(
        "ns3::ConstantRateWifiManager",
        "DataMode", StringValue("ErpOfdmRate6Mbps"),
        "ControlMode", StringValue("ErpOfdmRate6Mbps"));

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiMacHelper mac;
    Ssid ssid = Ssid("HealthcareNetwork");

    mac.SetType("ns3::StaWifiMac",
                "Ssid", SsidValue(ssid),
                "ActiveProbing", BooleanValue(false));

    NetDeviceContainer stationDevices = wifi.Install(phy, mac, stations);

    mac.SetType("ns3::ApWifiMac",
                "Ssid", SsidValue(ssid));

    NetDeviceContainer apDevices = wifi.Install(phy, mac, accessPoint);

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    InternetStackHelper internet;
    internet.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    address.Assign(stationDevices);
    Ipv4InterfaceContainer apInterfaces = address.Assign(apDevices);

    uint16_t healthcarePort = 9000;
    uint16_t backgroundPort = 9002;

    std::string socketType =
        protocol == "tcp"
            ? "ns3::TcpSocketFactory"
            : "ns3::UdpSocketFactory";

    PacketSinkHelper healthcareSink(
        socketType,
        InetSocketAddress(Ipv4Address::GetAny(), healthcarePort));

    ApplicationContainer healthcareServer =
        healthcareSink.Install(nodes.Get(1));

    healthcareServer.Start(Seconds(0.5));
    healthcareServer.Stop(Seconds(10.0));

    OnOffHelper healthcareSource(
        socketType,
        InetSocketAddress(apInterfaces.GetAddress(0), healthcarePort));

    healthcareSource.SetAttribute("DataRate", DataRateValue(DataRate("80kbps")));
    healthcareSource.SetAttribute("PacketSize", UintegerValue(100));
    healthcareSource.SetAttribute(
        "OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    healthcareSource.SetAttribute(
        "OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer healthcareClient =
        healthcareSource.Install(nodes.Get(0));

    healthcareClient.Start(Seconds(2.0));
    healthcareClient.Stop(Seconds(9.0));

    PacketSinkHelper backgroundSink(
        "ns3::UdpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), backgroundPort));

    ApplicationContainer backgroundServer =
        backgroundSink.Install(nodes.Get(1));

    backgroundServer.Start(Seconds(0.5));
    backgroundServer.Stop(Seconds(10.0));

    OnOffHelper backgroundSource(
        "ns3::UdpSocketFactory",
        InetSocketAddress(apInterfaces.GetAddress(0), backgroundPort));

    uint64_t backgroundBps =
        static_cast<uint64_t>(backgroundMbps * 1000000.0);

    backgroundSource.SetAttribute(
        "DataRate", DataRateValue(DataRate(backgroundBps)));
    backgroundSource.SetAttribute("PacketSize", UintegerValue(1000));
    backgroundSource.SetAttribute(
        "OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    backgroundSource.SetAttribute(
        "OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer backgroundClient =
        backgroundSource.Install(nodes.Get(2));

    backgroundClient.Start(Seconds(2.0));
    backgroundClient.Stop(Seconds(9.0));

    FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

    Simulator::Stop(Seconds(15.0));
    Simulator::Run();

    monitor->CheckForLostPackets();

    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());

    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

    std::cout << "\nHealthcare wireless experiment\n";
    std::cout << "Protocol: " << protocol << "\n";
    std::cout << "Background offered rate: "
              << backgroundMbps << " Mbps\n\n";

    std::cout << "Flow\tTxPackets\tRxPackets\tLost\t"
              << "Throughput_kbps\tAvgDelay_ms\tJitter_ms\n";

    for (const auto &entry : stats)
    {
        Ipv4FlowClassifier::FiveTuple tuple =
            classifier->FindFlow(entry.first);

        if (tuple.destinationPort != healthcarePort &&
            tuple.destinationPort != backgroundPort)
        {
            continue;
        }

        const FlowMonitor::FlowStats &flow = entry.second;

        double duration =
            (flow.timeLastRxPacket - flow.timeFirstRxPacket).GetSeconds();

        double throughputKbps = 0.0;

        if (flow.rxPackets > 0 && duration > 0.0)
        {
            throughputKbps =
                flow.rxBytes * 8.0 / duration / 1000.0;
        }

        double avgDelayMs = 0.0;
        double avgJitterMs = 0.0;

        if (flow.rxPackets > 0)
        {
            avgDelayMs =
                flow.delaySum.GetSeconds() * 1000.0 / flow.rxPackets;
        }

        if (flow.rxPackets > 1)
        {
            avgJitterMs =
                flow.jitterSum.GetSeconds() * 1000.0 /
                (flow.rxPackets - 1);
        }

        std::string flowName =
            tuple.destinationPort == healthcarePort
                ? "Healthcare"
                : "Background";

        std::cout << flowName << "\t"
                  << flow.txPackets << "\t"
                  << flow.rxPackets << "\t"
                  << flow.lostPackets << "\t"
                  << throughputKbps << "\t"
                  << avgDelayMs << "\t"
                  << avgJitterMs << "\n";
    }

    std::string filename =
        "healthcare-wireless-" + protocol + "-" +
        std::to_string(static_cast<int>(backgroundMbps)) + "Mbps.xml";

    monitor->SerializeToXmlFile(filename, true, true);

    std::cout << "\nSaved FlowMonitor results to " << filename << "\n";

    Simulator::Destroy();

    return 0;
}

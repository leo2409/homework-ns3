#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
// MODULI PER CSMA
#include "ns3/csma-star-helper.h"
#include "ns3/csma-module.h"
// MODULI PER WIFI
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/ssid.h"
//#include "point-to-point-star.h"

using namespace ns3;
using namespace std;

const string matricola = "1933819";
NS_LOG_COMPONENT_DEFINE("Task_" + matricola);


int
main(int argc, char* argv[])
{
    //LogComponentEnable("Task_1933819", LOG_LEVEL_LOGIC);
    bool verbose = true;
    bool tracing = false;
    string studentId = "0";
    bool enableRtsCts = false;

    CommandLine cmd(__FILE__);
    //cin >> studentId;
    cmd.AddValue("studentId", "Stringa della Matricola Referente", studentId);
    cmd.AddValue("enableRtsCts", "Forza l'uso del meccanismo RtsCts", enableRtsCts);
    //variabile standard
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    cmd.AddValue("tracing", "Enable pcap tracing", tracing);

    cmd.Parse(argc, argv);
    
    cout << studentId << endl;

    // Passaggio matricola come parametro, se non è corretta il programma deve interrompersi
    if (studentId.compare(matricola)){
        NS_LOG_UNCOND("Matricola Errata");
        return 0;
    }

    // ========================= COSTRUZIONE TOPOLOGIA =========================
    NS_LOG_UNCOND("Creazione topologia di rete");

    // ------------------------- CSMA ROUTER 2 NODI 0-1 -------------------------
    // CSMA STELLA ROUTER 2 e server 0 e 1
    CsmaHelper csma_10_200;       
    csma_10_200.SetChannelAttribute("DataRate", StringValue("10Mbps"));
    csma_10_200.SetChannelAttribute("Delay", StringValue("200ms"));

    // creo un hub router 2 e 2 spoke nodi 0 e 1
    NodeContainer starNodes;
    starNodes.Create(3);
    // n2 -> 0
    // n1 -> 1
    // n0 -> 2
    Ptr<Node> n2 =  starNodes.Get(0);
    Ptr<Node> n1 =  starNodes.Get(1);
    Ptr<Node> n0 =  starNodes.Get(2);

    NetDeviceContainer starCsma;
    starCsma = csma_10_200.Install(starNodes);



    // ------------------------- WIFI ROUTER 10 NODI 11-19 -------------------------

    //  WIFI: ROUTER-WIFI 10 E NODI (LAPTOP) 11-19
    // access point wifi router nodo 10
    NodeContainer wifiApNode;
    wifiApNode.Create(1);
    Ptr<Node> n10 =  wifiApNode.Get(0);

    NodeContainer wifiStaNodes;
    wifiStaNodes.Create(9);
    Ptr<Node> n11 =  wifiStaNodes.Get(0);
    Ptr<Node> n12 =  wifiStaNodes.Get(1);
    Ptr<Node> n13 =  wifiStaNodes.Get(2);
    Ptr<Node> n14 =  wifiStaNodes.Get(3);
    Ptr<Node> n15 =  wifiStaNodes.Get(4);
    Ptr<Node> n16 =  wifiStaNodes.Get(5);
    Ptr<Node> n17 =  wifiStaNodes.Get(6);
    Ptr<Node> n18 =  wifiStaNodes.Get(7);
    Ptr<Node> n19 =  wifiStaNodes.Get(8);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    // Impostiamo i protocolli e la banda del WI-FI
    WifiHelper wifi;
    wifi.SetStandard (WIFI_STANDARD_80211g);
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");

    WifiMacHelper mac;
    Ssid ssid = Ssid(matricola);

    NetDeviceContainer staDevices;
    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
    staDevices = wifi.Install(phy, mac, wifiStaNodes);
    
    NetDeviceContainer apDevices;
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    apDevices = wifi.Install(phy, mac, wifiApNode);

    // mobilità dei client wifi
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(0.0),
                                  "MinY",
                                  DoubleValue(0.0),
                                  "DeltaX",
                                  DoubleValue(5.0),
                                  "DeltaY",
                                  DoubleValue(5.0),
                                  "GridWidth",
                                  UintegerValue(3),
                                  "LayoutType",
                                  StringValue("RowFirst"));
                                  
    /*  Qui definisco che ogni nodo si può muovere per un raggio quadrato con 30 m di lato */
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds",
                              RectangleValue(Rectangle(-15, 15, -15, 15)));
     // installo il modello di mobilità sui nodi
     mobility.Install(wifiStaNodes);

     // modello di mobilità per il router wifi stazionario
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNode);

    // ------------------------- ALBERO NODI 5-9 -------------------------

    PointToPointHelper ptp_5_20;
    ptp_5_20.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    ptp_5_20.SetChannelAttribute("Delay", StringValue("20ms"));

    NodeContainer tree;
    tree.Create(5);
   Ptr<Node> n5 =  tree.Get(0);
   Ptr<Node> n6 =  tree.Get(1);
   Ptr<Node> n7 =  tree.Get(2);
   Ptr<Node> n8 =  tree.Get(3);
   Ptr<Node> n9 =  tree.Get(4);
    
    NetDeviceContainer treePtp;
    treePtp.Add(ptp_5_20.Install(n5, n6)); // n5 --- n6
    treePtp.Add(ptp_5_20.Install(n5, n7)); // n5 --- n7
    treePtp.Add(ptp_5_20.Install(n6, n8)); // n6 --- n8
    treePtp.Add(ptp_5_20.Install(n6, n9)); // n6 --- n9

    // ------------------------- NODI CENTRALI -------------------------

    NodeContainer centralNodes;
    centralNodes.Create(2);
    Ptr<Node> n3 =  tree.Get(0);
    Ptr<Node> n4 =  tree.Get(1);

    PointToPointHelper ptp_100_20;
    ptp_100_20.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    ptp_100_20.SetChannelAttribute("Delay", StringValue("20ms"));

    PointToPointHelper ptp_10_200;
    ptp_10_200.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    ptp_10_200.SetChannelAttribute("Delay", StringValue("200ms"));

    NetDeviceContainer centralNodesPtp;
    centralNodesPtp.Add(ptp_10_200.Install(n3, n4));  // n3 --- n4

    centralNodesPtp.Add(ptp_100_20.Install(n3, n5));          // n3 --- n5
    centralNodesPtp.Add(ptp_100_20.Install(n3, n10));    // n3 --- n10

    centralNodesPtp.Add(ptp_100_20.Install(n4, n2));  // n4 --- n2
    centralNodesPtp.Add(ptp_100_20.Install(n4, n5));          // n4 --- n5
    centralNodesPtp.Add(ptp_100_20.Install(n4, n10));    // n4 --- n10

    // ========================= SERVIZI DI RETE =========================

    NS_LOG_UNCOND("Servizi di rete");

    InternetStackHelper stack;

    stack.Install(starNodes);
    stack.Install(tree);
    stack.Install(centralNodes);
    stack.Install(wifiApNode);
    stack.Install(wifiStaNodes);


    // ========================= ASSEGNAZIONE INDIRIZZI IP =========================

    NS_LOG_UNCOND("Indirizzi IP");

    Ipv4AddressHelper address;

    // per la rete centrale
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer centralInterface;
    centralInterface = address.Assign(centralNodesPtp);

    // per i nodi n0, n1 ed n2 usiamo una maschera con 8 slot
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaStarInterface;
    csmaStarInterface = address.Assign(starCsma);

    // per l'albero da n5 - n9
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer treeInterface;
    treeInterface = address.Assign(treePtp);

    //per la rete wi-fi n10 - n19
    address.SetBase("10.1.4.0", "255.255.255.0");
    address.Assign(staDevices);
    address.Assign(apDevices);


    //  PORTA

    //uint16_t port = 9;

    
    // ========================= LIVELLO APPLICATIVO CLIENT =========================
    /*
    // Source 0
    OnOffHelper source0("ns3::UdpSockerFactory", InetSocketAddress(csmaStarInterface.GetAddress(1), port));

    // Nodo 11
    uint16_t nBytes = 1341;
    source0.SetAttribute("PacketSize", UintegerValue(nBytes));
    ApplicationContainer s0App1 = source0.Install(wifiStaNodes.Get(0));
    s0App1.Start(Seconds(0.36));
    s0App1.Stop(Seconds(15.0));

    //Nodo 6
    nBytes = 1431;
    source0.SetAttribute("PacketSize", UintegerValue(nBytes));
    ApplicationContainer s0App2 = source0.Install(tree.Get(1));
    s0App2.Start(Seconds(3.2));
    s0App2.Stop(Seconds(15.0));
    // End Source 0

    // Source 1
    OnOffHelper source1("ns3::UdpSocketFactory", InetSocketAddress(csmaStarInterface.GetAddress(3), port));  // TODO rivedere questo 3?

    // Nodo 19
    nBytes = 1567;
    source1.SetAttribute("PacketSize", UintegerValue(nBytes));
    ApplicationContainer s1App1 = source1.Install(wifiStaNodes.Get(0));
    s1App1.Start(Seconds(3.55));
    s1App1.Stop(Seconds(15.0));
    // End Source 1
    */
    
    // ========================= LIVELLO APPLICATIVO SERVER =========================

    NS_LOG_UNCOND("Livello applicativo");

    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(n5);
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(treeInterface.GetAddress(0),9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(n9);
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));
    /*
    PacketSinkHelper sink("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));

    // Server 0
    ApplicationContainer server0 = sink.Install(cmsaStar_2Nodes.Get(0));
    server0.Start(Seconds(0.0));
    // Server 0

    // Server 1
    ApplicationContainer server1 = sink.Install(cmsaStar_2Nodes.Get(1));
    server1.Start(Seconds(0.0));
    // Server 1

    // Porta

    uint16_t porta = 9;

    // ========================= LIVELLO APPLICATIVO ECHO =========================
    
    UdpEchoServerHelper echoServer(porta);

    // Server 3
    ApplicationContainer echoApp = echoServer.Install(centralNodes.Get(0));
    echoApp.Start(Seconds(1.0));
    echoApp.Stop(Seconds(15.0));

    // Client 8
    UdpEchoClientHelper echoClient(treeContainer.GetAddress(3), porta);
    echoClient.SetAttribute("MaxPackets", UintegerValue(250));
    echoClient.SetAttribute("Interval", TimeValue(MicroSeconds(20.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1029));

    ApplicationContainer clientApps = echoClient.Install(tree.Get(3));
    
    echoClient.SetFill(clientApps.Get(0), "Edoardo, Toderi, 1933819, Andrea, Guida, 1948214, Leonardo, Brunetti, 1939837");

    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(15.0));
    */



    NS_LOG_UNCOND("Popolazione Tabelle Routing");

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    NS_LOG_UNCOND("Tracing");
    /*
    if(tracing){
        phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
        phy.EnablePcap("router-wifi-nodo-10", apDevices.Get(0), true);

        //csma_10_200.EnablePcap("switch-nodo-2", csmaStar_2.GetHubDevices(), true);

        //ptp_100_20.EnablePcap("switch-nodo-4", centralNodesPtp.Get(1), true, true);

        ptp_5_20.EnablePcap("switch-node-5", treePtp.Get(0), true, true);
    }
    */
    cout << "Inizio Simulazione" << std::endl; 
    Simulator::Run();
    Simulator::Stop(Seconds(15.0)); 
	cout << "Fine Simulazione" << std::endl;
    Simulator::Destroy();

    return 0;


}
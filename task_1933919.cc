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
#include "point-to-point-star.h"

using namespace ns3;
using namespace std;

const string matricola = "1933819";
NS_LOG_COMPONENT_DEFINE("Task_" + matricola);

int
main(int argc, char* argv[])
{
    bool verbose = true;
    bool tracing = false;
    string studentId = "0";
    bool enableRtsCts = false;

    CommandLine cmd(__FILE__);
    cmd.AddValue("studentId", "Stringa della Matricola Referente", studentId);
    cmd.AddValue("enableRtsCts", "Forza l'uso del meccanismo RtsCts", enableRtsCts);
    //variabile standard
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    cmd.AddValue("tracing", "Enable pcap tracing", tracing);

    // Passaggio matricola come parametro, se non è corretta il programma deve interrompersi
    if (studentId.compare(matricola)){
        //NS_LOG_UNCOND("Matricola Errata");
        return 0;
    }

    // ========================= COSTRUZIONE TOPOLOGIA =========================

    // ------------------------- CSMA ROUTER 2 NODE 0-1 -------------------------
    // CSMA STELLA ROUTER 2 e server 0 e 1
    CsmaHelper csma1;       
    csma1.SetChannelAttribute("DataRate", StringValue("10Mbps"));
    csma1.SetChannelAttribute("Delay", StringValue("200ms"));

    // creo un hub router 2 e 2 spoke nodi 0 e 1
    CsmaStarHelper csmaStar(2, csma1);
    
    NodeContainer csmaStarNodes;
    csmaStarNodes.Add(csmaStar.GetHub());           //nodo n2
    csmaStarNodes.Add(csmaStar.GetSpokeNode(0));    //nodo n0
    csmaStarNodes.Add(csmaStar.GetSpokeNode(1));    //nodo n1

    // installo le connessioni al livello di collegamento. Questo dispositivo deve essere usato per assegnare ip e stack di rete
    NetDeviceContainer csmaStella1 = csma1.Install(csmaStarNodes);

    // ------------------------- WIFI ROUTER 10 NODE 11-19 -------------------------

    //  WIFI: ROUTER-WIFI 10 E NODI (LAPTOP) 11-19
    // acces point wifi router nodo 10
    NodeContainer wifiApNode;
    wifiApNode.Create(1);

    NodeContainer wifiStaNodes;
    wifiStaNodes.Create(4);

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

    // ------------------------- ALBERO NODE 5-9 -------------------------

    PointToPointHelper ptp_5_20;
    ptp_5_20.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    ptp_5_20.SetChannelAttribute("Delay", StringValue("20ms"));

    // stella ptp 5,6,7
    PointToPointStarHelper ptpStar5(2, ptp_5_20);

    NodeContainer ptpStar5Nodes;
    ptpStar5Nodes.Add(ptpStar5.GetHub());           //nodo n5
    ptpStar5Nodes.Add(ptpStar5.GetSpokeNode(0));    //nodo n6
    ptpStar5Nodes.Add(ptpStar5.GetSpokeNode(1));    //nodo n7

    // installo le connessioni al livello di collegamento.
    NetDeviceContainer stellaPtp5; 
    stellaPtp5.Add(ptp1.Install(ptpStar5Nodes.Get(0), ptpStar5Nodes.Get(1)));
    stellaPtp5.Add(ptp1.Install(ptpStar5Nodes.Get(0), ptpStar5Nodes.Get(2)));

    Node n8,n9;
    
    /* NodeContainer n8, n9;
    n8.Create(1);
    n9.Create(1);
    NodeContainer n68, n69;
    n68 = NodeContainer(ptpStar1Nodes.Get(1),n8);
    n69 = NodeContainer(ptpStar1Nodes.Get(1),n9); */

    NetDeviceContainer stellaPtp6; 
    stellaPtp6.Add(ptp1.Install(ptpStar5Nodes.Get(0), &n8));
    stellaPtp6.Add(ptp1.Install(ptpStar5Nodes.Get(0), &n9));

    // ------------------------- PTP 3 E 4 -------------------------

    NodeContainer centralNodes;
    centralNodes.Create(2);

    PointToPointHelper ptp2;
    ptp2.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    ptp2.SetChannelAttribute("Delay", StringValue("200ms"));

    NetDeviceContainer ptp3_4;
    ptp3_4.Add(ptp2.Install(centralNodes.Get(0),centralNodes.Get(1)));

    // ------------------------- STELLA PTP CENTRO ROUTER 4 E SPOKES ROUTER 5,2,10 -------------------------
    NetDeviceContainer stellaPtp4;
    //stellaPtp4.Add(ptpcentralNodes.Get(0), wifiApNode.Get(0));



}
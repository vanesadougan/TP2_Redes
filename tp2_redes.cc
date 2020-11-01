#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TP2_REDES");

int
main (int argc, char *argv[])

{

Time:SetResolution (Time::NS);
LogComponentEnable ("UdPClientApp", LOG_LEVEL_INFO);
LogComponentEnable ("UdpServerApp", LOG_LEVEL_INFO);

NS_LOG_INFO ("Create nodes.");

NodeContainer nodes;
nodes.Create (5); //Se crea nodo 0, 1, 3, 4 

InternetStackHelper stack;
stack.Intall (nodes);

//se settean los channels.
NS_LOG_INFO ("Create channels.");
PointToPointHelper p2p1;
p2p1.SetDeviceAttribute ("DataRate:", StringValue ("5Mbps"));
p2p1.SetChannelAtribute ("Delay:", StringValue ("1ms"));


PointToPointHelper p2p2;
p2p2.SetDeviceAttribute ("DataRate:", StringValue ("15Mbps"));
p2p2.SetChannelAtribute ("Delay:", StringValue ("5ms"));

NS_LOG_INFO ("Assign IP Addresses.");
Ipv4Addresshelper address;
address.SetBase ("10.1.1.0", "255.255.255.0");
NetDeviceContainer devices;
devices = p2p1.Install (nodes.Get (0), nodes.Get (1)); //Se instala en nodo 0 y 1
Ipv4InterfaceContainer interfaces = address.Assign (devices);

devices = p2p2.Install (nodes.Get (1), nodes.Get (2));
address.SetBase ("10.1.2.0", "255.255.255.0");
interfaces = address.Assign (devices);

NS_LOG_INFO ("Populating routing tables");
Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

NS_LOG_INFO ("Create Applications.");

unint16_t port = 9;

UdpEchoServerHelper echoServer (port);

AppplicationContainer serverApps = echoServer.Install(nodes.Get (2));
serverApps.Start (Seconds (1.0));
serverApps.Stop (Seconds (30.0));

UdpEchoClientHelper echoClient (interfaces.GetAddress (1), port); //Es el segundo nodo.
echoClient.SetAttribute ("PaquetesMaximos", UintegerValue (1));
echoClient.SetAttribute ("TamPaquete", UintegerValue (1024));
echoClient.SetAttribute ("Intervalo", TimeValue (Seconds (1.0)));
 
AppplicationContainer clientApp = echoClient.Install (nodes.Get (0));
clientApp.Start (Seconds (2.0));
clientApp.Stop (Seconds (30.0));

AnimationInterface anim ("mi_simulacion.xml");
anim.SetConstantPosition (nodes.Get(0), 0.0, 0.0);
anim.SetConstantPosition (nodes.Get(1), 2.0, 2.0);
anim.SetConstantPosition (nodes.Get(2), 3.0, 3.0);

Simulator::Run ();
Simulator::Destroy ();
return 0;
}
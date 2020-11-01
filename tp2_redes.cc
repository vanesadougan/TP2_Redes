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
nodes.Create (5); //Se crea nodo 0, 1, 2, 3, 4 

InternetStackHelper stack;
stack.Intall (nodes);

//se settean los channels.
NS_LOG_INFO ("Create channels.");
PointToPointHelper p2Central;
p2Central.SetDeviceAttribute ("DataRate:", StringValue ("5Mbps"));
p2Central.SetChannelAtribute ("Delay:", StringValue ("1ms"));


PointToPointHelper p2Izq;
p2Izq.SetDeviceAttribute ("DataRate:", StringValue ("15Mbps"));
p2Izq.SetChannelAtribute ("Delay:", StringValue ("5ms"));


PointToPointHelper p2Der;
p2Der.SetDeviceAttribute ("DataRate:", StringValue ("15Mbps"));
p2Der.SetChannelAtribute ("Delay:", StringValue ("5ms"));

NS_LOG_INFO ("Create Dumbbell");
PointToPointDumbbellHelper topologia (3, p2Izq, 3, p2Der, p2Central);

NS_LOG_INFO ("Assign IP Addresses.");
topologia.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"),
                                Ipv4AddressHelper ("10.1.2.0", "255.255.255.0"),
                                Ipv4AddressHelper ("10.1.3.0", "255.255.255.0"));

NS_LOG_INFO ("Populating routing tables");
Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

NS_LOG_INFO ("Create Applications.");

unint16_t port = 9;

UdpEchoServerHelper echoServer (port);

AppplicationContainer serverApps = echoServer.Install(nodes.Get (2));
serverApps.Start (Seconds (1.0));
serverApps.Stop (Seconds (30.0));

/*UdpEchoClientHelper echoClient (interfaces.GetAddress (1), port); //Es el segundo nodo.
echoClient.SetAttribute ("PaquetesMaximos", UintegerValue (1));
echoClient.SetAttribute ("TamPaquete", UintegerValue (1024));
echoClient.SetAttribute ("Intervalo", TimeValue (Seconds (1.0)));
 
AppplicationContainer clientApp = echoClient.Install (nodes.Get (0));
clientApp.Start (Seconds (2.0));
clientApp.Stop (Seconds (30.0));
*/
AnimationInterface anim ("mi_simulacion.xml");
anim.SetConstantPosition (topologia.GetLeft(0), 0.0, 0.0);
Names::Add ("Sender TCP1", topologia.GetLeft(0));

anim.SetConstantPosition (topologia.GetLeft(1), 0.0, 2.0);
Names::Add ("Sender UDP1", topologia.GetLeft(1));

anim.SetConstantPosition (topologia.GetLeft(2), 0.0, 4.0);
Names::Add ("Sender TCP2", topologia.GetLeft(2));

anim.SetConstantPosition (topologia.GetLeft(), 2.0, 2.0);
Names::Add ("Router1", topologia.GetLeft());

anim.SetConstantPosition (topologia.GetRight(), 4.0, 2.0);
Names::Add ("Router2", topologia.GetRight());

anim.SetConstantPosition (topologia.GetRight(0), 6.0, 0.0);
Names::Add ("Receiver TCP1", topologia.GetRight(0));

anim.SetConstantPosition (topologia.GetRight(1), 6.0, 2.0);
Names::Add ("Receiver UDP1", topologia.GetRight(1));

anim.SetConstantPosition (topologia.GetRight(2), 6.0, 4.0);
Names::Add ("Receiver TCP2", topologia.GetRight(2));



Simulator::Run ();
Simulator::Destroy ();
return 0;
}
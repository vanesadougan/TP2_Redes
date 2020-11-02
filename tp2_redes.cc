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

bool habilitarUDP=true;
bool habilitarTCP=false;
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


NS_LOG_INFO ("Configurado Stack");

  InternetStackHelper stack;
  NodeContainer senders;
  NodeContainer receivers;
  for (uint32_t i = 0; i < topologia.LeftCount (); ++i)
    {
      senders.Add (topologia.GetLeft (i));
      stack.Install (topologia.GetLeft (i));
    }
  for (uint32_t i = 0; i < topologia.RightCount (); ++i)
    {
      receivers.Add (topologia.GetRight (i));
      stack.Install (topologia.GetRight (i));
    }

  stack.Install (dumbbell.GetLeft ());
  stack.Install (dumbbell.GetRight ());

NS_LOG_INFO ("Configurado Application layer");

configApplicationLayer (senders, topologia.GetRightIpv4Address (0), topologia.GetRightIpv4Address (1), topologia.GetRightIpv4Address (2), receivers);



NodeContainer container;
  container.Add (senders);
  container.Add (dumbbell.GetLeft ());
  container.Add (dumbbell.GetRight ());
  container.Add (receivers);


Simulator::Run ();
Simulator::Destroy ();
return 0;

void configApplicationLayer (NodeContainer senders, Ipv4Address receiverTCP1 , Ipv4Address receiverUDP1, Ipv4Address receiverTCP2, receivers);
{

  // Create the OnOff applications to send TCP to the server
  OnOffHelper udpOnOffApplication = createOnOffApplication ("ns3::UdpSocketFactory");
    uint16_t udpPort = 80005;
  OnOffHelper tcpOnOffApplication = createOnOffApplication ("ns3::TcpSocketFactory");
    uint16_t tcpPort = 80006;
  
  ApplicationContainer appContainerSenders;

  PacketSinkHelper udp_sink ("ns3::UdpSocketFactory",
                            InetSocketAddress (Ipv4Address::GetAny (), udpPort));

   PacketSinkHelper tcp_sink ("ns3::TcpSocketFactory",
                            InetSocketAddress (Ipv4Address::GetAny (), tcpPort));

   ApplicationContainer appContainerReceivers;



 // set up SenderTCP1 with onOff over TCP to send to receiverTCP1
  appContainerSenders.Add (setUpApplication (tcpOnOffApplication, senders.Get (0), receiverTCP1, tcpPort));
  appContainerReceivers.Add (tcp_sink.Install (receivers.Get (0)));
  // set up SenderTCP2 with onOff over TCP to send to receiverTCP2
 appContainerSenders.Add (setUpApplication (tcpOnOffApplication, senders.Get (2), receiverTCP2, tcpPort));
  appContainerReceivers.Add (tcp_sink.Install (receivers.Get (2)));


  if (habilitarTCP)
    {
      // set up senderUDP1 with onOff over TCP to send to receiverUDP1
      appContainerSenders.Add (setUpApplication (tcpOnOffApplication, senders.Get (1), receiverUPD1, tcpPort));
      appContainerReceivers.Add (tcp_sink.Install (receivers.Get (1)));
    }

  if (habilitarUDP && !habilitarTCP)
    {
      // set up senderUDP1 with onOff over UDP to send to receiverUDP1
      appContainerSenders.Add (setUpApplication (udpOnOffApplication, senders.Get (1), receiverUDP1, udpPort));
      appContainerReceivers.Add (udp_sink.Install (receivers.Get (1)));
    }

  

  appContainerSenders.Start (Seconds (1.0));
  appContainerSenders.Stop (Seconds (30.0);
  appContainerReceivers.Start (Seconds (0.0));
  appContainerReceivers.Stop (Seconds (30.0));

}

}
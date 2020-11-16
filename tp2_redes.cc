#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/point-to-point-layout-module.h" 
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include <iomanip>

using namespace ns3;
//Para intecalar entre el ejercicio 2,3 parte 1, o la prueba de Hybla en la parte 3 hay que setear estos flags: 
bool habilitarUDP=false;
bool todosTCP=false;

NS_LOG_COMPONENT_DEFINE ("TP2_REDES");

uint32_t megabytesDataRate = 150;

OnOffHelper createOnOffApplication (std::string socketFactory){
  OnOffHelper application (socketFactory, Address ());
  application.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  application.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  application.SetAttribute ("DataRate", DataRateValue (DataRate (megabytesDataRate * 8 * 1024 * 1024)));
  return application;
}

ApplicationContainer
setUpApplication (OnOffHelper application, Ptr<Node> source, Ipv4Address destination, uint16_t port){
  AddressValue address (InetSocketAddress (destination, port));
  application.SetAttribute ("Remote", address);
  return application.Install (source);
}

void configApplications (NodeContainer senders, Ipv4Address receiverTCP1 , Ipv4Address receiverUDP1, Ipv4Address receiverTCP2, NodeContainer receivers){
  OnOffHelper udpOnOffApplication = createOnOffApplication ("ns3::UdpSocketFactory");
  uint16_t udpPort = 8060;
  OnOffHelper tcpOnOffApplication = createOnOffApplication ("ns3::TcpSocketFactory");
  uint16_t tcpPort = 8020;
  
  ApplicationContainer appContainerSenders;

  PacketSinkHelper udp_sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), udpPort));

  PacketSinkHelper tcp_sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), tcpPort));

  ApplicationContainer appContainerReceivers;


  appContainerSenders.Add (setUpApplication (tcpOnOffApplication, senders.Get (0), receiverTCP1, tcpPort));
  appContainerReceivers.Add (tcp_sink.Install (receivers.Get (0)));
  
  appContainerSenders.Add (setUpApplication (tcpOnOffApplication, senders.Get (2), receiverTCP2, tcpPort));
  appContainerReceivers.Add (tcp_sink.Install (receivers.Get (2)));

  
  if (todosTCP){
    appContainerSenders.Add (setUpApplication (tcpOnOffApplication, senders.Get (1), receiverUDP1, tcpPort));
    appContainerReceivers.Add (tcp_sink.Install (receivers.Get (1)));
  }

  if (habilitarUDP && !todosTCP){
    appContainerSenders.Add (setUpApplication (udpOnOffApplication, senders.Get (1), receiverUDP1, udpPort));
    appContainerReceivers.Add (udp_sink.Install (receivers.Get (1)));
  }
  
  appContainerSenders.Start (Seconds (1.0));
  appContainerSenders.Stop (Seconds (30.0));
  appContainerReceivers.Start (Seconds (0.0));
  appContainerReceivers.Stop (Seconds (30.0));
}

int main (int argc, char *argv[]){
  //Descomentar para habilitar la implementacion de TCP Hybla
  //Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpHybla"));
  NS_LOG_INFO ("Create nodes.");

  //Partes de la topologia
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2Central;
  p2Central.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  p2Central.SetChannelAttribute ("Delay", StringValue ("50ms"));

  PointToPointHelper p2Izq;
  p2Izq.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2Izq.SetChannelAttribute ("Delay", StringValue ("1ms"));

  PointToPointHelper p2Der;
  p2Der.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2Der.SetChannelAttribute ("Delay", StringValue ("1ms"));

  NS_LOG_INFO ("Create Dumbbell");
  PointToPointDumbbellHelper topologia (3, p2Izq, 3, p2Der, p2Central);

  NS_LOG_INFO ("Create Animation.");
  //Creo la animacion para visualizar con NetAnim
  AnimationInterface::SetConstantPosition (topologia.GetLeft(0), 0.0, 0.0);
  Names::Add ("Sender TCP1", topologia.GetLeft(0));

  AnimationInterface::SetConstantPosition (topologia.GetLeft(1), 0.0, 2.0);
  Names::Add ("Sender UDP1", topologia.GetLeft(1));


  AnimationInterface::SetConstantPosition (topologia.GetLeft(2), 0.0, 4.0);
  Names::Add ("Sender TCP2", topologia.GetLeft(2));

  AnimationInterface::SetConstantPosition (topologia.GetLeft(), 2.0, 2.0);
  Names::Add ("Router1", topologia.GetLeft());

  AnimationInterface::SetConstantPosition (topologia.GetRight(), 4.0, 2.0);
  Names::Add ("Router2", topologia.GetRight());

  AnimationInterface::SetConstantPosition (topologia.GetRight(0), 6.0, 0.0);
  Names::Add ("Receiver TCP1", topologia.GetRight(0));

  AnimationInterface::SetConstantPosition (topologia.GetRight(1), 6.0, 2.0);
  Names::Add ("Receiver UDP1", topologia.GetRight(1));

  AnimationInterface::SetConstantPosition (topologia.GetRight(2), 6.0, 4.0);
  Names::Add ("Receiver TCP2", topologia.GetRight(2));
  AnimationInterface anim ("mi_simulacion.xml");
  anim.EnablePacketMetadata(true);

  NS_LOG_INFO ("Configurado Stack");

  InternetStackHelper stack;
  NodeContainer senders;
  NodeContainer receivers;
  for (uint32_t i = 0; i < topologia.LeftCount (); ++i){
      senders.Add (topologia.GetLeft (i));
      stack.Install (topologia.GetLeft (i));
  }

  for (uint32_t i = 0; i < topologia.RightCount (); ++i){
      receivers.Add (topologia.GetRight (i));
      stack.Install (topologia.GetRight (i));
  }

  stack.Install (topologia.GetLeft ());
  stack.Install (topologia.GetRight ());

  //Seteo las direcciones IP
  NS_LOG_INFO ("Assign IP Addresses.");
  topologia.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"),
                                  Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"),
                                  Ipv4AddressHelper ("10.3.1.0", "255.255.255.0"));

  NS_LOG_INFO ("Populating routing tables");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  NS_LOG_INFO ("Configurado Application layer");

  configApplications (senders, topologia.GetRightIpv4Address (0), topologia.GetRightIpv4Address (1), topologia.GetRightIpv4Address (2), receivers);

  NodeContainer container;
  container.Add (senders);
  container.Add (topologia.GetLeft ());
  container.Add (topologia.GetRight ());
  container.Add (receivers);

  AsciiTraceHelper asciiTrace;
  p2Izq.EnableAsciiAll (asciiTrace.CreateFileStream ("izquierda.tr"));
  p2Izq.EnablePcapAll ("izquierda");

  p2Der.EnableAsciiAll (asciiTrace.CreateFileStream ("derecha.tr"));

  p2Central.EnableAsciiAll (asciiTrace.CreateFileStream ("central.tr"));

  Simulator::Stop (Seconds (10.0)); 

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> flmon = flowmon.Install(container);

  Simulator::Run (); 

  flmon->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = flmon->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i) {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      std::cout << "Flujo " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
      std::cout << "  Paquetes totales transmitidos: " << i->second.txPackets << "\n";
      std::cout << "  Bytes totales transmitidos:   " << i->second.txBytes << "\n";
      std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / 9.0 / 1000 / 1000 << " Mbps\n";
      std::cout << "  Paquetes totales recibidos: " << i->second.rxPackets << "\n";
      std::cout << "  Bytes totales recibidos:   " << i->second.rxBytes << "\n";
      std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / 9.0 / 1000 / 1000 << " Mbps\n";
      std::cout << "  Paquetes Perdidos: " << i->second.lostPackets << "\n";
  }

  flmon->SerializeToXmlFile("estadisticas.xml", true, true);

  Simulator::Destroy ();
  return 0;
}

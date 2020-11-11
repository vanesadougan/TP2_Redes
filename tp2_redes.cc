#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/point-to-point-layout-module.h" //Dumbbel helper
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;
bool habilitarUDP=false;
bool habilitarTCP=true;

NS_LOG_COMPONENT_DEFINE ("TP2_REDES");

// Refactor ya!
uint32_t megabytesDataRate = 1;
OnOffHelper createOnOffApplication (std::string socketFactory){
  OnOffHelper application (socketFactory, Address ());
  application.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  application.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  application.SetAttribute ("DataRate",
                            DataRateValue (DataRate (megabytesDataRate * 8 * 1024 * 1024)));
  return application;
}

ApplicationContainer
setUpApplication (OnOffHelper application, Ptr<Node> source, Ipv4Address destination, uint16_t port)
{
  AddressValue address (InetSocketAddress (destination, port));
  application.SetAttribute ("Remote", address);
  return application.Install (source);
}

void configApplicationLayer (NodeContainer senders, Ipv4Address receiverTCP1 , Ipv4Address receiverUDP1, Ipv4Address receiverTCP2, NodeContainer receivers){
  // Create the OnOff applications to send TCP to the server
  OnOffHelper udpOnOffApplication = createOnOffApplication ("ns3::UdpSocketFactory");
    uint16_t udpPort = 8060;
  OnOffHelper tcpOnOffApplication = createOnOffApplication ("ns3::TcpSocketFactory");
    uint16_t tcpPort = 8020;
  
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
      appContainerSenders.Add (setUpApplication (tcpOnOffApplication, senders.Get (1), receiverUDP1, tcpPort));
      appContainerReceivers.Add (tcp_sink.Install (receivers.Get (1)));
    }

  if (habilitarUDP && !habilitarTCP)
    {
      // set up senderUDP1 with onOff over UDP to send to receiverUDP1
      appContainerSenders.Add (setUpApplication (udpOnOffApplication, senders.Get (1), receiverUDP1, udpPort));
      appContainerReceivers.Add (udp_sink.Install (receivers.Get (1)));
    }
  
  appContainerSenders.Start (Seconds (1.0));
  appContainerSenders.Stop (Seconds (90.0));
  appContainerReceivers.Start (Seconds (0.0));
  appContainerReceivers.Stop (Seconds (90.0));

}



int main (int argc, char *argv[]){


NS_LOG_INFO ("Create nodes.");

//Partes de la topologia
NS_LOG_INFO ("Create channels.");
PointToPointHelper p2Central;
p2Central.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
p2Central.SetChannelAttribute ("Delay", StringValue ("1ms"));

PointToPointHelper p2Izq;
p2Izq.SetDeviceAttribute ("DataRate", StringValue ("150Mbps"));
p2Izq.SetChannelAttribute ("Delay", StringValue ("1ms"));

PointToPointHelper p2Der;
p2Der.SetDeviceAttribute ("DataRate", StringValue ("150Mbps"));
p2Der.SetChannelAttribute ("Delay", StringValue ("1ms"));

NS_LOG_INFO ("Create Dumbbell");
PointToPointDumbbellHelper topologia (3, p2Izq, 3, p2Der, p2Central);

NS_LOG_INFO ("Create Applications.");


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


NS_LOG_INFO ("Assign IP Addresses.");
topologia.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"),
                                Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"),
                                Ipv4AddressHelper ("10.3.1.0", "255.255.255.0"));

NS_LOG_INFO ("Populating routing tables");
Ipv4GlobalRoutingHelper::PopulateRoutingTables ();




NS_LOG_INFO ("Configurado Application layer");

configApplicationLayer (senders, topologia.GetRightIpv4Address (0), topologia.GetRightIpv4Address (1), topologia.GetRightIpv4Address (2), receivers);



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


 
 //define stop time of simulator
  Simulator::Stop (Seconds (10.0)); //define stop time of simulator
 // Ptr<FlowMonitor> flowmon;
  //FlowMonitorHelper flowmonHelper;
  //flowmon = flowmonHelper.InstallAll();

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> flmon = flowmon.Install(container);

  Simulator::Run (); //run the simulation and destroy it once done

flmon->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = flmon->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin ();
       i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      std::cout << "Flujo " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress
                << ")\n";
      std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
      std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
      std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / 9.0 / 1000 / 1000 << " Mbps\n";
      std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
      std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / 9.0 / 1000 / 1000 << " Mbps\n";
      std::cout << "  Lost Packets: " << i->second.lostPackets << "\n";
    }



  flmon->SerializeToXmlFile("estadisticas.xml", true, true);

Simulator::Destroy ();
return 0;


}

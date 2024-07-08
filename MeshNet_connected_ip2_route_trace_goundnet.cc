#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"  
#include "ns3/ipv4.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"
#include <string>
#include <cstdlib>
#include <time.h>
#include <typeinfo>
#include "ns3/command-line.h"

#include "ns3/point-to-point-net-device.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("MeshNet_connected_ip");



class MyEventHandler : public ns3::Object
{
    public:
        //Cut Links
        void CutP2PLink_GS_Sat(NetDeviceContainer container, int i);
        void CutP2PLink_between_orbit(NetDeviceContainer container, int i);
        //Rebuild Links
        void RebuildP2PLink_GS_Sat(NetDeviceContainer container, int i);
};

void MyEventHandler::CutP2PLink_GS_Sat(NetDeviceContainer container, int i)
{
    //Get from GS_Node
    Ptr<PointToPointNetDevice> p2pNetDevice = DynamicCast<PointToPointNetDevice>(container.Get(i));
    p2pNetDevice->DownTheLink();
    std::cout << "CutP2PLink From GS:" << i << " at time: "<< Simulator::Now().GetSeconds() << "s" << std::endl;
}
void MyEventHandler::CutP2PLink_between_orbit(NetDeviceContainer container, int i)
{
    //Get from Sat_Node
    Ptr<PointToPointNetDevice> p2pNetDevice = DynamicCast<PointToPointNetDevice>(container.Get(i));
    p2pNetDevice->DownTheLink();
    std::cout << "CutP2PLink From Sat_netdevice:" << i << " at time: "<< Simulator::Now().GetSeconds() << "s" << std::endl;
}
void MyEventHandler::RebuildP2PLink_GS_Sat(NetDeviceContainer container, int i)
{
    Ptr<PointToPointNetDevice> p2pNetDevice = DynamicCast<PointToPointNetDevice>(container.Get(i));
    p2pNetDevice->UpTheLink();
    std::cout << "RebuildP2PLink From GS:" << i << " at time: "<< Simulator::Now().GetSeconds() << "s" << std::endl;
}


Ipv4Address 
GetIPAddress(Ptr<ns3::Node> node)
{
  Ptr<Ipv4> ippp = node-> GetObject<Ipv4> ();
  //获得第1个接口的第0个地址--多数为第一个接口，第0个接口是本机127.0.0.1
  Ipv4Address ipaddress = ippp->GetAddress(1,0).GetLocal();
  return ipaddress;
}


int main(int argc, char* argv[])
{
    Time::SetResolution(Time::NS);
    //LogComponentEnable("UdpClient", LOG_LEVEL_INFO);
    //LogComponentEnable("UdpServer", LOG_LEVEL_INFO);
    /*
        需要进行"外部传参"的参数有：
        (1) 单次仿真时长(即链路切断前后，拓扑处于稳定状态的持续时长)
        (2) GS与Sat的连接编号banngo1、banngo2
    */
    CommandLine cmd;
    double simulation_total_time = 20.0;
    int Connected_Sat_banngo1 = 2;
    int Connected_Sat_banngo2 = 22;

    cmd.AddValue("time","Total time of simulation this time",simulation_total_time);
    cmd.AddValue("sat1","number of GS-connected satellite(the first one)",Connected_Sat_banngo1);
    cmd.AddValue("sat2","number of GS-connected satellite(the second one)",Connected_Sat_banngo2);
    cmd.Parse (argc, argv);
    std::cout<< ">>>>>>Set Parameters: "<< simulation_total_time << "," << Connected_Sat_banngo1 << "," << Connected_Sat_banngo2 <<std::endl;
    //DEFAULT: ./ns3 run scratch/MeshNet_connected_ip2_route_trace_goundnet.cc -- --time=20.0 --sat1=2 --sat2=22

    //############################################################################################################################################################################################
    //Node Creation and Management
    NodeContainer Sat_Node;
    NodeContainer GS_Node;
    NodeContainer GSnet_Node;
    int Number_of_Sat_per_orbit = 8;
    int Number_of_orbit = 5;
    int Number_of_GS = 2;
    int Number_of_GSnet_per_row = 10;
    int Number_of_GSnet_row = 10;
    Sat_Node.Create(Number_of_Sat_per_orbit * Number_of_orbit);
    GS_Node.Create(Number_of_GS);
    GSnet_Node.Create(Number_of_GSnet_per_row * Number_of_GSnet_row);
    //NodeContainer All_Nodes = NodeContainer(Sat_Node,GS_Node);

    //############################################################################################################################################################################################
    //P2P NetDevices
    PointToPointHelper ptp_same_orbit;    
    ptp_same_orbit.SetDeviceAttribute("DataRate",StringValue("100Mbps"));
    ptp_same_orbit.SetDeviceAttribute("ReceiveErrorModel",StringValue("ns3::RateErrorModel"));
    ptp_same_orbit.SetChannelAttribute("Delay",StringValue("20ms"));
    PointToPointHelper ptp_between_orbit;    
    ptp_between_orbit.SetDeviceAttribute("DataRate",StringValue("100Mbps"));
    ptp_between_orbit.SetDeviceAttribute("ReceiveErrorModel",StringValue("ns3::RateErrorModel"));
    ptp_between_orbit.SetChannelAttribute("Delay",StringValue("50ms"));

    int i = 0;
    int j = 0;
    int ch1 = 1;
    int ch2 = 1;
    NetDeviceContainer same_orbit_NDcontainer;
    Ipv4InterfaceContainer Sat_ip_same_orbit;
    //同轨道内，相邻卫星之间的链路建立
    for (i=0; i<=Number_of_orbit-1; i++)    //i:choose orbit
    {
        ch2 = 1;
        std::cout << "--------------Orbit:" << i << "--------------" << std::endl;
        for (j=0; j<=Number_of_Sat_per_orbit-1; j++)  //j:choose one sat in certain orbit
        {
            NodeContainer temp_container;
            if(j!=Number_of_Sat_per_orbit-1)
                temp_container = NodeContainer(Sat_Node.Get( i * (Number_of_Sat_per_orbit) + j),
                                               Sat_Node.Get( i * (Number_of_Sat_per_orbit) + j + 1));
            else
                temp_container = NodeContainer(Sat_Node.Get( i * (Number_of_Sat_per_orbit) + j),
                                               Sat_Node.Get( i * (Number_of_Sat_per_orbit)    ));

            //NetDeviceContainer same_orbit_NDcontainer;
            NetDeviceContainer device = ptp_same_orbit.Install(temp_container);
            same_orbit_NDcontainer.Add(device);
            if(j!=Number_of_Sat_per_orbit-1)
                std::cout <<  "Node[" << i * (Number_of_Sat_per_orbit) + j << "]&" 
                        <<  "Node[" << i * (Number_of_Sat_per_orbit) + j + 1 <<"] has been connected!"<< std::endl;
            else 
                std::cout <<  "Node[" << i * (Number_of_Sat_per_orbit) + j << "]&" 
                        <<  "Node[" << i * (Number_of_Sat_per_orbit)  <<"] has been connected!!"<< std::endl;
            //IPV4 STACK
            InternetStackHelper stack;
            stack.Install(temp_container);

            //IP Attribute
            std::string addr("192.");
            addr.append(std::to_string(ch1)).append(".").append(std::to_string(ch2)).append(".0");  //over 9 is okey
            std::cout << "子网：" << addr << std::endl;

            std::string sat_address = addr;
            Ipv4AddressHelper ipv4;
            ipv4.SetBase(ns3::Ipv4Address(sat_address.c_str()), "255.255.255.0");
            Sat_ip_same_orbit.Add(ipv4.Assign(device));
            ch2++;
        }
        ch1++;
    }
    ch1 = 1;
    ch2 = 1;

    //相邻轨道，卫星之间的链路建立
    NetDeviceContainer between_orbit_NDcontainer;
    Ipv4InterfaceContainer Sat_ip_between_orbit;
    for (i=0; i<=Number_of_orbit-1; i++)    
    {
        ch2 = 1;
        std::cout << "--------------Orbit:" << i << " AND " << "Orbit:" << i+1 << "--------------" << std::endl;
        for (j=0; j<=Number_of_Sat_per_orbit-1; j++)  
        {
            NodeContainer temp_container;
            if(i!=Number_of_orbit-1)
                temp_container = NodeContainer(Sat_Node.Get(  i  * Number_of_Sat_per_orbit + j),
                                               Sat_Node.Get((i+1)* Number_of_Sat_per_orbit + j));
            else
                temp_container = NodeContainer(Sat_Node.Get(  i  * Number_of_Sat_per_orbit + j),
                                               Sat_Node.Get(  0  * Number_of_Sat_per_orbit + j));

            //NetDeviceContainer between_orbit_NDcontainer;
            NetDeviceContainer device = ptp_between_orbit.Install(temp_container);
            between_orbit_NDcontainer.Add(device);
            if(i!=Number_of_orbit-1) 
                std::cout <<  "Node[" << i*Number_of_Sat_per_orbit+j << "]&" 
                        <<  "Node[" << (i+1)*Number_of_Sat_per_orbit+j <<"] has been connected!"<< std::endl;
            else
                std::cout <<  "Node[" << i*Number_of_Sat_per_orbit+j << "]&" 
                        <<  "Node[" << 0*Number_of_Sat_per_orbit+j <<"] has been connected!!"<< std::endl;

            //IPV4 STACK
            InternetStackHelper stack;
            stack.Install(temp_container);

            //IP Attribute
            std::string addr("191.");
            addr.append(std::to_string(ch1)).append(".").append(std::to_string(ch2)).append(".0");
            std::cout << "子网：" << addr << std::endl;

            std::string sat_address = addr;
            Ipv4AddressHelper ipv4;
            ipv4.SetBase(ns3::Ipv4Address(sat_address.c_str()), "255.255.255.0");
            Sat_ip_between_orbit.Add(ipv4.Assign(device));
            ch2++;
        }
        ch1++;
    }
    //All net-devices--
    NetDeviceContainer all_netdevice = NetDeviceContainer(same_orbit_NDcontainer,between_orbit_NDcontainer);

    //################################################################################### GS-NET ###################################################################################
    PointToPointHelper ptp_same_row;    
    ptp_same_row.SetDeviceAttribute("DataRate",StringValue("100Mbps"));
    ptp_same_row.SetDeviceAttribute("ReceiveErrorModel",StringValue("ns3::RateErrorModel"));
    ptp_same_row.SetChannelAttribute("Delay",StringValue("100ms"));
    PointToPointHelper ptp_between_row;    
    ptp_between_row.SetDeviceAttribute("DataRate",StringValue("100Mbps"));
    ptp_between_row.SetDeviceAttribute("ReceiveErrorModel",StringValue("ns3::RateErrorModel"));
    ptp_between_row.SetChannelAttribute("Delay",StringValue("100ms"));

    int m = 0;
    int n = 0;
    int ch3 = 1;
    int ch4 = 1;
    NetDeviceContainer GS_same_row_container;
    Ipv4InterfaceContainer GS_ip_same_row;
    for (m=0; m<=Number_of_GSnet_row-1; m++)    //i:choose row
    {
        ch3 = 1;
        for (n=0; n<=Number_of_GSnet_per_row-1; n++)  //j:choose one sat in certain orbit
        {
            NodeContainer temp_container;
            if(n!=Number_of_GSnet_per_row-1)
                temp_container = NodeContainer(GSnet_Node.Get( m * (Number_of_GSnet_per_row) + n),
                                               GSnet_Node.Get( m * (Number_of_GSnet_per_row) + n + 1));
            else
                temp_container = NodeContainer(GSnet_Node.Get( m * (Number_of_GSnet_per_row) + n),
                                               GSnet_Node.Get( m * (Number_of_GSnet_per_row)    ));
            NetDeviceContainer device = ptp_same_row.Install(temp_container);
            GS_same_row_container.Add(device);
            //IPV4 STACK
            InternetStackHelper stack;
            stack.Install(temp_container);
            //IP Attribute
            std::string addr("114.");
            addr.append(std::to_string(ch3)).append(".").append(std::to_string(ch4)).append(".0");  //over 9 is okey
            std::string gs_address = addr;
            Ipv4AddressHelper ipv4;
            ipv4.SetBase(ns3::Ipv4Address(gs_address.c_str()), "255.255.255.0");
            GS_ip_same_row.Add(ipv4.Assign(device));
            ch4++;
        }
        ch3++;
    }
    ch3 = 1;
    ch4 = 1;
    NetDeviceContainer GS_between_row_container;
    Ipv4InterfaceContainer GS_ip_between_row;
    for (m=0; m<=Number_of_GSnet_row-1; m++)    
    {
        ch3 = 1;
        for (n=0; n<=Number_of_GSnet_per_row-1; n++)  
        {
            NodeContainer temp_container;
            if(m!=Number_of_GSnet_row-1)
                temp_container = NodeContainer(GSnet_Node.Get(  m  * Number_of_GSnet_per_row + n),
                                               GSnet_Node.Get((m+1)* Number_of_GSnet_per_row + n));
            else
                temp_container = NodeContainer(GSnet_Node.Get(  m  * Number_of_GSnet_per_row + n),
                                               GSnet_Node.Get(  0  * Number_of_GSnet_per_row + n));
            NetDeviceContainer device = ptp_between_row.Install(temp_container);
            GS_between_row_container.Add(device);

            //IPV4 STACK
            InternetStackHelper stack;
            stack.Install(temp_container);
            //IP Attribute
            std::string addr("115.");
            addr.append(std::to_string(ch3)).append(".").append(std::to_string(ch4)).append(".0");
            std::string gs_address = addr;
            Ipv4AddressHelper ipv4;
            ipv4.SetBase(ns3::Ipv4Address(gs_address.c_str()), "255.255.255.0");
            GS_ip_between_row.Add(ipv4.Assign(device));
            ch4++;
        }
        ch3++;
    }

    //测试地面基站与卫星节点之间链路的建立和切断（无任何策略） *********************************************
    PointToPointHelper ptp_GS_Sat;
    ptp_GS_Sat.SetDeviceAttribute("DataRate",StringValue("100Mbps"));
    ptp_GS_Sat.SetChannelAttribute("Delay",StringValue("300ms"));
    //与地面基站链接的卫星编号（这个通过shell输入）---------------------------------------------------------------------------------
    //-------> int Connected_Sat_banngo1 = 2;
    //-------> int Connected_Sat_banngo2 = 22;
    NodeContainer GS_Sat1 = NodeContainer(GS_Node.Get(0),Sat_Node.Get(Connected_Sat_banngo1));
    NetDeviceContainer GS_Sat_device1 = ptp_GS_Sat.Install(GS_Sat1);
    NodeContainer GS_Sat2 = NodeContainer(GS_Node.Get(1),Sat_Node.Get(Connected_Sat_banngo2));
    NetDeviceContainer GS_Sat_device2 = ptp_GS_Sat.Install(GS_Sat2);
    NodeContainer GS_Sat = NodeContainer(GS_Sat1,GS_Sat2);
    std::cout << ">>>>>>>>>>>>> GS0 is connected to Sat[" << Connected_Sat_banngo1 << "]" << std::endl;
    std::cout << ">>>>>>>>>>>>> GS1 is connected to Sat[" << Connected_Sat_banngo2 << "]" << std::endl;

    PointToPointHelper bridge;
    bridge.SetDeviceAttribute("DataRate",StringValue("200Mbps"));
    bridge.SetChannelAttribute("Delay",StringValue("20ms"));
    NodeContainer bridge1 = NodeContainer(GS_Node.Get(0),GSnet_Node.Get(23));
    NetDeviceContainer bridge_device1 = bridge.Install(bridge1);
    NodeContainer bridge2 = NodeContainer(GS_Node.Get(1),GSnet_Node.Get(86));
    NetDeviceContainer bridge_device2 = bridge.Install(bridge2);

    InternetStackHelper stack;
    stack.Install(GS_Sat1);
    stack.Install(GS_Sat2);
    stack.Install(bridge1);
    stack.Install(bridge2);
    //Attention:!!!!!!!!!!!!!!! 不能把两个地面基站设在同一个子网内!!!!!!!!!!!!!!!!!!!!!!!
    Ipv4AddressHelper address;
    Ipv4InterfaceContainer GS_ip;
    address.SetBase("192.168.1.0", "255.255.255.0");
    GS_ip.Add(address.Assign(GS_Sat_device1));
    address.SetBase("192.168.2.0", "255.255.255.0");
    GS_ip.Add(address.Assign(GS_Sat_device2));
    Ipv4InterfaceContainer bridge_ip;
    address.SetBase("192.168.3.0", "255.255.255.0");
    bridge_ip.Add(address.Assign(bridge_device1));
    address.SetBase("192.168.4.0", "255.255.255.0");
    bridge_ip.Add(address.Assign(bridge_device2));


    //Routing Protocal-------------------------------------------------------------------------------
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    //################################################################################################################################################################################
    //GS-Sat Link--------------------------------------------------
    std::cout << "\n----------------------Checking IP-Address[GS-Sat]----------------------" << std::endl;
    int ccnt = 0;
    for(i=0;i<=GS_ip.GetN()/2-1;i++)
    {
        std::cout << "GS IP:" << GS_ip.GetAddress(i+ccnt) << " & " 
                  << "Sat IP:" << GS_ip.GetAddress(i+ccnt+1) << std::endl;
        ccnt++;
    }
    //Same orbit-----------------------------------------------
    std::cout << "\n----------------------Checking IP-Address[Same Orbit]----------------------" << std::endl;
    int cnt = 0;
    for(i=0;i<=Number_of_Sat_per_orbit*Number_of_orbit -1;i++)
    {
        std::cout << "Node[" << i << "]---> " 
                  << "IP1:" << Sat_ip_same_orbit.GetAddress(i+cnt) << "  &  "
                  << "IP2:" << Sat_ip_same_orbit.GetAddress(i+cnt+1) << std::endl;
        cnt++;               
    }
    //Between Orbit-------------------------------------------------
    std::cout << "\n----------------------Checking IP-Address[Between Orbit]----------------------" << std::endl;
    int cntt = 0;
    for(i=0;i<=Number_of_Sat_per_orbit*Number_of_orbit -1;i++)
    {
        std::cout << "Node[" << i << "]---> " 
                  << "IP1:" << Sat_ip_between_orbit.GetAddress(i+cntt) << "  &  "
                  << "IP2:" << Sat_ip_between_orbit.GetAddress(i+cntt+1) << std::endl;
        cntt++;               
    }


    //################################################################################################################################################################################
    //Application [UDP: client ---> server]
    //其中一个GS作为服务器server
    UdpServerHelper Server(114);
    int server_number =22;
    //ApplicationContainer GS_serverApps = Server.Install(GS_Node.Get(server_number));
    ApplicationContainer GS_serverApps = Server.Install(GSnet_Node.Get(server_number));    //现在以对面网络中的任意一个地面基站作为客户端和服务器
    GS_serverApps.Start(Seconds(0.0));
    GS_serverApps.Stop(Seconds(simulation_total_time));
    //另一个GS作为client 
    //UdpClientHelper Client(GS_ip.GetAddress(server_number*2), 114);      //netdevice在container里面的编号正好是节点号的两倍   
    UdpClientHelper Client(GS_ip_between_row.GetAddress(server_number*2), 114);     
    Client.SetAttribute("MaxPackets", UintegerValue(200));
    Client.SetAttribute("Interval", TimeValue(Seconds(0.1)));
    Client.SetAttribute("PacketSize", UintegerValue(1024));
    int client_number = 87;
    //ApplicationContainer GS_clientApps = Client.Install(GS_Node.Get(client_number));
    ApplicationContainer GS_clientApps = Client.Install(GSnet_Node.Get(client_number));

    GS_clientApps.Start(Seconds(0.0));
    GS_clientApps.Stop(Seconds(simulation_total_time));

    std::cout << "\n\n" << std::endl;
    //std::cout << "Transmitting data from:" << GS_ip.GetAddress(server_number*2) << " ----> " << GS_ip.GetAddress(client_number*2) << std::endl;
    std::cout << "\n\n" << std::endl;

    //################################################################################################################################################################################
    //Location
    MobilityHelper mobility;
    //GS's Position ----------------
    srand((unsigned)time(NULL));
    double GS_x = 11;
    double GS_y = 66;  
    double GS_z = 0.0;
    double distance_x = 20;
    double distance_y = 10;
    //GS1
    Ptr<ListPositionAllocator> GS1_position = CreateObject<ListPositionAllocator>();
    GS1_position -> Add(Vector(GS_x,GS_y,GS_z));
    mobility.SetPositionAllocator(GS1_position);
    mobility.Install(GS_Node.Get(0));
    //GS2
    Ptr<ListPositionAllocator> GS2_position = CreateObject<ListPositionAllocator>();
    GS2_position -> Add(Vector(GS_x+distance_x,GS_y+distance_y,GS_z));
    mobility.SetPositionAllocator(GS2_position);
    mobility.Install(GS_Node.Get(1));

    //Satellites' Position (At the beginning) -------------------
    double step_in_orbit = 10.0;
    double step_between_orbit = 5.0;
    double base_x = 3.0;    
    double base_y = 3.0;
    double base_z = 100.0;
    Ptr<ListPositionAllocator> position = CreateObject<ListPositionAllocator>();
    for (i=0; i<Number_of_orbit; i++)
    {
        for (j=0; j<Number_of_Sat_per_orbit; j++)
        {
            //卫星沿着y轴运动，轨道方向为y轴。
            /*
                position->Add(Vector(base_x + step_in_orbit * i,
                                    base_y + step_between_orbit * j,
                                    base_z));
            */
            //卫星沿着x轴运动，轨道方向为x轴。
            position->Add(Vector(base_x + step_between_orbit * j,
                                base_y + step_in_orbit * i,
                                base_z));
        }
    }
    mobility.SetPositionAllocator(position);
    //Satellites' movement model and parameters ----------------------
    mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobility.Install(Sat_Node);
    double velocity_x = 0.5;
    double velocity_y = 0.0;
    double velocity_z = 0.0;
    for(i=0;i <= Number_of_Sat_per_orbit * Number_of_orbit -1;i++)
    {
        Ptr<ConstantVelocityMobilityModel> mob = Sat_Node.Get(i)->GetObject<ConstantVelocityMobilityModel>();
        mob->SetVelocity(Vector(velocity_x, velocity_y, velocity_z));
    }

    //GS-NET(no movement)
    double derta_x = 5.0;
    double derta_y = 6.0;
    double x0 = 50.0;
    double y0 = 30.0;
    double z0 = 0.0;
    Ptr<ListPositionAllocator> GSnet_position = CreateObject<ListPositionAllocator>();
    for (m=0; m<Number_of_GSnet_row; m++)
    {
        for (n=0; n<Number_of_GSnet_per_row; n++)
        {
            GSnet_position->Add(Vector(x0 + derta_x * n,
                                       y0 + derta_y * m,
                                       z0));
        }
    }
    mobility.SetPositionAllocator(GSnet_position);
    mobility.Install(GSnet_Node);

    //Visualization xml##############################################################################################################################################################################################
    AnimationInterface anim("Mesh_connected_ip2_route_trace_groundnet.xml"); 
    //Net-Performance valuation tools
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();


    //##############################################################################################################################################################################################
    //Event Controlling
    Ptr<MyEventHandler> myHandler = CreateObject<MyEventHandler>();
    //Simulator::Schedule(Seconds(2.0),&MyEventHandler::CutP2PLink_GS_Sat,myHandler,GS_Sat_device2,0);
    //Simulator::Schedule(Seconds(10.0),&MyEventHandler::RebuildP2PLink_GS_Sat,myHandler,GS_Sat_device2,0);

    //-----> double simulation_total_time = 20.0;
    Simulator::Stop(Seconds(simulation_total_time));
    Simulator::Run();
    //################################################################## Network Performance Calculation ##################################################################
    uint32_t SentPackets = 0;
    uint32_t ReceivedPackets = 0;
    uint32_t LostPackets = 0;
    int k = 0;
    float AvgThroughput = 0;
    Time Jitter;
    Time Delay;
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
    std::map<FlowId,FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
    std::cout << "\n\n" <<std::endl;
    for (std::map<FlowId,FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter!=stats.end(); ++iter)
    {
        Ipv4FlowClassifier::FiveTuple t= classifier->FindFlow(iter->first);
        NS_LOG_UNCOND(">>>>>Flow ID:" <<iter->first << "<<<<<");
        NS_LOG_UNCOND("Src Addr:"<<t.sourceAddress << " -------> Dst Addr:"<< t.destinationAddress);
        NS_LOG_UNCOND("Sent Packets = "<<iter->second.txPackets);
        NS_LOG_UNCOND("Received Packets = " <<iter->second.rxPackets);
        NS_LOG_UNCOND("Lost Packets = " <<iter->second.txPackets-iter->second.rxPackets);
        NS_LOG_UNCOND("Packet delivery ratio = " <<iter->second.rxPackets*100/iter->second.txPackets << "%");
        NS_LOG_UNCOND("Packet loss ratio = "<< (iter->second.txPackets-iter->second.rxPackets)*100/iter->second.rxPackets << "%");
        NS_LOG_UNCOND("Received Packets = "<<iter->second.rxPackets);
        NS_LOG_UNCOND("Delay = "<<iter->second.delaySum);
        NS_LOG_UNCOND("Jitter = "<<iter->second.jitterSum);
        NS_LOG_UNCOND("Throughput = " <<iter->second.rxBytes * 8.0/(iter->second.timeLastRxPacket.GetSeconds()- iter->second.timeFirstTxPacket.GetSeconds())/1024<<"Kbps");
        SentPackets = SentPackets +(iter->second.txPackets);
        ReceivedPackets = ReceivedPackets +(iter->second.rxPackets);
        LostPackets = LostPackets + (iter->second.txPackets - iter->second.rxPackets);
        AvgThroughput = AvgThroughput + (iter->second.rxBytes * 8.0/(iter->second.timeLastRxPacket.GetSeconds()- iter->second.timeFirstTxPacket.GetSeconds())/1024);
        Delay = Delay + (iter->second.delaySum);
        Jitter = Jitter + (iter->second.jitterSum);

        k = k + 1;
    }

    std::cout << "\n\n" <<std::endl;
    AvgThroughput= AvgThroughput/k;
    NS_LOG_UNCOND("...----Total Results of the simulation.----...."<<std::endl);
    NS_LOG_UNCOND("TotalSent packets = "<< SentPackets);
    NS_LOG_UNCOND("TotalReceived Packets = "<< ReceivedPackets); 
    NS_LOG_UNCOND("Total Lost Packets = " << LostPackets);
    NS_LOG_UNCOND("Packet Loss ratio = "<<((LostPackets*100)/SentPackets)<< "%");
    NS_LOG_UNCOND("Packet delivery ratio = "<<((ReceivedPackets*100)/SentPackets)<< "%" );
    NS_LOG_UNCOND("Average Throughput = " << AvgThroughput<< "Kbps");
    NS_LOG_UNCOND("End to End Delay = " << Delay);
    NS_LOG_UNCOND("End to End Jitter delay = "<< Jitter);
    NS_LOG_UNCOND("Total Flod id "<< k);
    //monitor->SerializeToXmlFile("manet-routing.xml",true, true);
    //#####################################################################################################################################################################
    Simulator::Destroy();

    //SAVE the OUTPUT [Read] -------------------------------------------------------
    std::string filename("SIMOUTPUT_Detailed_forread.txt");
    std::ofstream outputFile(filename,std::ios::app);
    if (!outputFile) {
        std::cerr << "Failed to open the file." << std::endl;
        return 1;
    }
    // 将参数的值写入文件[Read]
    outputFile << "GS(src)----SAT["<< Connected_Sat_banngo1 << "]  ----------> SAT[" << Connected_Sat_banngo2 << "]----GS(dst)" << std::endl;
    outputFile << "TotalSent packets = "<< SentPackets << std::endl;
    outputFile << "Total Lost Packets = " << LostPackets << std::endl;
    outputFile << "Packet Loss ratio = " << ((LostPackets*100)/SentPackets)<< "%" << std::endl;
    outputFile << "Average Throughput = " << AvgThroughput<< "Kbps" << std::endl;
    outputFile << "End to End Delay = " << Delay << std::endl;
    outputFile << "End to End Jitter delay = "<< Jitter << std::endl;
    outputFile << "\n\n" << std::endl;
    // 关闭文件流
    outputFile.close();
    std::cout << "\n\n ++++++++ Simulation Parameters have been saved to <" << filename << ">" << std::endl;

    //SAVE the OUTPUT [Useable file] -------------------------------------------------------
    std::string filename_foruse("SIMOUTPUT_foruse.txt");
    std::ofstream outputFile_foruse(filename_foruse,std::ios::app);
    if (!outputFile_foruse) {
        std::cerr << "Failed to open the file." << std::endl;
        return 1;
    }
    // 将参数的值写入文件[Read]
    outputFile_foruse << ((LostPackets*100)/SentPackets) << std::endl;
    outputFile_foruse <<  AvgThroughput << std::endl;
    outputFile_foruse <<  Delay << std::endl;
    outputFile_foruse << Jitter << std::endl;
    // 关闭文件流
    outputFile_foruse.close();
    std::cout << " ++++++++ Useable Simulation Parameters have been saved to <" << filename_foruse << ">" << std::endl;

    return 0; 
}

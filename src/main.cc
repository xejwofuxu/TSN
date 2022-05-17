#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pcap.h> /* GIMME a libpcap plz! */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include <faker_tsn.h>
#include "test.h"

using namespace faker_tsn;

static void Recv(const char* deviceName, int num, TSNFrameBody* tsn) {
    union ethframe {
        struct
        {
            struct ethhdr header;
            unsigned char data[ETH_DATA_LEN];
        } field;
        unsigned char buffer[ETH_FRAME_LEN];
    };

    /* create raw socket */
    int sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_8021Q));
    INFO("create socket");

    /* get interface index */
    int ifindex = LinkLayerInterface::getIndex(deviceName);
    INFO("get interface index");

    /* get interface mac address */
    unsigned char mac[6];
    LinkLayerInterface::getMacAddress(deviceName)->getRaw(mac);
    INFO("get mac address");

    /* put interface into promiscuous mode */
    // struct packet_mreq mreq = {0};
    // mreq.mr_ifindex = ifindex;
    // mreq.mr_type = PACKET_MR_PROMISC;
    // if (setsockopt(sockfd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1) {
    //     perror("setsockopt");
    //     exit(1);
    // }

    /* set linklayer address */
    struct sockaddr_ll saddrll;
    memset((void*)&saddrll, 0, sizeof(saddrll));
    saddrll.sll_family = PF_PACKET;                           // set family
    saddrll.sll_ifindex = ifindex;                            // set index
    saddrll.sll_halen = ETH_ALEN;                             // set address length
    saddrll.sll_protocol = htons(ETH_P_ALL);                  // set protocp
    memcpy((void*)(saddrll.sll_addr), (void*)mac, ETH_ALEN);  // set mac
    INFO("construct link layer address (sockaddr_ll) struct");

    /* bind handle to interface */
    if (bind(sockfd, (struct sockaddr*)&saddrll, sizeof(saddrll)) == -1) {
        perror("bind");
        exit(1);
    }
    INFO("bind socket to link layer address (sockaddr_ll) struct\n");

    /* receive data */
    char recvbuf[ETH_FRAME_LEN + 4 + 6];
    socklen_t src_addr_len = sizeof(saddrll);
    for (int i = 0; i < num; i++) {
        if (recv(sockfd, recvbuf, ETH_FRAME_LEN + 4 + 6, 0) > 0) {
            // if (recvfrom(sockfd, recvbuf, ETH_FRAME_LEN, 0, (struct sockaddr*)&saddrll, &src_addr_len) > 0) {
            // if (recvfrom(sockfd, recvbuf, ETH_FRAME_LEN, 0, NULL, NULL) > 0) {
            INFO("recv success!");
        } else {
            INFO("recv failed!");
        }

        // construct frame
        // union ethframe* frame = (union ethframe*)recvbuf;
        // unsigned char dest[ETH_ALEN];
        // unsigned char src[ETH_ALEN];
        // unsigned short proto;
        // char data[ETH_DATA_LEN];
        // memcpy(dest, frame->field.header.h_dest, ETH_ALEN);
        // memcpy(dest, frame->field.header.h_source, ETH_ALEN);
        // proto = ntohs(frame->field.header.h_proto);
        // memcpy(data, frame->field.data, ETH_DATA_LEN);
        // INFO("construct ethernet frame");
        // INFO("dest mac = " + MacAddress::toRaw(dest));
        // INFO("src mac = " + MacAddress::toRaw(src));
        // INFO("protocol = " + std::to_string(proto));
        // INFO("data = " + std::string(data));

        INFO("Decode frame");

        union tsn_frame* frame = (union tsn_frame*)recvbuf;
        char data[ETH_DATA_LEN];
        memcpy(data, frame->filed.data, ETH_DATA_LEN);

        // ethernet header
        INFO("dest mac = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(frame), 6));
        INFO("src mac = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(frame) + 6, 6));
        INFO("protocol = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(frame) + 12, 2));
        // vlan tag
        INFO("TCI = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(frame) + 14, 2));
        VlanTCI vlanTCI = VlanTCI::parse(frame->filed.header.vlan_tag.h_vlan_TCI);
        INFO("VlanTCI.pcp = " + std::to_string(vlanTCI.pcp));
        INFO("protocol = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(frame) + 16, 2));
        INFO("reserved = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(frame) + 18, 2));
        // r-tag
        INFO("sequence number = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(frame) + 20, 2));
        INFO("protocol = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(frame) + 22, 2));
        // data
        // INFO("data = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(frame) + 24, 13));
        INFO("data(string) = " + std::string(reinterpret_cast<char*>(frame) + 24));

        // 转换
        // tsn->setType(IEEE_802_1Q_TSN_FRAME);
        // tsn->setPCP(vlanTCI.pcp);
        // tsn->setVID(vlanTCI.vid);
        // // tsn->setSeq();
        // tsn->setData(std::string(reinterpret_cast<char*>(frame) + 24));
    }

    close(sockfd);
}

int main(int argc, char **argv) {
    // test_state_pattern();
    // test_logger();
    // test_port_manager();
    // test_reactor();

    char *dev; /* name of the device to use */
    char *net; /* dot notation of the network address */
    char *mask;/* dot notation of the network mask */
    int ret;   /* return code */
    char errbuf[PCAP_ERRBUF_SIZE];
    bpf_u_int32 netp; /* ip */
    bpf_u_int32 maskp;/* subnet mask */
    struct in_addr addr;

    /* ask pcap to find a valid device for use to sniff on */
    dev = pcap_lookupdev(errbuf);

    /* error checking */
    if(dev == NULL)
    {
        printf("%s\n",errbuf);
        exit(1);
    }

    // /* print out device name */
    printf("DEV: %s\n",dev);

    // /* ask pcap for the network address and mask of the device */
    ret = pcap_lookupnet(dev,&netp,&maskp,errbuf);

    if(ret == -1)
    {
        printf("%s\n",errbuf);
        exit(1);
    }

    /* get the network address in a human readable form */
    addr.s_addr = netp;
    net = inet_ntoa(addr);

    if(net == NULL)/* thanks Scott :-P */
    {
        perror("inet_ntoa");
        exit(1);
    }
    printf("NET: %s\n",net);

    /* do the same as above for the device's mask */
    addr.s_addr = maskp;
    mask = inet_ntoa(addr);

    if(mask == NULL) {
        perror("inet_ntoa");
        exit(1);
    }

    printf("MASK: %s\n",mask);

    std::shared_ptr<IPort> port = std::make_shared<DataPort>(dev);
    std::shared_ptr<IPortState> creationState = std::make_shared<CreationPortState>();
    creationState->doAction(port);

    // add port into port manager
    std::shared_ptr<PortManager> portManager = TSNContext::getInstance().getPortManager();
    portManager->appendPort(port);

    // load mac table
    MacTable::loadRouteXML("/home/reptile/下载/TSN/config/routes.xml");


    // QueueContext que(port);

    // TSNFrameBody* tsn;
    // // socket 监听捕捉
    // Recv(dev, 2, tsn);
    // // 将捕捉到的帧转化为TSNFrame类的结构存入队列中进行处理

    

    // // 转发
    // // enable reactor
    // Reactor::getInstance().handle_events();

    return 0;
}
#include <faker_tsn.h>
#include <gtest/gtest.h>

#include <string>

#include "../src/networking/VlanTCI.h"
#include "../src/networking/if_ether.h"
#include "../src/utils/container/ConvertUtils.h"

using namespace faker_tsn;

void port_status(unsigned int flags)
{
	if(flags & IFF_UP) {	       /* Interface is up.  */
		printf("is up\n");		
	}
	if(flags & IFF_BROADCAST) {    /* Broadcast address valid.  */
		printf("is broadcast\n");	
	}
	if(flags & IFF_LOOPBACK) {     /* Is a loopback net.  */
		printf("is loop back\n");	
	}
	if(flags & IFF_POINTOPOINT) {  /* Interface is point-to-point link.  点到点*/
		printf("is point to point\n");	
	}
	if(flags & IFF_RUNNING) {
		printf("is running\n");	
	}
	if(flags & IFF_PROMISC)	{
		printf("is promisc\n");	
	}
}

static void TestGetMulticastMacAddress() {
    printf("测试 1: TestGetMulticastMacAddress \n");
    // 套接字，协议族数据包，接受所有类型的数据帧
    int sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    // Interface request，接口请求
    struct ifreq buffer;
    int ifindex;
    memset(&buffer, 0x00, sizeof(buffer));
    strncpy(buffer.ifr_name, "ens33", IFNAMSIZ);  // ifr_name

    // 系统调用，SIOCADDMULTI 不能得到硬件地址
    // if (ioctl(sockfd, SIOCADDMULTI, &buffer) < 0) {  /* Multicast address lists	*/
    //     printf("Error: could not get interface index\n");
    //     // TODO handle error
    // }

    // 1、系统调用，获取网卡 MAC 地址，socket io control get interface hardware address
    if (ioctl(sockfd, SIOCGIFHWADDR, &buffer) < 0) {   /* Get hardware address		*/
        printf("Error: could not get MAC address\n");
    }
    // 输出 MAC 地址
    unsigned char macaddr[ETH_ALEN]={0}; // ETH_ALEN（6）是 MAC 地址长度
    memcpy(macaddr, buffer.ifr_hwaddr.sa_data, ETH_ALEN); //取输出的 MAC 地址
    printf("MAC 地址：");
    for(int i=0; i<ETH_ALEN; i++) printf("%02x ", macaddr[i]);


    // 2、系统调用，获取网卡的 IP 地址，socket io control get interface address
    if (ioctl(sockfd, SIOCGIFADDR, &buffer) < 0) {    /* get PA address		*/
        printf("Error: could not get IP address\n");
    }
    // 输出 IP 地址
    std::cerr << "\nIP 地址: " << inet_ntoa(((struct sockaddr_in*)&(buffer.ifr_ifru.ifru_addr))->sin_addr) << '\n';


    struct ifconf ifc;
    // ifconf 必须进行以下初始化 ioctl 才不会返回 -1
    struct ifreq buf[32]; // buf 多大没影响，后面系统调用都会重新赋值
    ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = (caddr_t) buf;
    // 3、系统调用，获取接口配置，socket io control get interface config
    int ret = ioctl(sockfd, SIOCGIFCONF, &ifc);  
    if(ret < 0) {
		printf("get if config info failed\n");
	}
	/* 网口总数 */	
    int num = ifc.ifc_len/sizeof(struct ifreq);
	printf("interface num is %d\n\n", num);
	while(num-- > 0) {
        printf("net device: %s\n", buf[num].ifr_name);	

		/* 获取第 n 个网口信息 */
		ret = ioctl(sockfd, SIOCGIFFLAGS, (char*)&buf[num]);
		if(ret) continue;

        /* 获取第 n 个网口的索引值 */
        // INFO("interface index is: " + std::to_string(buf[num].ifr_ifindex));
        printf("interface index is: %d\n",buf[num].ifr_ifru.ifru_ivalue);
 
		/* 获取网口状态 */
        printf("flag is: %d\n", buf[num].ifr_flags);
		port_status(buf[num].ifr_flags);
		
		/* 获取当前网卡的 IP 地址 */
		ret = ioctl(sockfd, SIOCGIFADDR, (char*)&buf[num]);
		if(ret) continue;
		printf("IP address is: %s\n", inet_ntoa(((struct sockaddr_in *)(&buf[num].ifr_addr))->sin_addr));
 
		/* 获取当前网卡的 MAC 地址 */
		ret = ioctl(sockfd, SIOCGIFHWADDR, (char*)&buf[num]);
		if(ret) continue;
		printf("MAC address is: %02x:%02x:%02x:%02x:%02x:%02x\n\n",
			(unsigned char)buf[num].ifr_hwaddr.sa_data[0],
			(unsigned char)buf[num].ifr_hwaddr.sa_data[1],
			(unsigned char)buf[num].ifr_hwaddr.sa_data[2],
			(unsigned char)buf[num].ifr_hwaddr.sa_data[3],
			(unsigned char)buf[num].ifr_hwaddr.sa_data[4],
			(unsigned char)buf[num].ifr_hwaddr.sa_data[5]
			);
    }

    // 4、系统调用，获取接口索引（LinkLayerInterface 类中自定义的 getIndex() 函数），socket io control get interface index
    sockfd = socket(PF_PACKET, SOCK_RAW, 0);
    if (ioctl(sockfd, SIOCGIFINDEX, &buffer) < 0) {  // used ioctl to find interface index
        ERROR("Could not get interface index");
        throw IndexNotFound();
    }
    // 输出接口索引，buffer.ifr_name 是 ens33
    INFO("interface index is: " + std::to_string(buffer.ifr_ifindex));
    printf("net device: %s\n", buffer.ifr_ifrn.ifrn_name);
    // 输出 IP 地址
    std::cerr << "IP 地址: " << inet_ntoa(((struct sockaddr_in*)&(buffer.ifr_ifru.ifru_addr))->sin_addr) << '\n';
    // 输出 MAC 地址
    memcpy(macaddr, buffer.ifr_hwaddr.sa_data, ETH_ALEN); //取输出的 MAC 地址
    printf("MAC 地址：");
    for(int i=0; i<ETH_ALEN; i++) printf("%02x ", macaddr[i]);
}

static void TestGetMacAddress() {
    printf("\n\n\n测试 2: TestGetMacAddress \n");
    try {
        MacAddress* macAddress = LinkLayerInterface::getMacAddress("ens33"); // 获取 ens33 网卡的 MAC 地址
        // MAC 地址转换，输出十进制版的 MAC 地址
        unsigned char rawMac[ETH_ALEN];
        macAddress->getRaw(rawMac);
        printf("raw MAC address is: ");
        for (int i = 0; i < ETH_ALEN; i++) {
            printf("%d ", rawMac[i]);
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}

static void TestGetIndex() {
    printf("\n\n\n测试 3: TestGetIndex \n");
    try {
        int index = LinkLayerInterface::getIndex("ens33");
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}

static void TestFindInterface() {
    printf("\n\n测试 4: TestFindInterface \n");
    try {
        LinkLayerInterface* interface = LinkLayerInterface::findInterface("ens33");
        INFO(interface->toString()); // LinkLayerInterface 类对象包含成员：name、index 和 mac
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}

static void TestIfEther() {
    printf("\n\n测试 5: TestIfEther \n");
    const char* deviceName = "ens33";

    /* get interface index */
    int ifindex = LinkLayerInterface::getIndex(deviceName);
    printf("\n");

    /* get interface mac address */
    unsigned char src[8];
    LinkLayerInterface::getMacAddress(deviceName)->getRaw(src);
    printf("\n");

    // test Ethernet header：源地址 h_source、目的地址 h_dest、类型 h_proto
    struct ethhdr eth_hdr;
    INFO("Ethernet header length = " + std::to_string(sizeof(eth_hdr)));
    memset(&eth_hdr, 0x00, sizeof(eth_hdr));
    memcpy(&eth_hdr.h_source, src, ETH_ALEN);  // set src mac
    eth_hdr.h_proto = htons(ETH_P_8021Q);      // set IEEE 802.1Q protocol
    printf("以太网帧头的源 MAC 地址：");
    for(int i=0; i<ETH_ALEN; i++) printf("%02x ", eth_hdr.h_source[i]);
    printf("\n以太网帧头的目的 MAC 地址：");
    for(int i=0; i<ETH_ALEN; i++) printf("%02x ", eth_hdr.h_dest[i]);
    INFO("\nprotocol = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(&eth_hdr.h_proto), 2) + "\n");

    // test TCI，802.1Q 标记（4 字节）由 TPID（2 字节）和 TCI（2 字节）组成
    // TPID（标记协议标识符）：值总是 0x 8100，表示 802.1Q 标记
    // TCI（标记控制信息）（1）pcp：3 位的优先级字段 Priority
    //                 （2）dei：1 位的 CFI（规范格式指示器），指示 MAC 地址的格式，0 表示以太网 MAC 地址
    //                 （3）vid：最后 12 位，VLAN 标识符（VID），用于标识该数据帧应该发送到哪一个 VLAN
    // 1、使用官方库定义的结构体 vlan_tci，发现只可以定义结构体，无法转换得到完整的字段，所以写了自己的结构体 vlan_tci，其中给出了相互转换的方式
    struct vlan_tci tci;
    INFO("TCI length = " + std::to_string(sizeof(tci)));
    memset(&tci, 0x00, sizeof(tci));
    tci.pcp = 1;
    tci.vid = 1;
    INFO("pcp = " + std::to_string(tci.pcp));  // 001
    INFO("dei = " + std::to_string(tci.dei));  // 0
    INFO("vid = " + std::to_string(tci.vid));  // 000000000001
    // N    0x2001 : 0010 0000 , 0000 0001
    // H    0x0120 : 0000 0001 , 0010 0000
    INFO("TCI = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(&tci), 2) + "\n");

    // 2、网络字节顺序
    __be16 _tci = htons(0xE001);  // 1110 0000 0000 0001，将整型变量从主机字节顺序转变成网络字节顺序（从高到低）
    INFO("n_TCI = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(&_tci), 2));
    __be16 __tci = 0xE001;
    INFO("h_TCI = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(&__tci), 2) + "\n");

    // 3、使用自己定义的结构体 VlanTCI
    VlanTCI vlanTCI = VlanTCI::parse(_tci);
    INFO("VlanTCI.pcp = " + std::to_string(vlanTCI.pcp));
    INFO("VlanTCI.dei = " + std::to_string(vlanTCI.dei));
    INFO("VlanTCI.vid = " + std::to_string(vlanTCI.vid));

    // test VLAN tag
    struct vlan_hdr vlan_tag;
    INFO("VLAN tag length = " + std::to_string(sizeof(vlan_tag)));
    memset(&vlan_tag, 0x00, sizeof(vlan_tag));
    memcpy(&vlan_tag.h_vlan_TCI, &_tci, sizeof(_tci));        // set TCI
    vlan_tag.h_vlan_encapsulated_proto = htons(ETH_P_8021Q);  // set IEEE 1722 protocol
    INFO("TCI = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(&vlan_tag.h_vlan_TCI), 2));
    INFO("protocol = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(&vlan_tag.h_vlan_encapsulated_proto), 2) + "\n");

    // test R-tag
    struct rtag_hdr rtag;
    INFO("R-tag length = " + std::to_string(sizeof(rtag)));
    memset(&rtag, 0x00, sizeof(rtag));
    rtag.h_rtag_seq_num = htons(1);  // set
    rtag.h_rtag_encapsulated_proto = htons(ETH_P_ALL);
    INFO("reserved = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(&rtag.h_rtag_rsved), 2));
    INFO("sequence number = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(&rtag.h_rtag_seq_num), 2));
    INFO("protocol = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(&rtag.h_rtag_encapsulated_proto), 2) + "\n");

    // construct TSN frame
    union tsn_frame frame;
    char* data = "hello world\n";
    INFO("raw data = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(data), 13));
    memset(&frame, 0x00, sizeof(tsn_frame));
    INFO("TSN frame length = " + std::to_string(sizeof(frame)));
    memcpy(&frame.filed.header.eth_hdr, &eth_hdr, sizeof(eth_hdr));    // 设置以太网帧头
    memcpy(&frame.filed.header.vlan_tag, &vlan_tag, sizeof(vlan_tag)); // 设置 vlan tag
    memcpy(&frame.filed.header.r_tag, &rtag, sizeof(rtag));            // 设置 r_tag
    memcpy(frame.filed.data, data, strlen(data));                      // 设置 data
    // eth_hdr
    printf("以太网帧头的源 MAC 地址：");
    for(int i=0; i<ETH_ALEN; i++) printf("%02x ", frame.filed.header.eth_hdr.h_source[i]);
    printf("\n以太网帧头的目的 MAC 地址：");
    for(int i=0; i<ETH_ALEN; i++) printf("%02x ", frame.filed.header.eth_hdr.h_dest[i]);
    INFO("\nprotocol = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(&frame.filed.header.eth_hdr.h_proto), 2));
    // vlan_tag
    INFO("TCI = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(&frame.filed.header.vlan_tag.h_vlan_TCI), 2));
    INFO("protocol = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(&frame.filed.header.vlan_tag.h_vlan_encapsulated_proto), 2));
    // rtag
    INFO("reserved = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(&frame.filed.header.r_tag.h_rtag_rsved), 2));
    INFO("sequence number = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(&frame.filed.header.r_tag.h_rtag_seq_num), 2));
    INFO("protocol = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(&frame.filed.header.r_tag.h_rtag_encapsulated_proto), 2));
    // // data
    INFO("data = " + ConvertUtils::converBinToHexString(reinterpret_cast<unsigned char*>(frame.filed.data), 13));
}

static void TestMacTable() {
    MacTable::loadRouteXML("/home/reptile/桌面/tsn_app/config/routes.xml"); 
    INFO("\n" + MacTable::toString());
}

TEST(TEST_NETWORKING, TEST_NETWORKING_INTERFACE) {
    TestGetMulticastMacAddress(); // 通过套接字获取所有网卡设备的 MAC 地址
    TestGetMacAddress();          // 通过抓包库 libpcap 获取指定网卡设备的 MAC 地址（LinkLayerInterface 类中定义的方法）
    TestGetIndex();               // 通过套接字获取指定网卡设备的接口索引（LinkLayerInterface 类中定义的方法）
    TestFindInterface();          // 通过套接字和抓包库 libpcap 实现 LinkLayerInterface 类对象的定义（name、index、mac）
    TestIfEther();                // 上面四种的综合
}

TEST(TEST_NETWORKING, TEST_MAC_TABLE) {
    TestMacTable();
}
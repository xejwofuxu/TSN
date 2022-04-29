#include "LinkLayerInterface.h"

namespace faker_tsn {

void LinkLayerInterface::bind(int sockfd, std::shared_ptr<IAddress> address) {
    std::shared_ptr<MacAddress> macAddress = std::dynamic_pointer_cast<MacAddress>(address);
    struct sockaddr_ll saddrll;
    unsigned char mac[ETH_ALEN];
    this->m_macAddress->getRaw(mac);
    memset((void*)&saddrll, 0, sizeof(saddrll));
    saddrll.sll_family = macAddress->getFamily();             // set family
    saddrll.sll_ifindex = this->m_index;                      // set index
    saddrll.sll_halen = ETH_ALEN;                             // set address length
    saddrll.sll_protocol = htons(macAddress->getProtocol());  // set protocp
    memcpy((void*)(saddrll.sll_addr), (void*)mac, ETH_ALEN);  // set mac

    if (::bind(sockfd, (struct sockaddr*)&saddrll, sizeof(saddrll)) == -1) {
        ERROR("Bind failed");
        throw BindSocketToInterfaceFailed();
    }
}

LinkLayerInterface* LinkLayerInterface::findInterface(const char* name) {
    // 包的接口类，通过抓包库 libpcap 实现
    pcap_if_t* devs;
    char errbuf[PCAP_ERRBUF_SIZE];
    // pcap_findalldevs()查找所有设备并存到 devs 中
    if (pcap_findalldevs(&devs, errbuf) < 0) {
        ERROR("Couldn't open devices: " + std::string(errbuf));
        throw InterfaceNotFound();
    }

    // 初始化链路层接口类对象
    LinkLayerInterface* interface = new LinkLayerInterface();
    bool flag = false;
    // 遍历 devs 查找符合 name 的网卡设备
    for (pcap_if_t* dev = devs; dev != NULL; dev = dev->next) {
        if (strcmp(dev->name, name) == 0) {
            interface = new LinkLayerInterface(name); // 赋值名字
            flag = true;
            break;
        }
    }
    /* free all pcap_if_t struct */
    pcap_freealldevs(devs);

    if (flag == false) {
        throw InterfaceNotFound();
    } else {
        return interface;
    }
}

std::vector<LinkLayerInterface> LinkLayerInterface::findAllInterfaces() {
}

MacAddress* LinkLayerInterface::getMacAddress(const char* name) {
    // 包的接口类，通过抓包库 libpcap 实现
    pcap_if_t* devs;
    char errbuf[PCAP_ERRBUF_SIZE];
    // pcap_findalldevs()查找所有设备并存到 devs 中
    if (pcap_findalldevs(&devs, errbuf) < 0) {
        ERROR("Couldn't open devices: " + std::string(errbuf));
        throw MacAddressNotFound();
    }

    int count = 1; // 计数
    bool flag = false;
    MacAddress* macAddress = new MacAddress();
    // 遍历 devs 查找符合 name 的网卡设备
    for (pcap_if_t* dev = devs; dev != NULL; dev = dev->next) {
        printf("counter : %d\n", count++);

        if (strcmp(dev->name, name) != 0)
            continue;
        // 找到符合 name 的网卡设备，遍历设备的地址
        for (pcap_addr_t* addr = dev->addresses; addr != NULL; addr = addr->next) {
            if (addr->addr == NULL)
                continue;
            // sa_family 套接字地址族，类型是 PF_PACKET（协议族包）
            if (addr->addr->sa_family != PF_PACKET)
                continue;
            // 链路层 
            sockaddr_ll* hwaddr = (sockaddr_ll*)addr->addr;
            unsigned char mac[ETH_ALEN];
            memcpy((void*)(mac), (void*)hwaddr->sll_addr, ETH_ALEN);
            // macAddress->setIndex(LinkLayerInterface::getIndex(name)); // 好像不需要也行？因为不像第一个测试（test_networking.cc 中的 TestGetMulticastMacAddress()），是通过套接字获取 MAC 地址
            macAddress->setMacAddress(mac);
            flag = true;
            INFO("MAC address is: " + std::string(macAddress->toString()));
        }
    }

    /* free all pcap_if_t struct */
    pcap_freealldevs(devs);

    if (flag == false) {
        throw MacAddressNotFound();
    } else {
        return macAddress;
    }
}

int LinkLayerInterface::getIndex(const char* name) {
    // 通过套接字实现
    struct ifreq buffer;  // interface request struct used for socket ioctl's
    int sockfd = socket(PF_PACKET, SOCK_RAW, 0);
    memset(&buffer, 0x00, sizeof(buffer));
    strncpy(buffer.ifr_name, name, IFNAMSIZ);        // set interface name
    // 系统调用，获取接口索引，socket io control get interface index
    if (ioctl(sockfd, SIOCGIFINDEX, &buffer) < 0) {  // used ioctl to find interface index
        ERROR("Could not get interface index");
        throw IndexNotFound();
    }
    close(sockfd);
    INFO("interface index is: " + std::to_string(buffer.ifr_ifindex));
    return buffer.ifr_ifindex;
}

}  // namespace faker_tsn

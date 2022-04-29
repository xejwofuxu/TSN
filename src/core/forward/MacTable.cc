#include "MacTable.h"
#include <iomanip>

namespace faker_tsn {

std::unordered_map<mac_token, ForwardPair> MacTable::items;

std::string MacTable::toString() {
    std::stringstream ss;
    // <mac addr, forward pair> map
    for (auto it = MacTable::items.begin(); it != MacTable::items.end(); it++) {
        ss << "MAC_TOKEN: "; // MAC 地址
        unsigned char mac[ETH_ALEN];
        memcpy(&mac, &(it->first), ETH_ALEN); 
        // 设置域宽 setw 和填充 setfill 头文件 #include <iomanip>
        for(int i = 0; i < 6; i++) ss << std::hex << std::setw(2) << std::setfill('0') << (int)mac[i] << " ";
        ss << " ";
        ss << it->second.toString();      // 结构体 ForwardPair（包含端口和端口存在时间），调用其中定义的函数 toString()
        ss << std::endl;
    }
    return ss.str();
}

void MacTable::loadRouteXML(std::string filename) {
    XMLDocument doc;
    doc.LoadFile(filename.c_str());
    XMLElement* root = doc.RootElement();
    /* get filetr database */
    XMLElement* fileterDB = root->FirstChildElement("filteringDatabase");

    // 匹配指定设备（config.ini 中的 deviceName）
    while (fileterDB) {
        INFO(fileterDB->Attribute("id"));
        // 基本形式为strcmp(str1,str2)，若str1=str2，则返回零；若str1<str2，则返回负数；若str1>str2，则返回正数。
        if (strcmp(fileterDB->Attribute("id"), ConfigSetting::getInstance().get<const char*>("deviceName")) == 0)
            break;
        fileterDB = fileterDB->NextSiblingElement();
    }
    if (!fileterDB) {
        ERROR("lack of filerdatabase");
        throw LackOfTagException("filteringDatabase");
    }
    
    /* get static list */
    XMLElement* staticList = fileterDB->FirstChildElement("static");
    /* get forward contents */
    XMLElement* forwardList = staticList->FirstChildElement("forward");
    /* get individual address */
    XMLElement* individualAddress = forwardList->FirstChildElement("individualAddress");
    while (individualAddress) {
        if (strcmp(individualAddress->Name(), "individualAddress") == 0) {
            const char* portString = individualAddress->Attribute("port");
            const char* macString = individualAddress->Attribute("macAddress");
            // port
            std::vector<unsigned short> portIndexes;
            portIndexes = MacTable::parsePortIndex(portString);
            printf("port: ");
            std::vector<unsigned short>::iterator it;
            for (it = portIndexes.begin(); it != portIndexes.end(); it++) printf("%u ", *it);
            // MAC 地址
            unsigned char mac[ETH_ALEN];
            MacTable::parseMacAddress(macString, mac);
            printf("MAC 地址: ");
            for(int i = 0; i < 6; i++) printf("%02x ", mac[i]);
            printf("\n");
            MacTable::addItem(mac, portIndexes);
        }
        individualAddress = individualAddress->NextSiblingElement();
    }
    /* get multicastAddress */
    XMLElement* multicastAddress = forwardList->FirstChildElement("multicastAddress");
    while (multicastAddress) {
        if (strcmp(multicastAddress->Name(), "multicastAddress") == 0) {
            const char* portString = multicastAddress->Attribute("ports");
            const char* macString = multicastAddress->Attribute("macAddress");
            // port
            std::vector<unsigned short> portIndexes;
            portIndexes = MacTable::parsePortIndex(portString);
            printf("port: ");
            std::vector<unsigned short>::iterator it;
            for (it = portIndexes.begin(); it != portIndexes.end(); it++) printf("%u ", *it);
            // MAC 地址
            unsigned char mac[ETH_ALEN];
            MacTable::parseMacAddress(macString, mac);
            printf("MAC 地址: ");
            for(int i = 0; i < 6; i++) printf("%02x ", mac[i]);
            printf("\n"); 
            MacTable::addItem(mac, portIndexes);
        }
        multicastAddress = multicastAddress->NextSiblingElement();
    }
}

void MacTable::parseMacAddress(const std::string& macString, unsigned char* mac) {
    for (int i = 0, j = 0; i < 6; i++, j += 3) {
        std::string temp = macString.substr(j, 2);
        int n = std::stoi(temp, 0, 16);
        memcpy(mac + i, (unsigned char*)&n, 1);
    }
}

/* conver port string to port vector */
std::vector<unsigned short> MacTable::parsePortIndex(const std::string& portString) {
    std::vector<unsigned short> output;
    if (portString.find(" ") == std::string::npos) {  // single port
        unsigned short portIndex = std::atoi(portString.c_str());
        output.push_back(portIndex);
    } else {  // multiple ports
        std::string strs = portString + " ";
        size_t start = 0;
        size_t end = strs.find(" ");
        while (end != std::string::npos) {
            unsigned short portIndex = std::atoi(strs.substr(start, end - start).c_str());
            output.push_back(portIndex);
            start = end + 1;
            end = strs.find(" ", start);
        }
    }
    return output;
}

}  // namespace faker_tsn

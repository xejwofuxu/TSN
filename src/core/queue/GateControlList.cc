#include "GateControlList.h"

namespace faker_tsn {

// REFLECT(GateControlList);

void GateControlList::loadScheduleXML(std::string filename) {
    XMLDocument doc;
    doc.LoadFile(filename.c_str());
    XMLElement* schedule = doc.RootElement();
    /* get period */
    const char* timeUnit = schedule->Attribute("timeUnit");
    long interval = atoi(schedule->FirstChildElement("cycle")->GetText());
    INFO("interval: " + std::to_string(interval) + "\n");
    Time::TimeInterval timeInterval = Time::converIntegerToTimeInterval(interval, timeUnit);
    /* get gate control list items */
    XMLElement* device = schedule->FirstChildElement("switch");

    // 匹配指定设备（config.ini 中的 deviceName）
    while (device) {
        // 基本形式为strcmp(str1,str2)，若str1=str2，则返回零；若str1<str2，则返回负数；若str1>str2，则返回正数。
        if (strcmp(device->Attribute("name"), ConfigSetting::getInstance().get<const char*>("deviceName")) == 0)
            break;
        device = device->NextSiblingElement();
    }
    if (!device) {
        ERROR("lack of device");
    }

    // 匹配指定端口（门控列表类中的端口号 m_portId）
    XMLElement* port = device->FirstChildElement("port");
    while (static_cast<unsigned int>(atoi(port->Attribute("id"))) != this->m_portId) {
        printf("端口号匹配不对：%d\n", atoi(port->Attribute("id")));
        port = port->NextSiblingElement();
    }

    // 处理数据包
    XMLElement* entry = port->FirstChildElement("entry");
    while (entry) {
        // length，时长
        long length = atoi(entry->FirstChildElement("length")->GetText());
        INFO("length: " + std::to_string(length));
        // bitvector
        std::bitset<8> bitvector(std::string(entry->FirstChildElement("bitvector")->GetText()));
        INFO("bitvector: " + bitvector.to_string() +"\n");
        // 传输门类赋值：length 根据时长设置时间间隔、bitvector 传输门状态
        this->m_gcl.emplace_back(
            Time::converIntegerToTimeInterval(length, timeUnit),
            bitvector
        );
        entry = entry->NextSiblingElement();
    }
}

}  // namespace faker_tsn

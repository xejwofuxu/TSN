#include <gtest/gtest.h>
#include "faker_tsn.h"
#include "test.h"

using namespace faker_tsn;

static void TestReactor() {
    // stdinEventHandler, EVENT_TYPE::READ
    // std::shared_ptr<IPort> port = std::make_shared<ConsolePort>();  // create port

    // port->registerEventHandler();
    // Reactor::getInstance().handle_events();

    // recvTSNFrameEventHandler, EVENT_TYPE::READ
    // sendTSNFrameEventHandler, EVENT_TYPE::WRITE
    PortManager portManager;
    // 获取所有设备名称
    portManager.findAllDeviceName();
    portManager.createPortFromDeviceNameList();
    // 获取端口号
    auto port = portManager.getPort(0);
    port->registerEventHandler();
    Reactor::getInstance().handle_events();
    port->sendTest();
}

TEST(TEST_REACTOR, TEST_REACTOR) {
    TestReactor();
}
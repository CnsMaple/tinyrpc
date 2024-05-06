#include "Log/Log.h"
#include "Rpc/PBProtocol.h"
#include "Rpc/RpcChannel.h"
#include "Status/Status.h"
#include "ThreadPool/ThreadPool.h"
#include <iostream>
#include <sstream>

void testThreadPoolAndLog()
{
    MyTinyRPC::ThreadPool &threadPool = MyTinyRPC::ThreadPool::getInstance();

    for (int i = 0; i < 100; i++)
    {
        threadPool.enqueue(
            [&]()
            {
                std::stringstream ss;
                ss << "Task " << std::this_thread::get_id() << " is running";
                LOG_ERROR(ss.str().c_str());
            });
    }
}

void testPBProtocol()
{
    // MyTinyRPC::PBProtocol pbProtocol;
    MyTinyRPC::PBProtocol::s_ptr pbProtocol = std::make_shared<MyTinyRPC::PBProtocol>();
    pbProtocol->m_msg_id = "1";
    pbProtocol->m_msg_err = "2";
    pbProtocol->m_msg_method_name = "3";
    pbProtocol->m_pb_data = "4";
    MyTinyRPC::TcpBuffer::s_ptr data = pbProtocol->encode();
    if (data == nullptr)
    {
        LOG_INFO("encode failed");
        return;
    }
    MyTinyRPC::TcpBuffer::s_ptr data_test = std::make_shared<MyTinyRPC::TcpBuffer>(100);
    data_test->moveWriteIndex(90);
    data_test->moveReadIndex(89);
    data_test->writeFromChar(data->getBuffer(), data->readAbleSize());

    MyTinyRPC::PBProtocol::s_ptr pbProtocol2 = std::make_shared<MyTinyRPC::PBProtocol>();
    bool status = pbProtocol2->decode(data_test);
    if (!status)
    {
        LOG_INFO("decode failed");
        return;
    }
    else
    {
        LOG_INFO(pbProtocol2->m_msg_id.c_str());
        LOG_INFO(pbProtocol2->m_msg_err.c_str());
        LOG_INFO(pbProtocol2->m_msg_method_name.c_str());
        LOG_INFO(pbProtocol2->m_pb_data.c_str());
    }
}

void testPBProtocol1()
{
    MyTinyRPC::TcpBuffer::s_ptr buffer = std::make_shared<MyTinyRPC::TcpBuffer>();

    MyTinyRPC::PBProtocol::s_ptr pbProtocol = std::make_shared<MyTinyRPC::PBProtocol>();
    pbProtocol->m_msg_id = "1";
    pbProtocol->m_msg_err = "2";
    pbProtocol->m_msg_method_name = "3";
    pbProtocol->m_pb_data = "4";
    MyTinyRPC::TcpBuffer::s_ptr data = pbProtocol->encode();
    for (int i = 0; i < 10000000; i++)
    {
        buffer->writeFromChar(data->getBuffer(), data->readAbleSize());
    }
    MyTinyRPC::PBProtocol::s_ptr pbProtocol2 = std::make_shared<MyTinyRPC::PBProtocol>();
    while (bool status = pbProtocol2->decode(buffer))
    {
        if (!status)
        {
            LOG_INFO("decode failed");
            return;
        }
        else
        {
            // LOG_INFO(pbProtocol2->m_msg_id.c_str());
            // LOG_INFO(pbProtocol2->m_msg_err.c_str());
            // LOG_INFO(pbProtocol2->m_msg_method_name.c_str());
            // LOG_INFO(pbProtocol2->m_pb_data.c_str());
        }
    }
    LOG_INFO("end, last buffer size: %d", buffer->readAbleSize());
}

void testStatus()
{
    MyTinyRPC::Status status(MyTinyRPC::Status::Code::OK, "status msg");
    if (status)
    {
        LOG_INFO("status msg: %s", status.getMessage().c_str());
    }
}

int main()
{
    // testThreadPoolAndLog();
    // testPBProtocol();
    testPBProtocol1();
    // testStatus();
    return 0;
}
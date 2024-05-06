#include "Log/Log.h"
#include "Rpc/RpcChannel.h"
#include "Rpc/RpcClosure.h"
#include "Rpc/RpcController.h"
#include "Tcp/TcpClient.h"
#include "ThreadPool/ThreadPool.h"
#include <stdio.h>

#include "Proto/hello.pb.h"

int main()
{
    MyTinyRPC::Log::getInstance(MyTinyRPC::SILENT);
    // MyTinyRPC::Log::getInstance(MyTinyRPC::INFO);
    MyTinyRPC::TcpClient::s_ptr client = MyTinyRPC::TcpClient::getInstance();
    client->Connect("127.0.0.1", 8080);
    // 计算运行时间
    int num = 100000;
    std::cout << "start num: " << num << std::endl;
    auto start = std::chrono::steady_clock::now();
    for (int i = 1; i <= num; i++)
    {
        MyTinyRPC::RpcChannel::s_ptr rpcChannel = std::make_shared<MyTinyRPC::RpcChannel>();
        MyTinyRPC::RpcController::s_ptr rpcController =
            std::make_shared<MyTinyRPC::RpcController>();
        rpcController->setId(std::to_string(i));

        // 对于一个新函数，只需要改动下面的代码就好了
        std::shared_ptr<HelloRequest> req = std::make_shared<HelloRequest>();
        req->set_name("hello " + std::to_string(i));
        // makeCulResponse res;
        std::shared_ptr<HelloResponse> res = std::make_shared<HelloResponse>();
        MyTinyRPC::RpcClosure::s_ptr rpcClosure = std::make_shared<MyTinyRPC::RpcClosure>(
            [rpcController, res]() mutable
            {
                // std::cout << "get result: " << res->message() << std::endl;
            });

        rpcChannel->init(rpcController, req, res, rpcClosure);
        HelloService_Stub(rpcChannel.get())
            .SayHello(rpcController.get(), req.get(), res.get(), rpcClosure.get());
    }
    // sleep 10 s
    // std::this_thread::sleep_for(std::chrono::seconds(10));
    auto end1 = std::chrono::steady_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start);
    std::cout << "time cost(req): " << duration1.count() << " us" << std::endl;
    std::cout << "qps(req): " << num * 1000.0 / duration1.count() << "k" << std::endl;
    client->Close();
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "time cost(all): " << duration.count() << " us" << std::endl;
    std::cout << "qps(all): " << num * 1000.0 / duration.count() << "k" << std::endl;
    return 0;
}
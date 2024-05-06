#include "Log/Log.h"
#include "Rpc/RpcDispatcher.h"
#include "Tcp/TcpServer.h"

#include "Proto/hello.pb.h"

namespace MyTinyRPC
{
    class HelloImpl : public HelloService
    {
        public:
            void SayHello(google::protobuf::RpcController *controller,
                              const ::HelloRequest *request,
                              ::HelloResponse *response,
                              ::google::protobuf::Closure *done)
            {
                response->set_message(request->name() + " world");
                if (done)
                {
                    done->Run();
                    delete done;
                    done = NULL;
                }
            }
    };
} // namespace MyTinyRPC

int main()
{
    MyTinyRPC::Log::getInstance(MyTinyRPC::SILENT);
    // MyTinyRPC::Log::getInstance(MyTinyRPC::INFO);
    MyTinyRPC::RpcDispatcher::getInstance()->registerService(
        std::make_shared<MyTinyRPC::HelloImpl>());

    MyTinyRPC::TcpServer::s_ptr tcpServer = MyTinyRPC::TcpServer::getInstance(8080);
    return 0;
}
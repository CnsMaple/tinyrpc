#include "Proto/hello.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <iostream>
#include <string>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class HelloClient
{
    public:
        HelloClient(std::shared_ptr<Channel> channel) : stub_(HelloService::NewStub(channel))
        {
        }

        // 异步调用 SayHello 方法
        std::string SayHello(const std::string &user)
        {
            HelloRequest request;
            HelloResponse response;
            ClientContext context;

            request.set_name(user);
            Status status = stub_->SayHello(&context, request, &response);

            if (status.ok())
            {
                return response.message();
            }
            else
            {
                std::cout << status.error_code() << ": " << status.error_message() << std::endl;
                return "RPC failed";
            }
        }

    private:
        std::unique_ptr<HelloService::Stub> stub_;
};

int main()
{
    HelloClient greeter(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
    std::string user("world");
    // 计算运行时间
    auto start = std::chrono::steady_clock::now();
    for (int i = 1; i <= 100000; i++)
    {
        greeter.SayHello(user);
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "time cost: "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " us"
              << std::endl;
    // std::string reply = greeter.SayHello(user);
    // std::cout << "Greeter received: " << reply << std::endl;
    return 0;
}

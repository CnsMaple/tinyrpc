#include <iostream>
#include <string>
#include <grpcpp/grpcpp.h>
#include "Proto/hello.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

// 实现 service 接口的基类
class HelloServiceImpl final : public HelloService::Service {
  Status SayHello(ServerContext* context, const HelloRequest* request,
                  HelloResponse* reply) override {
    std::string prefix("Hello ");
    reply->set_message(prefix + request->name());
    return Status::OK;
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  HelloServiceImpl service;

  ServerBuilder builder;
  // 监听指定的地址和端口
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // 注册 service 实现
  builder.RegisterService(&service);
  // 构建和启动服务器
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  // 等待服务器关闭
  server->Wait();
}

int main() {
  RunServer();
  return 0;
}

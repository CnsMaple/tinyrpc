cmake_minimum_required(VERSION 3.5)

project(MyTinyRPC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 把头文件加入库
include_directories(src)

# find_package(gRPC CONFIG REQUIRED)

# 设置项目可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

set(SRCPACKAGE
    src/Log/Log.h
    src/Log/Log.cpp

    src/ThreadPool/ThreadPool.h
    src/ThreadPool/ThreadPool.cpp

    src/Tcp/TcpServer.h
    src/Tcp/TcpServer.cpp

    src/Tcp/TcpClient.h
    src/Tcp/TcpClient.cpp

    src/Tcp/TcpBuffer.h
    src/Tcp/TcpBuffer.cpp

    src/Rpc/RpcChannel.h
    src/Rpc/RpcChannel.cpp

    src/Rpc/PBProtocol.h
    src/Rpc/PBProtocol.cpp

    src/Rpc/RpcDispatcher.h
    src/Rpc/RpcDispatcher.cpp

    src/Rpc/RpcController.h
    src/Rpc/RpcController.cpp

    src/Rpc/RpcClosure.h
    src/Rpc/RpcClosure.cpp

    src/Status/Status.h
    src/Status/Status.cpp

    src/Proto/hello.pb.cc
    src/Proto/hello.pb.h
)

set(ISGRPC False)

if (ISGRPC)
    add_executable(grpc_server
        src/grpc_server.cpp
        src/Proto/hello.pb.cc
        src/Proto/hello.pb.h
        src/Proto/hello.grpc.pb.cc
        src/Proto/hello.grpc.pb.h
    )
    target_link_libraries(grpc_server protobuf.a gRPC::grpc++)

    add_executable(grpc_client
        src/grpc_client.cpp
        src/Proto/hello.pb.cc
        src/Proto/hello.pb.h
        src/Proto/hello.grpc.pb.cc
        src/Proto/hello.grpc.pb.h
    )
    target_link_libraries(grpc_client protobuf.a gRPC::grpc++)
else()
    add_executable(TcpServer
        src/server.cpp
        ${SRCPACKAGE}
    )
    target_link_libraries(TcpServer protobuf.a)

    add_executable(TcpClient
        src/client.cpp
        ${SRCPACKAGE}
    )
    target_link_libraries(TcpClient protobuf.a)
endif()


add_executable(test
    test/test.cpp
    ${SRCPACKAGE}
)
target_link_libraries(test protobuf.a)

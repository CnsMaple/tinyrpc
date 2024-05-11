#ifndef RPCCLOSURE_H
#define RPCCLOSURE_H

#include <functional>
#include <google/protobuf/service.h>
#include <memory>

namespace MyTinyRPC
{
    class RpcClosure : public google::protobuf::Closure
    {
        public:
            typedef std::shared_ptr<RpcClosure> s_ptr;
            RpcClosure(std::function<void()> func);
            ~RpcClosure();

            void Run();

        private:
            std::function<void()> m_func;
    };
} // namespace MyTinyRPC

#endif // RPCCLOSURE_H
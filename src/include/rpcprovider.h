#pragma once
#include "google/protobuf/service.h"
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpConnection.h>
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>
#include "rpcheader.pb.h"


class RpcProvider {
public:
    void NotifyService(google::protobuf::Service *service);

    void Run();
private:
    muduo::net::EventLoop m_eventloop;
    struct ServiceInfo {
        google::protobuf::Service *m_service;
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap;
    };
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;

    void onConnection(const muduo::net::TcpConnectionPtr&);

    void onMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*, muduo::Timestamp);

    void SendRpcResponse(const muduo::net::TcpConnectionPtr&, google::protobuf::Message*);
};
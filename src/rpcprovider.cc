#include "rpcprovider.h"
#include "mprpcapplication.h"

void RpcProvider::NotifyService(google::protobuf::Service *service) {
    ServiceInfo service_info;

    // 获取了服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务的名字
    std::string service_name = pserviceDesc->name();
    // 获取服务对象service的方法的数量
    int methodCnt = pserviceDesc->method_count();

    std::cout << "service_name:" << service_name << std::endl;
    // LOG_INFO("service_name:%s", service_name.c_str());

    for (int i=0; i < methodCnt; ++i)
    {
        // 获取了服务对象指定下标的服务方法的描述（抽象描述） UserService   Login
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});

        // LOG_INFO("method_name:%s", method_name.c_str());
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}

void RpcProvider::Run() {
    std::string ip = MprpcApplication::getInstance().getConfig().load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::getInstance().getConfig().load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    muduo::net::TcpServer server(&m_eventloop, address, "RpcProvider");
    server.setConnectionCallback(std::bind(&RpcProvider::onConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    server.setThreadNum(4);
    std::cout << "RpcProvider start service at ip:" << ip << " port:" << port << std::endl;
    server.start();
    m_eventloop.loop();
}

void RpcProvider::onConnection(const muduo::net::TcpConnectionPtr& conn) {
    if (!conn->connected()) {
        conn->shutdown();
    }
}

/*
在框架内部，RpcProvider和RpcConsumer协商好之间通信用的protobuf数据类型
service_name method_name args    定义proto的message类型，进行数据头的序列化和反序列化
                                 service_name method_name args_size
16UserServiceLoginzhang san123456   

header_size(4个字节) + header_str + args_str
10 "10"
10000 "1000000"
std::string   insert和copy方法 
*/

void RpcProvider::onMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buffer, muduo::Timestamp) {
    std::string recv_buf = buffer->retrieveAllAsString();
    
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0);

    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpcHeader.ParseFromString(rpc_header_str)) {
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else {
        std::cout << "rpc_header_str:" << rpc_header_str << " parse error!" << std::endl;
        return ;
    }
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // 打印调试信息
    std::cout << "============================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl; 
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl; 
    std::cout << "service_name: " << service_name << std::endl; 
    std::cout << "method_name: " << method_name << std::endl; 
    std::cout << "args_str: " << args_str << std::endl; 
    std::cout << "============================================" << std::endl;

    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end()) {
        std::cout << service_name << " is not exist!" << std::endl;
        return ;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end()) {
            std::cout << service_name << ":" << method_name << " is not exist!" << std::endl;
            return ;
        }

    google::protobuf::Service *service = it->second.m_service;
    const google::protobuf::MethodDescriptor *method = mit->second;

    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str)) {
        std::cout << "request parse error, content:" << args_str << std::endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();
                                                        //因为类型推导有问题直接全例化
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider, 
                                                const muduo::net::TcpConnectionPtr&, 
                                                google::protobuf::Message*>(this, 
                                                &RpcProvider::SendRpcResponse,
                                                conn, response);

    service->CallMethod(method, nullptr, request, response, done);
}

void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response) {
    std::string response_str;
    if (response->SerializeToString(&response_str)) {
        conn->send(response_str);
    }
    else {
        std::cout << "serialize response_str error!" << std::endl; 
    }
    conn->shutdown();
}
#include <iostream>
#include "test.pb.h"

using namespace fixbug;
int main() {
    LoginRequest req;
    req.set_name("lxl");
    req.set_pwd("123456");
    std::string send_str;
    if (req.SerializeToString(&send_str)) {
        std::cout << send_str << std::endl;
    }
    std::cout << "=============" << std::endl;
    if (req.SerializeToString(&send_str)) {
        std::cout << send_str.c_str() << std::endl;
    }

    LoginRequest res;
    if (res.ParseFromString(send_str)) {
        std::cout << res.name() << std::endl;
        std::cout << res.pwd() << std::endl;
    }

    return 0;
}
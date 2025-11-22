#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>
#include <string>

MprpcConfig MprpcApplication::m_config;

void showArgHelp() {
    std::cout << "format: command -i <configfile>" << std::endl;
}

void MprpcApplication::Init(int argc, char **argv) {
    if (argc < 2) {
        showArgHelp();
        exit(EXIT_FAILURE);
    }

    int c = 0;
    std::string config_file;
    while ((c = getopt(argc, argv, "i:")) != -1) {
        switch (c) {
            case 'i':
                config_file = optarg;
                break;
            case '?':
                showArgHelp();
                exit(EXIT_FAILURE);
            case ':':
                showArgHelp();
                exit(EXIT_FAILURE);
            default:   
                break;
        }
    }
    m_config.loadConfigFile(config_file.c_str());
}

MprpcApplication &MprpcApplication::getInstance() {
    static MprpcApplication app;
    return app;
}

MprpcConfig& MprpcApplication::getConfig() {
    return m_config;
}

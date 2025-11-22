#include "mprpcconfig.h"
#include <iostream>
#include <fstream>
#include <cerrno>
#include <cstring>
// void MprpcConfig::loadConfigFile(const char *config_file) {
//     FILE* fp = fopen(config_file, "r");
//     if (fp == nullptr) {
//         std::cout << config_file << " is not exist!" << std::endl;
//         exit(EXIT_FAILURE);
//     }

//     while (!feof(fp)) {
//         char buf[512] = {0};
//         fgets(buf, 512, fp);

//         std::string read_buf(buf);
//         Trim(read_buf);

//         if (read_buf[0] == '#' || read_buf.empty()) {
//             continue;
//         }

//         int idx = read_buf.find('=');
//         if (idx == -1) {
//             continue;
//         }

//         std::string key;
//         std::string value;
//         key = read_buf.substr(0, idx);
//         Trim(key);
//         int endidx = read_buf.find('\n');
//         value = read_buf.substr(idx + 1, endidx - idx - 1);
//         Trim(value);
//         m_configMap.insert({key, value});
//     }
//     fclose(fp);
// }

void MprpcConfig::loadConfigFile(const char *config_file) {
    if (config_file == nullptr) {
        std::cerr << "config_file is null" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::ifstream ifs(config_file);
    if (!ifs.is_open()) {
        std::cerr << config_file << " is not exist! errno=" << errno
                  << " (" << std::strerror(errno) << ")" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string line;
    while (std::getline(ifs, line)) {
        Trim(line);
        if (line.empty() || line[0] == '#') continue;

        std::size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        Trim(key);
        Trim(value);
        if (!key.empty()) {
            m_configMap.emplace(std::move(key), std::move(value));
        }
    }
    // ifs 自动关闭
}

std::string MprpcConfig::load(const std::string &key) {
    auto it = m_configMap.find(key);
    if (it == m_configMap.end()) {
        return "";
    }
    return it->second;
}

void MprpcConfig::Trim(std::string &src_buf) {
    int idx = src_buf.find_first_not_of(' ');
    if (idx != -1) {
        src_buf = src_buf.substr(idx, src_buf.size() - idx);
    }
    idx = src_buf.find_last_not_of(' ');
    if (idx != -1) {
        src_buf = src_buf.substr(0, idx + 1);
    }
}

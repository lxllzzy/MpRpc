#pragma once

#include <unordered_map>
#include <string>
#include <iostream>

class MprpcConfig {
public:
    void loadConfigFile(const char *config_file);

    std::string load(const std::string &key);
private:
    std::unordered_map<std::string, std::string> m_configMap;
    void Trim(std::string &src_str);

};
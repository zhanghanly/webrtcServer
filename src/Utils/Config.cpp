#include <fstream>
#include <sstream>
#include <iostream>
#include "Config.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

std::string Config::Server::addr() {
    return ip + ":" + std::to_string(port);
}

std::string Config::Server::dump() {
    std::stringstream ss; 
    ss << "server::ip: " << ip << "\n"
       << "server::port: " << port << "\n";

    return ss.str();
}

void from_json(const json& j, Config::Server& server) {
    j.at("port").get_to(server.port);
    j.at("ip").get_to(server.ip);
}

std::string Config::Worker::dump() {
    std::stringstream ss;
    ss << "worker::workerId: " << workerId << "\n"
       << "worker::publicIp: " << publicIp << "\n"
       << "worker::publicPort: " << publicPort << "\n"
       << "worker::useUdp: " << useUdp << "\n";
    return ss.str();
}

void from_json(const json& j, Config::Worker& worker) {
    j.at("workerId").get_to(worker.workerId);
    j.at("publicIp").get_to(worker.publicIp);
    j.at("publicPort").get_to(worker.publicPort);
    j.at("useUdp").get_to(worker.useUdp);
}


std::string Config::Rtc::dump() {
    std::stringstream ss;
    ss << "rtc::maxNackWaitTime: " << maxNackWaitTime << "\n"
       << "rtc::maxNackRetries: " << maxNackRetries << "\n";
    return ss.str();
}

void from_json(const json& j, Config::Rtc& rtc) {
    j.at("maxNackWaitTime").get_to(rtc.maxNackWaitTime);
    j.at("maxNackRetries").get_to(rtc.maxNackRetries);
}

Config& Config::instance() {
    static Config config;
    return config;
}

void Config::parse(const std::string& file) {
    json jfile;
    std::ifstream ifile(file);
    if (!ifile.is_open()) {
        return;
    }

    ifile >> jfile;
    for (auto& iter : jfile.items()) {
        if (iter.key() == "server") {
            server = iter.value().get<Server>();
        
        } else if (iter.key() == "worker") {
            worker = iter.value().get<Worker>();
        
        } else if (iter.key() == "rtc") {
            rtc = iter.value().get<Rtc>();
        
        }
    }
    
    std::cout << server.dump() << worker.dump() << rtc.dump() << std::endl;
}
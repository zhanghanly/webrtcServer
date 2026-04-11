#ifndef MS_CONFIG_HPP
#define MS_CONFIG_HPP

#include <string>

class Config {
public:
    struct Server {
        std::string ip;
        int port;
   
        std::string addr();
        std::string dump();
    };

    struct Worker {
        std::string workerId;
        std::string publicIp;
        int publicPort;
        bool useUdp;

        std::string dump();
    };

    struct Rtc {
        int maxNackWaitTime;
        int maxNackRetries;

        std::string dump();
    };

    static Config& instance();

    void parse(const std::string& file = "../conf/config.json");

    Server ServerConf() {
        return server;
    }

    Worker WorkerConf() {
        return worker;
    }

    Rtc RtcConf() {
        return rtc;
    }

private:
    Config() = default;

    Server server;
    Worker worker;
    Rtc rtc;
};

#endif
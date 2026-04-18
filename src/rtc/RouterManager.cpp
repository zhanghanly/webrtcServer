#include "RouterManager.hpp"
#include "Config.hpp"

namespace RTC {

void RouterManager::createWebRtcServer(void) {
    if (!_webRtcServer) {
        std::string	protocol = "UDP";
        std::string ip = "0.0.0.0";
        uint32_t port = Config::instance().WorkerConf().publicPort;
        std::string announcedAddress = Config::instance().WorkerConf().publicIp;
        std::string announcedIp = Config::instance().WorkerConf().publicIp;
        uint32_t announcedPort = Config::instance().WorkerConf().publicPort;
        bool tcp = false;
        bool ipv6Only = false;
        bool udpReusePort = false;
        uint32_t recvBufferSize = 0;
        uint32_t sendBufferSize = 0;

        server::ListenInfo listenInfo;
        listenInfo.set_protocol(protocol);
        listenInfo.set_ip(ip);
        listenInfo.set_port(port);
        listenInfo.set_announced_address(announcedAddress);
        listenInfo.set_announced_ip(announcedIp);
        listenInfo.set_announced_port(announcedPort);
        listenInfo.set_tcp(tcp);
        listenInfo.set_ipv6only(ipv6Only);
        listenInfo.set_udpreuseport(udpReusePort);
        listenInfo.set_recv_buffer_size(recvBufferSize);
        listenInfo.set_send_buffer_size(sendBufferSize);

        _webRtcServer = new WebRtcServer(nullptr, "WebRtcServer", listenInfo);
    }
}

void RouterManager::createRouter(const std::string& routerId,
                                 const server::ListenInfo& info) {
    if (_routers.find(routerId) == _routers.end()) {
        _routers[routerId] = new Router(nullptr, routerId, _webRtcServer);
    }
}

void RouterManager::removeRouter(const std::string& routerId) {

}

server::CreateTransportResponse RouterManager::createTransport(const std::string& routerId, 
                                                               const std::string& transportId) {
    if (_routers.find(routerId) != _routers.end()) {
        std::cout << "create webrtc transport" << std::endl;
        return _routers[routerId]->createTransport(transportId);
    } 
    
    return server::CreateTransportResponse();
}
    

void RouterManager::connectTransport(const std::string& routerId, 
                                     const std::string& transportId,
                                     const std::string& role, 
                                     const server::DtlsFingerprint& fingerprint) {
    if (_routers.find(routerId) != _routers.end()) {
        std::cout << "connect webrtc transport" << std::endl;
        return _routers[routerId]->connectTransport(transportId, role, fingerprint);
    } 
}

void RouterManager::removeTransport(const std::string& transportId) {

}
    
void RouterManager::addTransportProducer(const std::string& routerId,
                                         const std::string& transportId,
                                         const std::string& producerId,
                                         const std::string& kind,
                                         const server::RtpParameters& rtp_params) {
    if (_routers.find(routerId) != _routers.end()) {
        _routers[routerId]->addProducer(transportId, producerId, kind, rtp_params);
    }
}

void RouterManager::closeTransportProducer(const std::string& routerId,
                                           const std::string& transportId,
                                           const std::string& producerId,
                                           const std::string& kind) {
    if (_routers.find(routerId) != _routers.end()) {
        _routers[routerId]->closeProducer(transportId, producerId, kind);
    }
}

void RouterManager::addTransportConsumer(const std::string& routerId,
                                         const std::string& transportId,
                                         const std::string& producerId,
                                         const std::string& consumerId,
                                         const std::string& kind,
                                         const server::RtpParameters& rtp_params) {
    if (_routers.find(routerId) != _routers.end()) {
        _routers[routerId]->addConsumer(transportId, producerId, consumerId, kind, rtp_params);
    }
}
    
void RouterManager::closeTransportConsumer(const std::string& routerId,
                                           const std::string& transportId,
                                           const std::string& producerId,
                                           const std::string& consumerId,
                                           const std::string& kind) {
    if (_routers.find(routerId) != _routers.end()) {
        _routers[routerId]->closeConsumer(transportId, producerId, consumerId, kind);
    }
}

}
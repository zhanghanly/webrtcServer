#include <iostream>
#include <csignal>
#include <chrono>
#include <thread>
#include "grpc/workerClient.h"
#include "rtc/RouterManager.hpp"
#include "DepLibUV.hpp"
#include "DepUsrSCTP.hpp"
#include "DepOpenSSL.hpp"
#include "DepLibSRTP.hpp"
#include "DepLibWebRTC.hpp"
#include "Config.hpp"

int main(int argc, char* argv[]) {
    DepLibUV::ClassInit();
    DepLibUV::PrintVersion();
    DepOpenSSL::ClassInit();
	DepLibSRTP::ClassInit();
    DepUsrSCTP::CreateChecker();
	DepUsrSCTP::ClassInit();
#ifdef MS_LIBURING_SUPPORTED
	DepLibUring::ClassInit();
#endif
	DepLibWebRTC::ClassInit();
	Utils::Crypto::ClassInit();
	RTC::DtlsTransport::ClassInit();
	RTC::SrtpSession::ClassInit();
   
    Config::instance().parse();
    RTC::RouterManager manager; 
    std::string server_address = Config::instance().ServerConf().addr();
    std::string worker_id = Config::instance().WorkerConf().workerId;
    worker::WorkerClient client(server_address, worker_id);
    
    client.SetCreateRouterHandler([&worker_id, &manager](const std::string& req_worker_id,
                                                         const std::string& room_id,
                                                         const std::string& serverId,
                                                         const server::ListenInfo& info) {
        std::cout << "[CreateRouterHandler] Worker: " << req_worker_id 
                  << ", Room: " << room_id << std::endl;
       
        std::string router_id = room_id;
        manager.createRouter(router_id, info);
        
        return worker::ResponseBuilder::BuildCreateRouterResponse(router_id, true);
    });
    
    client.SetCreateTransportHandler([&worker_id, &manager](const std::string& req_worker_id,
                                                            const std::string& router_id,
                                                            const std::string& transport_id,
                                                            server::TransportDirection direction,
                                                            bool enable_udp,
                                                            bool enable_tcp,
                                                            bool prefer_udp,
                                                            bool prefer_tcp) {
        std::cout << "[CreateTransportHandler] Router: " << router_id 
                  << ", Transport: " << transport_id << std::endl;
        return manager.createTransport(router_id, transport_id);
    });
   
    client.SetConnectTransportHandler([&manager](const std::string& worker_id,
                                        const std::string& router_id,
                                        const std::string& transport_id,
                                        const std::string& dtls_role,
                                        const std::vector<server::DtlsFingerprint>& fingerprints) {
        std::cout << "[ConnectTransportHandler] Transport: " << transport_id << std::endl;
        manager.connectTransport(router_id, transport_id, dtls_role, fingerprints[0]);
        return worker::ResponseBuilder::BuildConnectTransportResponse(transport_id, true);
    });
    
    client.SetProduceHandler([&manager](
                               const std::string& router_id,
                               const std::string& transport_id,
                               const std::string& producer_id,
                               const std::string& kind,
                               const std::string& app_data,
                               const std::string& method,
                               const server::RtpParameters& rtp_params) {
        std::cout << "[ProduceHandler] Transport: " << transport_id 
                  << ", Kind: " << kind
                  << ", method: " << method << std::endl;
        if (method == "create") {
            manager.addTransportProducer(router_id, transport_id, producer_id, kind, rtp_params);
        
        } else if (method == "close") {
            manager.closeTransportProducer(router_id, transport_id, producer_id, kind);
        }
        
        return worker::ResponseBuilder::BuildProduceResponse(producer_id, true);
    });
    
    client.SetConsumeHandler([&manager](
                               const std::string& router_id,
                               const std::string& transport_id,
                               const std::string& producer_id,
                               const std::string& consumer_id,
                               const std::string& kind,
                               const std::string& app_data,
                               const std::string& method,
                               const server::RtpParameters& rtp_params) {
        std::cout << "[ConsumeHandler] Transport: " << transport_id 
                  << ", Producer: " << producer_id 
                  << ", Consumer: " << consumer_id
                  << ", kind: " << kind 
                  << ", method: " << method << std::endl;
        if (method == "create") {
            manager.addTransportConsumer(router_id, transport_id, producer_id, consumer_id, kind, rtp_params);
        
        } else if (method == "close") {
            manager.closeTransportConsumer(router_id, transport_id, producer_id, consumer_id, kind);
        }
        
        return worker::ResponseBuilder::BuildConsumeResponse(
            consumer_id, producer_id, server::MEDIA_KIND_VIDEO, true);
    });
    
    client.SetGlobalNotificationHandler([](const std::string& notification) {
        std::cout << "[GlobalNotification] " << notification << std::endl;
    });
    
    // connect grpc server
    if (!client.Connect()) {
        std::cerr << "Failed to connect to server. Exiting." << std::endl;
        return 1;
    }
    client.SendWorkerRegister();
        
    manager.createWebRtcServer();
    DepLibUV::RunLoop();
        
    return 0;
}
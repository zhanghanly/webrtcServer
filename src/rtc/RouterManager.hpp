#ifndef MS_ROUTER_MANAGER_HPP
#define MS_ROUTER_MANAGER_HPP

#include <map>
#include "Router.hpp"
#include "workerClient.h"

namespace RTC {

class RouterManager {
public:
    void createWebRtcServer(void);    

    void createRouter(const std::string& routerId, 
                      const server::ListenInfo& info); 
    
    void removeRouter(const std::string& routerId);
    
    server::CreateTransportResponse createTransport(const std::string& routerId, 
                                                    const std::string& transportId);
    
    void connectTransport(const std::string& routerId, const std::string& transportId,
                          const std::string& role, const server::DtlsFingerprint&);
    
    void removeTransport(const std::string& transportId);
    
    void addTransportProducer(const std::string& routerId,
                              const std::string& transportId,
                              const std::string& producerId,
                              const std::string& kind,
                              const server::RtpParameters& rtp_params);

    void closeTransportProducer(const std::string& transportId);
    
    void addTransportConsumer(const std::string& routerId,
                              const std::string& transportId,
                              const std::string& producerId,
                              const std::string& consumerId,
                              const std::string& kind,
                              const server::RtpParameters& rtp_params);
                            
    void closeTransportConsumer(const std::string& transportId);

private:
    WebRtcServer* _webRtcServer{nullptr};
    std::map<std::string, Router*> _routers;
};

}

#endif
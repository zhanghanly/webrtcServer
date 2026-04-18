// worker_client.h
#ifndef WORKER_CLIENT_H
#define WORKER_CLIENT_H

#include <string>
#include <memory>
#include <functional>
#include <map>
#include <vector>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <grpcpp/grpcpp.h>
#include "singal.pb.h"
#include "singal.grpc.pb.h"

namespace worker {

using CreateRouterHandler = std::function<server::CreateRouterResponse(
    const std::string& worker_id,
    const std::string& room_id,
    const std::string& serverId,
    const server::ListenInfo& info)>;

using CreateTransportHandler = std::function<server::CreateTransportResponse(
    const std::string& worker_id,
    const std::string& router_id,
    const std::string& transport_id,
    server::TransportDirection direction,
    bool enable_udp,
    bool enable_tcp,
    bool prefer_udp,
    bool prefer_tcp)>;

using ConnectTransportHandler = std::function<server::ConnectTransportResponse(
    const std::string& worker_id,
    const std::string& router_id,
    const std::string& transport_id,
    const std::string& dtls_role,
    const std::vector<server::DtlsFingerprint>& dtls_fingerprints)>;

using ProduceHandler = std::function<server::ProduceResponse(
    const std::string& router_id,
    const std::string& transport_id,
    const std::string& producer_id,
    const std::string& kind,
    const std::string& app_data,
    const std::string& method,
    const server::RtpParameters& rtp_parameters)>;

using ConsumeHandler = std::function<server::ConsumeResponse(
    const std::string& router_id,
    const std::string& transport_id,
    const std::string& producer_id,
    const std::string& consumer_id,
    const std::string& kind,
    const std::string& app_data,
    const std::string& method,
    const server::RtpParameters& rtp_parameters)>;

using GlobalNotificationHandler = std::function<void(const std::string& notification)>;

class WorkerClient {
public:
    explicit WorkerClient(const std::string& server_address, const std::string& worker_id);
    ~WorkerClient();

    bool Connect();
    void Disconnect();
    bool IsConnected() const;

    void SetCreateRouterHandler(CreateRouterHandler handler);
    void SetCreateTransportHandler(CreateTransportHandler handler);
    void SetConnectTransportHandler(ConnectTransportHandler handler);
    void SetProduceHandler(ProduceHandler handler);
    void SetConsumeHandler(ConsumeHandler handler);
    void SetGlobalNotificationHandler(GlobalNotificationHandler handler);

    void SendStats(const std::string& stats_json);
    
    void SendWorkerRegister();
    void SendWorkerKeepalive();
    
    std::string GetWorkerId() const { return worker_id_; }

private:
    std::unique_ptr<server::WebRtcService::Stub> stub_;
    std::shared_ptr<grpc::Channel> channel_;
    grpc::ClientContext context_;
    
    std::unique_ptr<grpc::ClientReaderWriter<server::WorkerToServer, 
                                            server::ServerToWorker>> stream_;
    
    const std::string worker_id_;
    std::atomic<bool> running_{false};
    std::thread read_thread_;
    std::thread write_thread_;
    
    CreateRouterHandler create_router_handler_;
    CreateTransportHandler create_transport_handler_;
    ConnectTransportHandler connect_transport_handler_;
    ProduceHandler produce_handler_;
    ConsumeHandler consume_handler_;
    GlobalNotificationHandler global_notification_handler_;
    
    struct PendingResponse {
        uint64_t seq_id;
        server::WorkerToServer response;
    };
    
    std::mutex response_mutex_;
    std::vector<PendingResponse> pending_responses_;
    std::condition_variable response_cv_;
    
    void StartStreaming();
    void StopStreaming();
    void ReadLoop();
    void WriteLoop();
    void ProcessRequest(const server::ServerToWorker& request);
    
    server::WorkerToServer BuildResponse(uint64_t seq_id);
    
    server::CreateRouterResponse HandleCreateRouter(const server::CreateRouterRequest& req);
    server::CreateTransportResponse HandleCreateTransport(const server::CreateTransportRequest& req);
    server::ConnectTransportResponse HandleConnectTransport(const server::ConnectTransportRequest& req);
    server::ProduceResponse HandleProduce(const server::ProduceRequest& req);
    server::ConsumeResponse HandleConsume(const server::ConsumeRequest& req);
    
    void QueueResponse(uint64_t seq_id, const server::WorkerToServer& response);
};

class ResponseBuilder {
public:
    static server::CreateRouterResponse BuildCreateRouterResponse(
        const std::string& router_id,
        bool success = true,
        const std::string& error_detail = "");
    
    static server::CreateTransportResponse BuildCreateTransportResponse(
        const std::string& transport_id,
        const std::string& ice_ufrag,
        const std::string& ice_pwd,
        const std::vector<server::IceCandidate>& ice_candidates,
        const std::vector<server::DtlsFingerprint>& dtls_fingerprints,
        const std::string& dtls_role = "auto",
        bool success = true,
        const std::string& error_detail = "");
    
    static server::ConnectTransportResponse BuildConnectTransportResponse(
        const std::string& transport_id,
        bool success = true,
        const std::string& error_detail = "");
    
    static server::ProduceResponse BuildProduceResponse(
        const std::string& producer_id,
        bool success = true,
        const std::string& error_detail = "");
    
    static server::ConsumeResponse BuildConsumeResponse(
        const std::string& consumer_id,
        const std::string& producer_id,
        server::MediaKind kind,
        bool success = true,
        const std::string& error_detail = "");
    
    static server::IceCandidate BuildIceCandidate(
        const std::string& foundation,
        uint32_t priority,
        const std::string& ip,
        const std::string& protocol,
        uint32_t port,
        const std::string& type = "host",
        const std::string& tcp_type = "");
    
    static server::DtlsFingerprint BuildDtlsFingerprint(
        const std::string& algorithm = "sha-256",
        const std::string& value = "");
};

} // namespace worker

#endif // WORKER_CLIENT_H
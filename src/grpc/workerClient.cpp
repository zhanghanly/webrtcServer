#include <iostream>
#include <chrono>
#include <random>
#include "Config.hpp"
#include "workerClient.h"

namespace worker {

WorkerClient::WorkerClient(const std::string& server_address, const std::string& worker_id)
    : worker_id_(worker_id) {
    channel_ = grpc::CreateChannel(server_address, 
                                  grpc::InsecureChannelCredentials());
    stub_ = server::WebRtcService::NewStub(channel_);
    
    std::cout << "Worker client created. Worker ID: " << worker_id_ 
              << ", Server: " << server_address << std::endl;
}

WorkerClient::~WorkerClient() {
    Disconnect();
}

bool WorkerClient::Connect() {
    if (IsConnected()) {
        std::cout << "Already connected" << std::endl;
        return true;
    }

    try {
        context_.set_wait_for_ready(true);
        //context_.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(100));
        
        stream_ = stub_->Sync(&context_);
        
        if (!stream_) {
            std::cerr << "Failed to create gRPC stream" << std::endl;
            return false;
        }

        running_ = true;
        
        read_thread_ = std::thread(&WorkerClient::ReadLoop, this);
        write_thread_ = std::thread(&WorkerClient::WriteLoop, this);
        
        std::cout << "Connected to server successfully" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Connection failed: " << e.what() << std::endl;
        return false;
    }
}

void WorkerClient::Disconnect() {
    if (!running_) return;
    
    std::cout << "Disconnecting from server..." << std::endl;
    running_ = false;
    
    {
        std::lock_guard<std::mutex> lock(response_mutex_);
        response_cv_.notify_all();
    }
    
    if (stream_) {
        stream_->WritesDone();
    }
    
    if (read_thread_.joinable()) {
        read_thread_.join();
    }
    
    if (write_thread_.joinable()) {
        write_thread_.join();
    }
    
    stream_.reset();
    std::cout << "Disconnected" << std::endl;
}

bool WorkerClient::IsConnected() const {
    return running_ && stream_ != nullptr;
}

void WorkerClient::SetCreateRouterHandler(CreateRouterHandler handler) {
    create_router_handler_ = std::move(handler);
}

void WorkerClient::SetCreateTransportHandler(CreateTransportHandler handler) {
    create_transport_handler_ = std::move(handler);
}

void WorkerClient::SetConnectTransportHandler(ConnectTransportHandler handler) {
    connect_transport_handler_ = std::move(handler);
}

void WorkerClient::SetProduceHandler(ProduceHandler handler) {
    produce_handler_ = std::move(handler);
}

void WorkerClient::SetConsumeHandler(ConsumeHandler handler) {
    consume_handler_ = std::move(handler);
}

void WorkerClient::SetGlobalNotificationHandler(GlobalNotificationHandler handler) {
    global_notification_handler_ = std::move(handler);
}

void WorkerClient::SendStats(const std::string& stats_json) {
    if (!IsConnected()) {
        std::cerr << "Not connected, cannot send stats" << std::endl;
        return;
    }

    server::WorkerToServer response;
    response.set_seq_id(0); 
    response.set_stats_push(stats_json);
    
    QueueResponse(0, response);
}

void WorkerClient::SendWorkerRegister() {
    if (!IsConnected()) {
        std::cerr << "Not connected, cannot send worker register" << std::endl;
        return;
    }

    server::WorkerToServer response;
    response.set_seq_id(0);
    
    auto* register_req = response.mutable_worker_register();
    register_req->set_worker_id(worker_id_);
    register_req->set_cpu_usage(0);
    register_req->set_memory_usage(0);
    register_req->set_router_count(0);
    register_req->set_use_udp(Config::instance().WorkerConf().useUdp);
    register_req->set_public_ip(Config::instance().WorkerConf().publicIp);
    register_req->set_public_port(Config::instance().WorkerConf().publicPort);
    
    QueueResponse(0, response);
    std::cout << "Sent worker register for worker: " << worker_id_ << std::endl;
}

void WorkerClient::SendWorkerKeepalive() {
    if (!IsConnected()) {
        std::cerr << "Not connected, cannot send worker keepalive" << std::endl;
        return;
    }

    server::WorkerToServer response;
    response.set_seq_id(0);
    
    auto* keepalive_req = response.mutable_worker_keepalive();
    keepalive_req->set_worker_id(worker_id_);

    QueueResponse(0, response);
}

void WorkerClient::ReadLoop() {
    server::ServerToWorker request;
    
    while (running_) {
        try {
            if (stream_ && stream_->Read(&request)) {
                ProcessRequest(request);
            } else {
                if (running_) {
                    std::cerr << "Stream read failed or server disconnected" << std::endl;
                }
                running_ = false;
                break;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error in read loop: " << e.what() << std::endl;
            if (running_) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }
}

void WorkerClient::WriteLoop() {
    while (running_) {
        std::vector<PendingResponse> responses_to_send;
        
        {
            std::unique_lock<std::mutex> lock(response_mutex_);
            response_cv_.wait_for(lock, std::chrono::milliseconds(100), 
                [this]() { return !pending_responses_.empty() || !running_; });
            
            if (!running_) break;
            
            if (!pending_responses_.empty()) {
                responses_to_send.swap(pending_responses_);
            }
        }
        
        for (const auto& pending : responses_to_send) {
            if (!IsConnected()) break;
            
            try {
                if (!stream_->Write(pending.response)) {
                    std::cerr << "Failed to write response to stream" << std::endl;
                    running_ = false;
                    break;
                }
                
                std::cout << "Sent response for seq_id: " << pending.seq_id << std::endl;
                
            } catch (const std::exception& e) {
                std::cerr << "Write failed: " << e.what() << std::endl;
                running_ = false;
                break;
            }
        }
        
        static auto last_stats_time = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        
        if (now - last_stats_time > std::chrono::seconds(30)) {
            SendWorkerKeepalive();
            last_stats_time = now;
            std::cout << "Sent periodic keepalive" << std::endl;
        }
    }
}

void WorkerClient::ProcessRequest(const server::ServerToWorker& request) {
    std::cout << "Received request, seq_id: " << request.seq_id() 
              << ", has_payload: " << request.payload_case() << std::endl;
    
    server::WorkerToServer response = BuildResponse(request.seq_id());
    
    switch (request.payload_case()) {
        case server::ServerToWorker::kCreateRouterReq: {
            const auto& req = request.create_router_req();
            auto res = HandleCreateRouter(req);
            response.mutable_create_router_res()->CopyFrom(res);
            break;
        }
            
        case server::ServerToWorker::kCreateTransportReq: {
            const auto& req = request.create_transport_req();
            auto res = HandleCreateTransport(req);
            response.mutable_create_transport_res()->CopyFrom(res);
            break;
        }
            
        case server::ServerToWorker::kConnectTransportReq: {
            const auto& req = request.connect_transport_req();
            auto res = HandleConnectTransport(req);
            response.mutable_connect_transport_res()->CopyFrom(res);
            break;
        }
            
        case server::ServerToWorker::kProduceReq: {
            const auto& req = request.produce_req();
            auto res = HandleProduce(req);
            response.mutable_produce_res()->CopyFrom(res);
            break;
        }
            
        case server::ServerToWorker::kConsumerReq: {
            const auto& req = request.consumer_req();
            auto res = HandleConsume(req);
            response.mutable_consumer_res()->CopyFrom(res);
            break;
        }
            
        case server::ServerToWorker::kGlobalNotification: {
            if (global_notification_handler_) {
                global_notification_handler_(request.global_notification());
            }
            return;
        }
            
        default:
            std::cerr << "Unknown request type: " << request.payload_case() << std::endl;
            return;
    }
    
    QueueResponse(request.seq_id(), response);
}

server::WorkerToServer WorkerClient::BuildResponse(uint64_t seq_id) {
    server::WorkerToServer response;
    response.set_seq_id(seq_id);
    return response;
}

server::CreateRouterResponse WorkerClient::HandleCreateRouter(const server::CreateRouterRequest& req) {
    std::cout << "Handling CreateRouter request. Room: " << req.room_id() 
              << ", ServerId: " << req.serverid() << std::endl;
    
    if (create_router_handler_) {
        return create_router_handler_(
            req.worker_id(),
            req.room_id(),
            req.serverid(),
            req.info());
    }
    
    return ResponseBuilder::BuildCreateRouterResponse(
        "router-" + req.room_id() + "-" + std::to_string(std::rand()),
        true);
}

server::CreateTransportResponse WorkerClient::HandleCreateTransport(const server::CreateTransportRequest& req) {
    std::cout << "Handling CreateTransport request. Router: " << req.router_id()
              << ", Transport: " << req.transport_id() << std::endl;
    
    if (create_transport_handler_) {
        return create_transport_handler_(
            req.worker_id(),
            req.router_id(),
            req.transport_id(),
            req.direction(),
            req.enable_udp(),
            req.enable_tcp(),
            req.prefer_udp(),
            req.prefer_tcp());
    }
    
    std::vector<server::IceCandidate> candidates;
    candidates.push_back(ResponseBuilder::BuildIceCandidate(
        "foundation1", 123456, "192.168.1.100", "udp", 40000));
    
    std::vector<server::DtlsFingerprint> fingerprints;
    fingerprints.push_back(ResponseBuilder::BuildDtlsFingerprint(
        "sha-256", "test-fingerprint-value"));
    
    return ResponseBuilder::BuildCreateTransportResponse(
        req.transport_id(),
        "ice-ufrag-" + req.transport_id(),
        "ice-pwd-" + req.transport_id(),
        candidates,
        fingerprints,
        "auto",
        true);
}

server::ConnectTransportResponse WorkerClient::HandleConnectTransport(const server::ConnectTransportRequest& req) {
    std::cout << "Handling ConnectTransport request. Transport: " << req.transport_id() << std::endl;
    
    if (connect_transport_handler_) {
        std::vector<server::DtlsFingerprint> fingerprints;
        for (const auto& fp : req.dtls_fingerprints()) {
            fingerprints.push_back(fp);
        }
        
        return connect_transport_handler_(
            req.worker_id(),
            req.router_id(),
            req.transport_id(),
            req.dtls_role(),
            fingerprints);
    }
    
    return ResponseBuilder::BuildConnectTransportResponse(
        req.transport_id(),
        true);
}

server::ProduceResponse WorkerClient::HandleProduce(const server::ProduceRequest& req) {
    if (produce_handler_) {
        return produce_handler_(
            req.router_id(),
            req.transport_id(),
            req.producer_id(),
            req.kind(),
            req.app_data(),
            req.method(),
            req.rtpparameters());
    }
    
    return ResponseBuilder::BuildProduceResponse(
        "producer-" + req.transport_id() + "-" + std::to_string(std::rand()),
        true);
}

server::ConsumeResponse WorkerClient::HandleConsume(const server::ConsumeRequest& req) {
    std::cout << "Handling Consume request. Transport: " << req.transport_id()
              << ", Producer: " << req.producer_id() << std::endl;
    
    if (consume_handler_) {
        return consume_handler_(
            req.router_id(),
            req.transport_id(),
            req.producer_id(),
            req.consumer_id(),
            req.kind(),
            req.app_data(),
            req.method(),
            req.rtpparameters());
    }
    
    return ResponseBuilder::BuildConsumeResponse(
        "consumer-" + req.producer_id() + "-" + std::to_string(std::rand()),
        req.producer_id(),
        server::MEDIA_KIND_VIDEO,
        true);
}

void WorkerClient::QueueResponse(uint64_t seq_id, const server::WorkerToServer& response) {
    std::lock_guard<std::mutex> lock(response_mutex_);
    pending_responses_.push_back({seq_id, response});
    response_cv_.notify_one();
}

server::CreateRouterResponse ResponseBuilder::BuildCreateRouterResponse(
    const std::string& router_id,
    bool success,
    const std::string& error_detail) {
    
    server::CreateRouterResponse response;
    response.set_router_id(router_id);
    response.set_success(success);
    if (!error_detail.empty()) {
        response.set_error_detail(error_detail);
    }
    return response;
}

server::CreateTransportResponse ResponseBuilder::BuildCreateTransportResponse(
    const std::string& transport_id,
    const std::string& ice_ufrag,
    const std::string& ice_pwd,
    const std::vector<server::IceCandidate>& ice_candidates,
    const std::vector<server::DtlsFingerprint>& dtls_fingerprints,
    const std::string& dtls_role,
    bool success,
    const std::string& error_detail) {
    
    server::CreateTransportResponse response;
    response.set_transport_id(transport_id);
    response.set_ice_ufrag(ice_ufrag);
    response.set_ice_pwd(ice_pwd);
    response.set_dtls_role(dtls_role);
    response.set_success(success);
    
    for (const auto& candidate : ice_candidates) {
        *response.add_ice_candidates() = candidate;
    }
    
    for (const auto& fingerprint : dtls_fingerprints) {
        *response.add_dtls_fingerprints() = fingerprint;
    }
    
    if (!error_detail.empty()) {
        response.set_error_detail(error_detail);
    }
    
    return response;
}

server::ConnectTransportResponse ResponseBuilder::BuildConnectTransportResponse(
    const std::string& transport_id,
    bool success,
    const std::string& error_detail) {
    
    server::ConnectTransportResponse response;
    response.set_transport_id(transport_id);
    response.set_success(success);
    if (!error_detail.empty()) {
        response.set_error_detail(error_detail);
    }
    return response;
}

server::ProduceResponse ResponseBuilder::BuildProduceResponse(
    const std::string& producer_id,
    bool success,
    const std::string& error_detail) {
    
    server::ProduceResponse response;
    response.set_producer_id(producer_id);
    response.set_success(success);
    if (!error_detail.empty()) {
        response.set_error_detail(error_detail);
    }
    return response;
}

server::ConsumeResponse ResponseBuilder::BuildConsumeResponse(
    const std::string& consumer_id,
    const std::string& producer_id,
    server::MediaKind kind,
    bool success,
    const std::string& error_detail) {
    
    server::ConsumeResponse response;
    response.set_consumer_id(consumer_id);
    response.set_producer_id(producer_id);
    response.set_kind(kind);
    response.set_success(success);
    if (!error_detail.empty()) {
        response.set_error_detail(error_detail);
    }
    return response;
}

server::IceCandidate ResponseBuilder::BuildIceCandidate(
    const std::string& foundation,
    uint32_t priority,
    const std::string& ip,
    const std::string& protocol,
    uint32_t port,
    const std::string& type,
    const std::string& tcp_type) {
    
    server::IceCandidate candidate;
    candidate.set_foundation(foundation);
    candidate.set_priority(priority);
    candidate.set_ip(ip);
    candidate.set_protocol(protocol);
    candidate.set_port(port);
    candidate.set_type(type);
    
    if (!tcp_type.empty()) {
        candidate.set_tcp_type(tcp_type);
    }
    
    return candidate;
}

server::DtlsFingerprint ResponseBuilder::BuildDtlsFingerprint(
    const std::string& algorithm,
    const std::string& value) {
    
    server::DtlsFingerprint fingerprint;
    fingerprint.set_algorithm(algorithm);
    fingerprint.set_value(value);
    return fingerprint;
}

} // namespace worker
#ifndef MS_RTC_ROUTER_HPP
#define MS_RTC_ROUTER_HPP

#include "common.hpp"
#include "Channel/ChannelRequest.hpp"
#include "rtc/Consumer.hpp"
#include "rtc/DataConsumer.hpp"
#include "rtc/DataProducer.hpp"
#include "rtc/Producer.hpp"
#include "rtc/RtpObserver.hpp"
#include "rtc/RtpPacket.hpp"
#include "rtc/RtpStreamRecv.hpp"
#include "rtc/Shared.hpp"
#include "rtc/Transport.hpp"
#include "rtc/WebRtcServer.hpp"
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include "workerClient.h"
#include <string>
#include <vector>

namespace RTC
{
	class Router : public RTC::Transport::Listener,
	               public RTC::RtpObserver::Listener,
	               public Channel::ChannelSocket::RequestHandler
	{
	public:
		explicit Router(RTC::Shared* shared, const std::string& id, WebRtcServer* server);
		~Router() override;

	public:
		flatbuffers::Offset<FBS::Router::DumpResponse> FillBuffer(
		  flatbuffers::FlatBufferBuilder& builder) const;

		/* Methods inherited from Channel::ChannelSocket::RequestHandler. */
	public:
		void HandleRequest(Channel::ChannelRequest* request) override;

	private:
		RTC::Transport* GetTransportById(const std::string& transportId) const;
		RTC::RtpObserver* GetRtpObserverById(const std::string& rtpObserverId) const;
		void CheckNoTransport(const std::string& transportId) const;
		void CheckNoRtpObserver(const std::string& rtpObserverId) const;

		/* Pure virtual methods inherited from RTC::Transport::Listener. */
	public:
		void OnTransportNewProducer(RTC::Transport* transport, RTC::Producer* producer) override;
		void OnTransportProducerClosed(RTC::Transport* transport, RTC::Producer* producer) override;
		void OnTransportProducerPaused(RTC::Transport* transport, RTC::Producer* producer) override;
		void OnTransportProducerResumed(RTC::Transport* transport, RTC::Producer* producer) override;
		void OnTransportProducerNewRtpStream(
		  RTC::Transport* transport,
		  RTC::Producer* producer,
		  RTC::RtpStreamRecv* rtpStream,
		  uint32_t mappedSsrc) override;
		void OnTransportProducerRtpStreamScore(
		  RTC::Transport* transport,
		  RTC::Producer* producer,
		  RTC::RtpStreamRecv* rtpStream,
		  uint8_t score,
		  uint8_t previousScore) override;
		void OnTransportProducerRtcpSenderReport(
		  RTC::Transport* transport,
		  RTC::Producer* producer,
		  RTC::RtpStreamRecv* rtpStream,
		  bool first) override;
		void OnTransportProducerRtpPacketReceived(
		  RTC::Transport* transport, RTC::Producer* producer, RTC::RtpPacket* packet) override;
		void OnTransportNeedWorstRemoteFractionLost(
		  RTC::Transport* transport,
		  RTC::Producer* producer,
		  uint32_t mappedSsrc,
		  uint8_t& worstRemoteFractionLost) override;
		void OnTransportNewConsumer(
		  RTC::Transport* transport, RTC::Consumer* consumer, const std::string& producerId) override;
		void OnTransportConsumerClosed(RTC::Transport* transport, RTC::Consumer* consumer) override;
		void OnTransportConsumerProducerClosed(RTC::Transport* transport, RTC::Consumer* consumer) override;
		void OnTransportConsumerKeyFrameRequested(
		  RTC::Transport* transport, RTC::Consumer* consumer, uint32_t mappedSsrc) override;
		void OnTransportNewDataProducer(RTC::Transport* transport, RTC::DataProducer* dataProducer) override;
		void OnTransportDataProducerClosed(RTC::Transport* transport, RTC::DataProducer* dataProducer) override;
		void OnTransportDataProducerPaused(RTC::Transport* transport, RTC::DataProducer* dataProducer) override;
		void OnTransportDataProducerResumed(
		  RTC::Transport* transport, RTC::DataProducer* dataProducer) override;
		void OnTransportDataProducerMessageReceived(
		  RTC::Transport* transport,
		  RTC::DataProducer* dataProducer,
		  const uint8_t* msg,
		  size_t len,
		  uint32_t ppid,
		  std::vector<uint16_t>& subchannels,
		  std::optional<uint16_t> requiredSubchannel) override;
		void OnTransportNewDataConsumer(
		  RTC::Transport* transport, RTC::DataConsumer* dataConsumer, std::string& dataProducerId) override;
		void OnTransportDataConsumerClosed(RTC::Transport* transport, RTC::DataConsumer* dataConsumer) override;
		void OnTransportDataConsumerDataProducerClosed(
		  RTC::Transport* transport, RTC::DataConsumer* dataConsumer) override;
		void OnTransportListenServerClosed(RTC::Transport* transport) override;

		/* Pure virtual methods inherited from RTC::RtpObserver::Listener. */
	public:
		RTC::Producer* RtpObserverGetProducer(RTC::RtpObserver* rtpObserver, const std::string& id) override;
		void OnRtpObserverAddProducer(RTC::RtpObserver* rtpObserver, RTC::Producer* producer) override;
		void OnRtpObserverRemoveProducer(RTC::RtpObserver* rtpObserver, RTC::Producer* producer) override;

	public:
		//void createWebRtcServer(const server::ListenInfo& info);
		server::CreateTransportResponse createTransport(const std::string& transportId);
		void connectTransport(const std::string& transportId, const std::string& role, const server::DtlsFingerprint&);
		void addProducer(const std::string& transport_id, const std::string& producer_id,
                         const std::string& kind, const server::RtpParameters& rtp_params);
        void addConsumer(const std::string& transportId, const std::string& producerId,
                         const std::string& consumerId, const std::string& kind,
                         const server::RtpParameters& rtp_params);

	public:
		// Passed by argument.
		std::string id;

	private:
		// Passed by argument.
		RTC::Shared* shared { nullptr };
		WebRtcServer* webRtcServer { nullptr };
		// Allocated by this.
		absl::flat_hash_map<std::string, RTC::Transport*> mapTransports;
		absl::flat_hash_map<std::string, RTC::RtpObserver*> mapRtpObservers;
		// Others.
		absl::flat_hash_map<RTC::Producer*, absl::flat_hash_set<RTC::Consumer*>> mapProducerConsumers;
		absl::flat_hash_map<RTC::Consumer*, RTC::Producer*> mapConsumerProducer;
		absl::flat_hash_map<RTC::Producer*, absl::flat_hash_set<RTC::RtpObserver*>> mapProducerRtpObservers;
		absl::flat_hash_map<std::string, RTC::Producer*> mapProducers;
		absl::flat_hash_map<RTC::DataProducer*, absl::flat_hash_set<RTC::DataConsumer*>>
		  mapDataProducerDataConsumers;
		absl::flat_hash_map<RTC::DataConsumer*, RTC::DataProducer*> mapDataConsumerDataProducer;
		absl::flat_hash_map<std::string, RTC::DataProducer*> mapDataProducers;
	};
} // namespace RTC

#endif

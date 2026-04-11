#define MS_CLASS "RTC::RTCP::FeedbackPs"
// #define MS_LOG_DEV_LEVEL 3

#include "rtc/rtcp/FeedbackPs.hpp"
#include "Logger.hpp"
#include "rtc/rtcp/FeedbackItem.hpp"
#include "rtc/rtcp/FeedbackPsFir.hpp"
#include "rtc/rtcp/FeedbackPsLei.hpp"
#include "rtc/rtcp/FeedbackPsRpsi.hpp"
#include "rtc/rtcp/FeedbackPsSli.hpp"
#include "rtc/rtcp/FeedbackPsTst.hpp"
#include "rtc/rtcp/FeedbackPsVbcm.hpp"

namespace RTC
{
	namespace RTCP
	{
		/* Class methods. */

		template<typename Item>
		FeedbackPsItemsPacket<Item>* FeedbackPsItemsPacket<Item>::Parse(const uint8_t* data, size_t len)
		{
			MS_TRACE();

			if (len < Packet::CommonHeaderSize + FeedbackPacket::HeaderSize)
			{
				MS_WARN_TAG(rtcp, "not enough space for Feedback packet, discarded");

				return nullptr;
			}

			// NOLINTNEXTLINE(llvm-qualified-auto)
			auto* commonHeader = const_cast<CommonHeader*>(reinterpret_cast<const CommonHeader*>(data));

			std::unique_ptr<FeedbackPsItemsPacket<Item>> packet(
			  new FeedbackPsItemsPacket<Item>(commonHeader));

			size_t offset = Packet::CommonHeaderSize + FeedbackPacket::HeaderSize;

			while (len > offset)
			{
				auto* item = FeedbackItem::Parse<Item>(data + offset, len - offset);

				if (item)
				{
					if (!item->IsCorrect())
					{
						delete item;

						break;
					}

					packet->AddItem(item);
					offset += item->GetSize();
				}
				else
				{
					break;
				}
			}

			return packet.release();
		}

		/* Instance methods. */

		template<typename Item>
		size_t FeedbackPsItemsPacket<Item>::Serialize(uint8_t* buffer)
		{
			MS_TRACE();

			size_t offset = FeedbackPacket::Serialize(buffer);

			for (auto* item : this->items)
			{
				offset += item->Serialize(buffer + offset);
			}

			return offset;
		}

		template<typename Item>
		void FeedbackPsItemsPacket<Item>::Dump(int indentation) const
		{
			MS_TRACE();

			MS_DUMP_CLEAN(
			  indentation, "<%s>", FeedbackPsPacket::MessageType2String(Item::MessageType).c_str());
			FeedbackPsPacket::Dump(indentation + 1);
			for (auto* item : this->items)
			{
				item->Dump(indentation + 1);
			}
			MS_DUMP_CLEAN(
			  indentation, "</%s>", FeedbackPsPacket::MessageType2String(Item::MessageType).c_str());
		}

		// explicit instantiation to have all FeedbackRtpPacket definitions in this file.
		template class FeedbackPsItemsPacket<FeedbackPsFirItem>;
		template class FeedbackPsItemsPacket<FeedbackPsSliItem>;
		template class FeedbackPsItemsPacket<FeedbackPsRpsiItem>;
		template class FeedbackPsItemsPacket<FeedbackPsTstrItem>;
		template class FeedbackPsItemsPacket<FeedbackPsTstnItem>;
		template class FeedbackPsItemsPacket<FeedbackPsVbcmItem>;
		template class FeedbackPsItemsPacket<FeedbackPsLeiItem>;
	} // namespace RTCP
} // namespace RTC

#ifndef THERON_DETAIL_MESSAGES_MESSAGESENDER_H
#define THERON_DETAIL_MESSAGES_MESSAGESENDER_H

#include "xlang2\x_address.h"
#include "xlang2\x_assert.h"
#include "xlang2\x_basictypes.h"
#include "xlang2\x_defines.h"
#include "xlang2\x_iallocator.h"
#include "xlang2\x_receiver.h"

#include "xlang2\private\directory\x_entry.h"
#include "xlang2\private\directory\x_staticdirectory.h"
#include "xlang2\private\handlers\x_fallbackhandlercollection.h"
#include "xlang2\private\messages\x_imessage.h"
#include "xlang2\private\mailboxes\x_mailbox.h"
#include "xlang2\private\mailboxprocessor\x_processor.h"


namespace xlang2
{
	namespace Detail
	{
		/**
		Helper class that sends allocated internal message objects.
		*/
		class MessageSender
		{
		public:

			/**
			Sends an allocated message to the given address.
			*/
			static bool Send(void * const dummy,
				Processor::Context *const processorContext,
				const uint32_t localFrameworkIndex,
				IMessage *const message,
				const Address &address,
				const bool localQueue = false);

			/**
			Delivers an allocated message to a receiver or an actor in some framework within this process.
			This function is non-inlined so serves as a call-point to avoid excessive inlining.
			*/
			static bool DeliverWithinLocalProcess(IMessage *const message, const Index &index);
		};


	} // namespace Detail
} // namespace xlang2


#endif // THERON_DETAIL_MESSAGES_MESSAGESENDER_H

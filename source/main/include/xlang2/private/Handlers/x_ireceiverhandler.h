#ifndef THERON_DETAIL_HANDLERS_IRECEIVERHANDLER_H
#define THERON_DETAIL_HANDLERS_IRECEIVERHANDLER_H

#include "xlang2\x_basictypes.h"
#include "xlang2\x_defines.h"

#include "xlang2\private\containers\x_list.h"
#include "xlang2\private\messages\x_imessage.h"

namespace xlang2
{
	namespace Detail
	{
		/**
		Baseclass that allows message handlers of various types to be stored in lists.
		*/
		class IReceiverHandler : public List<IReceiverHandler>::Node
		{
		public:

			/**
			Default constructor.
			*/
			THERON_FORCEINLINE IReceiverHandler()
			{
			}

			/**
			Virtual destructor.
			*/
			inline virtual ~IReceiverHandler()
			{
			}

			/**
			Returns the unique name of the message type handled by this handler.
			*/
			virtual const char *GetMessageTypeName() const = 0;

			/**
			Handles the given message, if it's of the type accepted by the handler.
			\return True, if the handler handled the message.
			*/
			virtual bool Handle(const IMessage *const message) const = 0;

		private:

			IReceiverHandler(const IReceiverHandler &other);
			IReceiverHandler &operator=(const IReceiverHandler &other);
		};


	} // namespace Detail
} // namespace xlang2


#endif // THERON_DETAIL_HANDLERS_IRECEIVERHANDLER_H

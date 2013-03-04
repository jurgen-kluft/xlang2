#ifndef THERON_DETAIL_HANDLERS_RECEIVERHANDLERCAST_H
#define THERON_DETAIL_HANDLERS_RECEIVERHANDLERCAST_H

#include "xlang2\x_assert.h"
#include "xlang2\x_defines.h"

#include "xlang2\private\handlers\x_ireceiverhandler.h"
#include "xlang2\private\handlers\x_receiverhandler.h"
#include "xlang2\private\messages\x_messagetraits.h"

namespace xlang2
{
	namespace Detail
	{
		/**
		\brief Dynamic cast utility for message handler pointers.
		*/
		template <class ObjectType, bool HAS_TYPE_NAME>
		class ReceiverHandlerCast
		{
		public:

			/**
			\brief Attempts to convert a given message handler, of unknown type, to one of a target type.
			*/
			template <class ValueType>
			THERON_FORCEINLINE static const ReceiverHandler<ObjectType, ValueType> *CastHandler(const IReceiverHandler *const handler)
			{
				THERON_ASSERT(handler);

				// If explicit type names are used then they must be defined for all message types.
				THERON_ASSERT_MSG(handler->GetMessageTypeName(), "Missing type name for message type");

				// Compare the handlers using type names.
				if (handler->GetMessageTypeName() != MessageTraits<ValueType>::TYPE_NAME)
				{
					return 0;
				}

				// Convert the given message handler to a handler for the known type.
				typedef ReceiverHandler<ObjectType, ValueType> HandlerType;
				return reinterpret_cast<const HandlerType *>(handler);
			}
		};

	} // namespace Detail
} // namespace xlang2


#endif // THERON_DETAIL_HANDLERS_RECEIVERHANDLERCAST_H

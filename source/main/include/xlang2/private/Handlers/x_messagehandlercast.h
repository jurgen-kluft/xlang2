#ifndef THERON_DETAIL_HANDLERS_MESSAGEHANDLERCAST_H
#define THERON_DETAIL_HANDLERS_MESSAGEHANDLERCAST_H

#include "xlang2\x_assert.h"
#include "xlang2\x_defines.h"

#include "xlang2\private\handlers\x_imessagehandler.h"
#include "xlang2\private\handlers\x_messagehandler.h"
#include "xlang2\private\messages\x_messagetraits.h"

namespace xlang2
{
	namespace Detail
	{
		/**
		\brief Dynamic cast utility for message handler pointers.
		A cast utility that can be used to dynamically cast a message handler of unknown type
		to a message handler of a known type at runtime, using stored runtime type information.
		If the unknown message handler is of the target type then the cast succeeds and a pointer
		to the typecast message handler is returned, otherwise a null pointer is returned.

		This utility roughly mimics the functionality of dynamic_cast, but includes
		two alternate implementations: one that uses dynamic_cast directly and another
		that rolls its own runtime type information only for message handler classes. The
		advantage of the second implementation is that the storage overhead of the extra
		runtime type information is not imposed on \em all classes, as with the C++ RTTI.
		If the second implementation is used consistently then dynamic_cast is not
		called at all, using a partial template specialization trick, and the C++ RTTI
		functionality can be turned off (usually by means of a compiler option).

		\note Partial template specialization is used here as a device to avoid
		introducing a hardcoded dependency on C++ RTTI. The dependency can be avoided
		by defining non-zero type names for all message types used in the application.

		\tparam ActorType The actor class for which the handler is registered.
		\tparam HAS_TYPE_NAME A flag indicating whether the message type has a name.
		*/
		template <class ActorType, bool HAS_TYPE_NAME>
		class MessageHandlerCast
		{
		public:

			/**
			\brief Attempts to convert a given message handler, of unknown type, to one of a target type.
			Returns a null pointer if the message handler is of the wrong type.
			\tparam ValueType The value type of the target message handler.
			\param handler A pointer to the message handler of unknown type.
			\return A pointer to the converted message handler, or null if the types don't match.
			*/
			template <class ValueType>
			THERON_FORCEINLINE static const MessageHandler<ActorType, ValueType> *CastHandler(const IMessageHandler *const handler)
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
				typedef MessageHandler<ActorType, ValueType> HandlerType;
				return reinterpret_cast<const HandlerType *>(handler);
			}
		};

	} // namespace Detail
} // namespace xlang2


#endif // THERON_DETAIL_HANDLERS_MESSAGEHANDLERCAST_H

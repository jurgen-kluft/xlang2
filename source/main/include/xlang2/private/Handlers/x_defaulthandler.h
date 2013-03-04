#ifndef THERON_DETAIL_HANDLERS_DEFAULTHANDLER_H
#define THERON_DETAIL_HANDLERS_DEFAULTHANDLER_H

#include "xlang2\private\messages\x_imessage.h"
#include "xlang2\private\handlers\x_idefaulthandler.h"

#include "xlang2\x_address.h"
#include "xlang2\x_assert.h"
#include "xlang2\x_defines.h"

namespace Theron
{
	class Actor;

	namespace Detail
	{
		/**
		Instantiable class template that remembers a default handler function.
		*/
		template <class ActorType>
		class DefaultHandler : public IDefaultHandler
		{
		public:

			/**
			Pointer to a member function of the actor type, which is the handler.
			*/
			typedef void (ActorType::*HandlerFunction)(const Address from);

			/**
			Constructor.
			*/
			THERON_FORCEINLINE explicit DefaultHandler(HandlerFunction function) : mHandlerFunction(function)
			{
			}

			/**
			Virtual destructor.
			*/
			inline virtual ~DefaultHandler()
			{
			}

			/**
			Handles the given message.
			\note The message is not consumed by the handler; just acted on or ignored.
			The message will be automatically destroyed when all handlers have seen it.
			*/
			inline virtual void Handle(Actor *const actor, const IMessage *const message) const;

		private:

			DefaultHandler(const DefaultHandler &other);
			DefaultHandler &operator=(const DefaultHandler &other);

			const HandlerFunction mHandlerFunction;     ///< Pointer to a handler member function on the actor.
		};


		template <class ActorType>
		inline void DefaultHandler<ActorType>::Handle(Actor *const actor, const IMessage *const message) const
		{
			THERON_ASSERT(actor);
			THERON_ASSERT(message);
			THERON_ASSERT(mHandlerFunction);

			// Call the handler, passing it the from address.
			// We can't pass the value because we don't even know the type.
			ActorType *const typedActor = static_cast<ActorType *>(actor);
			(typedActor->*mHandlerFunction)(message->From());
		}


	} // namespace Detail
} // namespace Theron


#endif // THERON_DETAIL_HANDLERS_DEFAULTHANDLER_H
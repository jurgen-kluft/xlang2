#ifndef THERON_DETAIL_HANDLERS_FALLBACKHANDLERCOLLECTION_H
#define THERON_DETAIL_HANDLERS_FALLBACKHANDLERCOLLECTION_H

#include <new>

#include "xlang2\x_address.h"
#include "xlang2\x_allocatormanager.h"
#include "xlang2\x_basictypes.h"
#include "xlang2\x_defines.h"
#include "xlang2\x_iallocator.h"

#include "xlang2\private\handlers\x_blindfallbackhandler.h"
#include "xlang2\private\handlers\x_fallbackhandler.h"
#include "xlang2\private\handlers\x_ifallbackhandler.h"
#include "xlang2\private\messages\x_imessage.h"


namespace xlang2
{
	namespace Detail
	{
		/**
		A collection of handlers for messages of different types.
		*/
		class FallbackHandlerCollection
		{
		public:

			/**
			Default constructor.
			*/
			FallbackHandlerCollection();

			/**
			Destructor.
			*/
			~FallbackHandlerCollection();

			/**
			Sets the handler in the collection.
			*/
			template <class ObjectType>
			inline bool Set(ObjectType *const handlerObject, void (ObjectType::*handler)(const Address from));

			/**
			Sets the handler in the collection.
			*/
			template <class ObjectType>
			inline bool Set(ObjectType *const handlerObject, void (ObjectType::*handler)(const void *const data, const uint32_t size, const Address from));

			/**
			Removes any registered handler from the collection.
			*/
			bool Clear();

			/**
			Handles the given message, passing it to each of the handlers in the collection.
			\return True, if one or more of the handlers in the collection handled the message.
			*/
			bool Handle(const IMessage *const message);

		private:

			FallbackHandlerCollection(const FallbackHandlerCollection &other);
			FallbackHandlerCollection &operator=(const FallbackHandlerCollection &other);

			/**
			Updates the registered handlers with any changes made since the last message was processed.
			\note This function is intentionally not force-inlined since the handlers don't usually change often.
			*/
			void UpdateHandlers();

			IFallbackHandler *mHandler;         ///< Currently registered handler (only one is supported).
			IFallbackHandler *mNewHandler;      ///< New registered handler (will replace the registered one on update).
			bool mHandlersDirty;                ///< Flag indicating that the handlers are out of date.
		};


		template <class ObjectType>
		inline bool FallbackHandlerCollection::Set(ObjectType *const handlerObject, void (ObjectType::*handler)(const Address from))
		{
			typedef FallbackHandler<ObjectType> MessageHandlerType;

			IAllocator *const allocator(AllocatorManager::Instance().GetAllocator());

			mHandlersDirty = true;

			// Destroy any previously set new handler.
			if (mNewHandler)
			{
				mNewHandler->~IFallbackHandler();
				allocator->Free(mNewHandler);
				mNewHandler = 0;
			}

			if (handlerObject && handler)
			{
				// Allocate memory for a message handler object.
				void *const memory = allocator->Allocate(sizeof(MessageHandlerType));
				if (memory == 0)
				{
					return false;
				}

				// Construct a handler object to remember the function pointer and message value type.
				mNewHandler = new (memory) MessageHandlerType(handlerObject, handler);
			}

			return true;
		}


		template <class ObjectType>
		inline bool FallbackHandlerCollection::Set(ObjectType *const handlerObject, void (ObjectType::*handler)(const void *const data, const uint32_t size, const Address from))
		{
			typedef BlindFallbackHandler<ObjectType> MessageHandlerType;

			IAllocator *const allocator(AllocatorManager::Instance().GetAllocator());

			mHandlersDirty = true;

			// Destroy any previously set new handler.
			if (mNewHandler)
			{
				mNewHandler->~IFallbackHandler();
				allocator->Free(mNewHandler);
				mNewHandler = 0;
			}

			if (handlerObject && handler)
			{
				// Allocate memory for a message handler object.
				void *const memory = allocator->Allocate(sizeof(MessageHandlerType));
				if (memory == 0)
				{
					return false;
				}

				// Construct a handler object to remember the function pointer and message value type.
				mNewHandler = new (memory) MessageHandlerType(handlerObject, handler);
			}

			return true;
		}


	} // namespace Detail
} // namespace xlang2


#endif // THERON_DETAIL_HANDLERS_FALLBACKHANDLERCOLLECTION_H

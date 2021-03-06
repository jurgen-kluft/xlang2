#include "xlang2\private\handlers\x_defaulthandlercollection.h"

namespace xlang2
{
	namespace Detail
	{
		DefaultHandlerCollection::DefaultHandlerCollection()
			: mHandler(0)
			, mNewHandler(0)
			, mHandlersDirty(false)
		{
		}


		DefaultHandlerCollection::~DefaultHandlerCollection()
		{
			Clear();
		}


		bool DefaultHandlerCollection::Clear()
		{
			IAllocator *const allocator(AllocatorManager::Instance().GetAllocator());

			// Destroy any currently set handlers.
			if (mHandler)
			{
				mHandler->~IDefaultHandler();
				allocator->Free(mHandler);
				mHandler = 0;
			}

			if (mNewHandler)
			{
				mNewHandler->~IDefaultHandler();
				allocator->Free(mNewHandler);
				mNewHandler = 0;
			}

			mHandlersDirty = false;
			return true;
		}


		bool DefaultHandlerCollection::Handle(Actor *const actor, const IMessage *const message)
		{
			bool handled(false);

			THERON_ASSERT(actor);
			THERON_ASSERT(message);

			// Update the handler list if there have been changes.
			if (mHandlersDirty)
			{
				UpdateHandlers();
			}

			if (mHandler)
			{
				mHandler->Handle(actor, message);
				handled = true;
			}

			return handled;
		}


		void DefaultHandlerCollection::UpdateHandlers()
		{
			IAllocator *const allocator(AllocatorManager::Instance().GetAllocator());

			mHandlersDirty = false;

			// Destroy any currently set handler.
			if (mHandler)
			{
				mHandler->~IDefaultHandler();
				allocator->Free(mHandler);
				mHandler = 0;
			}

			// Make the new handler (if any) the current handler.
			mHandler = mNewHandler;
			mNewHandler = 0;
		}


	} // namespace Detail
} // namespace xlang2



#include "xlang2\private\handlers\x_fallbackhandlercollection.h"


namespace xlang2
{
	namespace Detail
	{
		FallbackHandlerCollection::FallbackHandlerCollection()
			: mHandler(0)
			, mNewHandler(0)
			, mHandlersDirty(false)
		{
		}

		FallbackHandlerCollection::~FallbackHandlerCollection()
		{
			Clear();
		}

		bool FallbackHandlerCollection::Clear()
		{
			IAllocator *const allocator(AllocatorManager::Instance().GetAllocator());

			// Destroy any currently set handlers.
			if (mHandler)
			{
				mHandler->~IFallbackHandler();
				allocator->Free(mHandler);
				mHandler = 0;
			}

			if (mNewHandler)
			{
				mNewHandler->~IFallbackHandler();
				allocator->Free(mNewHandler);
				mNewHandler = 0;
			}

			mHandlersDirty = false;
			return true;
		}


		bool FallbackHandlerCollection::Handle(const IMessage *const message)
		{
			bool handled(false);

			THERON_ASSERT(message);

			// Update the handler list if there have been changes.
			if (mHandlersDirty)
			{
				UpdateHandlers();
			}

			if (mHandler)
			{
				mHandler->Handle(message);
				handled = true;
			}

			return handled;
		}


		void FallbackHandlerCollection::UpdateHandlers()
		{
			IAllocator *const allocator(AllocatorManager::Instance().GetAllocator());

			mHandlersDirty = false;

			// Destroy any currently set handler.
			if (mHandler)
			{
				mHandler->~IFallbackHandler();
				allocator->Free(mHandler);
				mHandler = 0;
			}

			// Make the new handler (if any) the current handler.
			mHandler = mNewHandler;
			mNewHandler = 0;
		}


	} // namespace Detail
} // namespace xlang2



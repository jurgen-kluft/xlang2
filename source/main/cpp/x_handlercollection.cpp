#include "xlang2\private\handlers\x_handlercollection.h"

namespace xlang2
{
	namespace Detail
	{
		HandlerCollection::HandlerCollection()
			: mHandlers()
			, mNewHandlers()
			, mHandlersDirty(false)
	{
	}

	HandlerCollection::~HandlerCollection()
	{
		Clear();
	}

	void HandlerCollection::UpdateHandlers()
	{
		IAllocator *const allocator(AllocatorManager::Instance().GetAllocator());

		mHandlersDirty = false;

		// Add any new handlers. We do this first in case any are already marked for deletion.
		// The handler class contains the next pointer, so handlers can only be in one list at a time.
		while (IMessageHandler *const handler = mNewHandlers.Front())
		{
			mNewHandlers.Remove(handler);
			mHandlers.Insert(handler);
		}

		// Transfer all handlers to the new handler list, omitting any which are marked for deletion.    
		while (IMessageHandler *const handler = mHandlers.Front())
		{
			mHandlers.Remove(handler);
			if (handler->IsMarked())
			{
				handler->~IMessageHandler();
				allocator->Free(handler);
			}
			else
			{
				mNewHandlers.Insert(handler);
			}
		}

		// Finally transfer the filtered handlers back in the actual list.
		while (IMessageHandler *const handler = mNewHandlers.Front())
		{
			mNewHandlers.Remove(handler);
			mHandlers.Insert(handler);
		}
	}


	} // namespace Detail
} // namespace xlang2



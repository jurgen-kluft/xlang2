#include "xlang2\x_actor.h"
#include "xlang2\x_assert.h"
#include "xlang2\x_framework.h"

namespace xlang2
{
	Actor::Actor()
		: mAddress()
		, mFramework(0)
		, mMessageHandlers()
		, mDefaultHandlers()
		, mProcessorContext(0)
		, mRefCount(0)
		, mMemory(0)
	{
	}

	Actor::Actor(Framework &framework, const char *const name) 
		: mAddress()
		, mFramework(&framework)
		, mMessageHandlers()
		, mDefaultHandlers()
		, mProcessorContext(0)
		, mRefCount(0)
		, mMemory(0)
	{
		// Check that the actor isn't being constructed by Framework::CreateActor.
		THERON_ASSERT_MSG(Detail::ActorRegistry::Lookup(this) == 0, "Use default Theron::Actor::Actor() baseclass constructor with CreateActor().");

		// Claim an available directory index and mailbox for this actor.
		mFramework->RegisterActor(this, name);
	}

	Actor::~Actor()
	{
		mFramework->DeregisterActor(this);
	}

	void Actor::Fallback(Detail::FallbackHandlerCollection *const fallbackHandlers, const Detail::IMessage *const message)
	{
		// If default handlers are registered with this actor, execute those.
		if (mDefaultHandlers.Handle(this, message))
		{
			return;
		}

		// Let the framework's fallback handlers handle the message.
		fallbackHandlers->Handle(message);
	}

} // namespace xlang2

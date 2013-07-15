#include "xunittest\xunittest.h"

#include <string.h>
#include <vector>
#include <string>
#include <queue>

#include "xlang2/x_theron.h"
#include "xlang2/private/threading/x_utils.h"


#define CHECK_TRUE_T(value, msg) \
	UT_TRY_BEGIN \
	if (UnitTest::check((value)!=0, false)) \
		testResults_.onTestFailure(__FILE__, __LINE__, mTestName, msg); \
	UT_CATCH_ALL \
		testResults_.onTestFailure(__FILE__, __LINE__, mTestName, "Unhandled exception in CHECK_TRUE(" #value ")"); \
	UT_CATCH_END



class TrivialActor : public xlang2::Actor
{
public:

	TrivialActor(xlang2::Framework &framework) : xlang2::Actor(framework)
	{
	}
};

template <class MessageType>
class Registrar : public xlang2::Actor
{
public:

	Registrar(xlang2::Framework &framework) : xlang2::Actor(framework)
	{
		RegisterHandler(this, &Registrar::Handler);
		IsHandlerRegistered(this, &Registrar::Handler);
		DeregisterHandler(this, &Registrar::Handler);
		IsHandlerRegistered(this, &Registrar::Handler);
	}

private:

	inline void Handler(const MessageType &/*message*/, const xlang2::Address /*from*/)
	{
	}
};

template <class MessageType>
class Replier : public xlang2::Actor
{
public:

	inline explicit Replier(xlang2::Framework &framework, const char *const name = 0) : xlang2::Actor(framework, name)
	{
		RegisterHandler(this, &Replier::Handler);
	}

private:

	inline void Handler(const MessageType &message, const xlang2::Address from)
	{
		Send(message, from);
	}
};

template <class MessageType>
class DefaultReplier : public xlang2::Actor
{
public:

	inline DefaultReplier(xlang2::Framework &framework) : xlang2::Actor(framework)
	{
		RegisterHandler(this, &DefaultReplier::Handler);
		SetDefaultHandler(this, &DefaultReplier::DefaultHandler);
	}

private:

	inline void Handler(const MessageType &message, const xlang2::Address from)
	{
		Send(message, from);
	}

	inline void DefaultHandler(const xlang2::Address from)
	{
		std::string hello("hello");
		Send(hello, from);
	}
};

class StringReplier : public Replier<const char *>
{
public:

	typedef Replier<const char *> Base;

	inline StringReplier(xlang2::Framework &framework) : Base(framework)
	{
	}
};

class Signaller : public xlang2::Actor
{
public:

	inline Signaller(xlang2::Framework &framework, const char *const name = 0) : xlang2::Actor(framework, name)
	{
		RegisterHandler(this, &Signaller::Signal);
	}

private:

	inline void Signal(const xlang2::Address &address, const xlang2::Address from)
	{
		// Send the 'from' address to the address received in the message.
		Send(from, address);
	}
};

class Switcher : public xlang2::Actor
{
public:

	inline Switcher(xlang2::Framework &framework) : xlang2::Actor(framework)
	{
		RegisterHandler(this, &Switcher::SayHello);
	}

private:

	inline void SayHello(const std::string &/*message*/, const xlang2::Address from)
	{
		DeregisterHandler(this, &Switcher::SayHello);
		RegisterHandler(this, &Switcher::SayGoodbye);
		Send(std::string("hello"), from);
	}

	inline void SayGoodbye(const std::string &/*message*/, const xlang2::Address from)
	{
		DeregisterHandler(this, &Switcher::SayGoodbye);
		RegisterHandler(this, &Switcher::SayHello);
		Send(std::string("goodbye"), from);
	}
};

template <class MessageType>
class Catcher
{
public:

	inline Catcher() : mMessage(), mFrom(xlang2::Address::Null())
	{
	}

	inline void Catch(const MessageType &message, const xlang2::Address from)
	{
		mMessage = message;
		mFrom = from;
	}

	MessageType mMessage;
	xlang2::Address mFrom;
};

class Counter : public xlang2::Actor
{
public:

	inline Counter(xlang2::Framework &framework) : xlang2::Actor(framework), mCount(0)
	{
		RegisterHandler(this, &Counter::Increment);
		RegisterHandler(this, &Counter::GetValue);
	}

private:

	inline void Increment(const int &message, const xlang2::Address /*from*/)
	{
		mCount += message;
	}

	inline void GetValue(const bool &/*message*/, const xlang2::Address from)
	{
		Send(mCount, from);
	}

	int mCount;
};

class TwoHandlerCounter : public xlang2::Actor
{
public:

	inline TwoHandlerCounter(xlang2::Framework &framework) : xlang2::Actor(framework), mCount(0)
	{
		RegisterHandler(this, &TwoHandlerCounter::IncrementOne);
		RegisterHandler(this, &TwoHandlerCounter::IncrementTwo);
		RegisterHandler(this, &TwoHandlerCounter::GetValue);
	}

private:

	inline void IncrementOne(const int &message, const xlang2::Address /*from*/)
	{
		mCount += message;
	}

	inline void IncrementTwo(const float &/*message*/, const xlang2::Address /*from*/)
	{
		++mCount;
	}

	inline void GetValue(const bool &/*message*/, const xlang2::Address from)
	{
		Send(mCount, from);
	}

	int mCount;
};

class MultipleHandlerCounter : public xlang2::Actor
{
public:

	inline MultipleHandlerCounter(xlang2::Framework &framework) : xlang2::Actor(framework), mCount(0)
	{
		RegisterHandler(this, &MultipleHandlerCounter::IncrementOne);
		RegisterHandler(this, &MultipleHandlerCounter::IncrementTwo);
		RegisterHandler(this, &MultipleHandlerCounter::GetValue);
	}

private:

	inline void IncrementOne(const int &message, const xlang2::Address /*from*/)
	{
		mCount += message;
	}

	inline void IncrementTwo(const int &/*message*/, const xlang2::Address /*from*/)
	{
		++mCount;
	}

	inline void GetValue(const bool &/*message*/, const xlang2::Address from)
	{
		Send(mCount, from);
	}

	int mCount;
};

template <class CountType>
class Sequencer : public xlang2::Actor
{
public:

	static const char *GOOD;
	static const char *BAD;

	inline Sequencer(xlang2::Framework &framework) : xlang2::Actor(framework), mNextValue(0), mStatus(GOOD)
	{
		RegisterHandler(this, &Sequencer::Receive);
		RegisterHandler(this, &Sequencer::GetValue);
	}

private:

	inline void Receive(const CountType &message, const xlang2::Address /*from*/)
	{
		if (message != mNextValue++)
		{
			mStatus = BAD;
		}
	}

	inline void GetValue(const bool &/*message*/, const xlang2::Address from)
	{
		Send(mStatus, from);
	}

	CountType mNextValue;
	const char *mStatus;
};

class Recursor : public xlang2::Actor
{
public:

	struct Parameters
	{
		int mCount;
	};

	inline Recursor(xlang2::Framework &framework, const Parameters &params) : xlang2::Actor(framework)
	{
		// Recursively create a child actor within the constructor.
		if (params.mCount > 0)
		{
			Parameters childParams;
			childParams.mCount = params.mCount - 1;

			xlang2::Framework &framework(GetFramework());
			Recursor child(framework, childParams);
		}
	}
};

class TailRecursor : public xlang2::Actor
{
public:

	struct Parameters
	{
		int mCount;
	};

	inline TailRecursor(xlang2::Framework &framework, const Parameters &params) : xlang2::Actor(framework), mCount(params.mCount)
	{
	}

	inline ~TailRecursor()
	{
		// Recursively create a child actor within the destructor.
		if (mCount > 0)
		{
			Parameters childParams;
			childParams.mCount = mCount - 1;

			xlang2::Framework &framework(GetFramework());
			TailRecursor child(framework, childParams);
		}
	}

private:

	int mCount;
};

class AutoSender : public xlang2::Actor
{
public:

	typedef xlang2::Address Parameters;

	inline AutoSender(xlang2::Framework &framework, const Parameters &address) : xlang2::Actor(framework)
	{
		// Send a message in the actor constructor.
		Send(0, address);

		// Send using TailSend to check that works too.
		TailSend(1, address);
	}
};

class TailSender : public xlang2::Actor
{
public:

	typedef xlang2::Address Parameters;

	inline TailSender(xlang2::Framework &framework, const Parameters &address) : xlang2::Actor(framework), mStoredAddress(address)
	{
	}

	inline ~TailSender()
	{
		// Send a message in the actor destructor.
		Send(0, mStoredAddress);

		// Send using TailSend to check that works too.
		TailSend(1, mStoredAddress);
	}

	xlang2::Address mStoredAddress;
};

class AutoDeregistrar : public xlang2::Actor
{
public:

	inline AutoDeregistrar(xlang2::Framework &framework) : xlang2::Actor(framework)
	{
		RegisterHandler(this, &AutoDeregistrar::HandlerOne);
		RegisterHandler(this, &AutoDeregistrar::HandlerTwo);
		DeregisterHandler(this, &AutoDeregistrar::HandlerOne);
	}

	inline void HandlerOne(const int &/*message*/, const xlang2::Address from)
	{
		Send(1, from);
	}

	inline void HandlerTwo(const int &/*message*/, const xlang2::Address from)
	{
		Send(2, from);
	}
};

class TailDeregistrar : public xlang2::Actor
{
public:

	inline TailDeregistrar(xlang2::Framework &framework) : xlang2::Actor(framework)
	{
		RegisterHandler(this, &TailDeregistrar::Handler);
	}

	inline ~TailDeregistrar()
	{
		DeregisterHandler(this, &TailDeregistrar::Handler);
	}

	inline void Handler(const int &/*message*/, const xlang2::Address from)
	{
		Send(0, from);
	}
};

class MessageQueueCounter : public xlang2::Actor
{
public:

	inline explicit MessageQueueCounter(xlang2::Framework &framework) : xlang2::Actor(framework)
	{
		RegisterHandler(this, &MessageQueueCounter::Handler);
	}

	inline void Handler(const int &/*message*/, const xlang2::Address from)
	{
		Send(GetNumQueuedMessages(), from);
	}
};

class BlindActor : public xlang2::Actor
{
public:

	inline BlindActor(xlang2::Framework &framework) : xlang2::Actor(framework)
	{
		SetDefaultHandler(this, &BlindActor::BlindDefaultHandler);
	}

private:

	inline void BlindDefaultHandler(const void *const data, const xlang2::uint32_t size, const xlang2::Address from)
	{
		// We know the message is a uint32_t.
		const xlang2::uint32_t *const p(reinterpret_cast<const xlang2::uint32_t *>(data));
		const xlang2::uint32_t value(*p);

		Send(value, from);
		Send(size, from);
	}
};

template <class MessageType>
class Accumulator
{
public:

	inline Accumulator() : mMessages()
	{
	}

	inline void Catch(const MessageType &message, const xlang2::Address /*from*/)
	{
		mMessages.push(message);
	}

	int Size()
	{
		return mMessages.size();
	}

	MessageType Pop()
	{
		THERON_ASSERT(mMessages.empty() == false);
		MessageType message(mMessages.front());
		mMessages.pop();
		return message;
	}

private:

	std::queue<MessageType> mMessages;
};

class HandlerChecker : public xlang2::Actor
{
public:

	inline HandlerChecker(xlang2::Framework &framework) : xlang2::Actor(framework)
	{
		RegisterHandler(this, &HandlerChecker::Check);
	}

private:

	inline void Check(const int & /*message*/, const xlang2::Address from)
	{
		Send(IsHandlerRegistered(this, &HandlerChecker::Check), from);
		Send(IsHandlerRegistered(this, &HandlerChecker::Dummy), from);
		Send(IsHandlerRegistered(this, &HandlerChecker::Unregistered), from);

		RegisterHandler(this, &HandlerChecker::Dummy);

		Send(IsHandlerRegistered(this, &HandlerChecker::Check), from);
		Send(IsHandlerRegistered(this, &HandlerChecker::Dummy), from);
		Send(IsHandlerRegistered(this, &HandlerChecker::Unregistered), from);

		DeregisterHandler(this, &HandlerChecker::Dummy);

		Send(IsHandlerRegistered(this, &HandlerChecker::Check), from);
		Send(IsHandlerRegistered(this, &HandlerChecker::Dummy), from);
		Send(IsHandlerRegistered(this, &HandlerChecker::Unregistered), from);

		DeregisterHandler(this, &HandlerChecker::Check);

		Send(IsHandlerRegistered(this, &HandlerChecker::Check), from);
		Send(IsHandlerRegistered(this, &HandlerChecker::Dummy), from);
		Send(IsHandlerRegistered(this, &HandlerChecker::Unregistered), from);

		RegisterHandler(this, &HandlerChecker::Dummy);
		RegisterHandler(this, &HandlerChecker::Check);
		RegisterHandler(this, &HandlerChecker::Check);

		Send(IsHandlerRegistered(this, &HandlerChecker::Check), from);
		Send(IsHandlerRegistered(this, &HandlerChecker::Dummy), from);
		Send(IsHandlerRegistered(this, &HandlerChecker::Unregistered), from);

		DeregisterHandler(this, &HandlerChecker::Check);

		Send(IsHandlerRegistered(this, &HandlerChecker::Check), from);
		Send(IsHandlerRegistered(this, &HandlerChecker::Dummy), from);
		Send(IsHandlerRegistered(this, &HandlerChecker::Unregistered), from);

		DeregisterHandler(this, &HandlerChecker::Check);

		Send(IsHandlerRegistered(this, &HandlerChecker::Check), from);
		Send(IsHandlerRegistered(this, &HandlerChecker::Dummy), from);
		Send(IsHandlerRegistered(this, &HandlerChecker::Unregistered), from);
	}

	inline void Dummy(const int & message, const xlang2::Address from)
	{
		// We do this just so that Dummy and Unregistered differ, so the compiler won't merge them!
		Send(message, from);
	}

	inline void Unregistered(const int & /*message*/, const xlang2::Address /*from*/)
	{
	}
};

class Nestor : public xlang2::Actor
{
public:

	struct CreateMessage { };
	struct DestroyMessage { };

	inline Nestor(xlang2::Framework &framework) : xlang2::Actor(framework)
	{
		RegisterHandler(this, &Nestor::Create);
		RegisterHandler(this, &Nestor::Destroy);
		RegisterHandler(this, &Nestor::Receive);
	}

private:

	typedef Replier<int> ChildActor;

	inline void Create(const CreateMessage & /*message*/, const xlang2::Address /*from*/)
	{
		mChildren.push_back(new ChildActor(GetFramework()));
		mChildren.push_back(new ChildActor(GetFramework()));
		mChildren.push_back(new ChildActor(GetFramework()));

		mReplies.push_back(false);
		mReplies.push_back(false);
		mReplies.push_back(false);

		Send(0, mChildren[0]->GetAddress());
		Send(1, mChildren[1]->GetAddress());
		Send(2, mChildren[2]->GetAddress());
	}

	inline void Destroy(const DestroyMessage & /*message*/, const xlang2::Address from)
	{
		mCaller = from;
		if (mReplies[0] && mReplies[1] && mReplies[2])
		{
			delete mChildren[0];
			delete mChildren[1];
			delete mChildren[2];

			mChildren.clear();
			Send(true, mCaller);
		}
	}

	inline void Receive(const int & message, const xlang2::Address /*from*/)
	{
		mReplies[message] = true;

		if (mCaller != xlang2::Address::Null() && mReplies[0] && mReplies[1] && mReplies[2])
		{
			delete mChildren[0];
			delete mChildren[1];
			delete mChildren[2];

			mChildren.clear();
			Send(true, mCaller);
		}
	}

	std::vector<xlang2::Actor *> mChildren;
	std::vector<bool> mReplies;
	xlang2::Address mCaller;
};
THERON_REGISTER_MESSAGE(Nestor::CreateMessage);
THERON_REGISTER_MESSAGE(Nestor::DestroyMessage);

class FallbackHandler
{
public:

	inline void Handle(const xlang2::Address from)
	{
		mAddress = from;
	}

	xlang2::Address mAddress;
};

class BlindFallbackHandler
{
public:

	BlindFallbackHandler() : mData(0), mValue(0), mSize(0)
	{
	}

	inline void Handle(const void *const data, const xlang2::uint32_t size, const xlang2::Address from)
	{
		mData = data;
		mValue = *reinterpret_cast<const xlang2::uint32_t *>(data);
		mSize = size;
		mAddress = from;
	}

	const void *mData;
	xlang2::uint32_t mValue;
	xlang2::uint32_t mSize;
	xlang2::Address mAddress;
};

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
typedef std::vector<int>	VectorMessage;
typedef float *				cfPointerMessage;
typedef const float *		fPointerMessage;
typedef const char *		StringMessage;
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------

class SomeOtherBaseclass
{
public:

	inline SomeOtherBaseclass()
	{
	}

	// The virtual destructor is required because this is a baseclass with virtual functions.
	inline virtual ~SomeOtherBaseclass()
	{
	}

	inline virtual void DoNothing()
	{
	}
};

class ActorFirst : public xlang2::Actor, public SomeOtherBaseclass
{
public:

	inline ActorFirst(xlang2::Framework &framework) : xlang2::Actor(framework)
	{
		RegisterHandler(this, &ActorFirst::Handler);
	}

	// The virtual destructor is required because we derived from baseclasses with virtual functions.
	inline virtual ~ActorFirst()
	{
	}

private:

	inline virtual void DoNothing()
	{
	}

	inline void Handler(const int &message, const xlang2::Address from)
	{
		Send(message, from);
	}
};

class ActorLast : public SomeOtherBaseclass, public xlang2::Actor
{
public:

	inline ActorLast(xlang2::Framework &framework) : xlang2::Actor(framework)
	{
		RegisterHandler(this, &ActorLast::Handler);
	}

	// The virtual destructor is required because we derived from baseclasses with virtual functions.
	inline virtual ~ActorLast()
	{
	}

private:

	inline virtual void DoNothing()
	{
	}

	inline void Handler(const int &message, const xlang2::Address from)
	{
		Send(message, from);
	}
};

struct EmptyMessage
{
};
THERON_REGISTER_MESSAGE(EmptyMessage);


class Forwarder : public xlang2::Actor
{
public:

	inline Forwarder(xlang2::Framework &framework, const xlang2::Address next) : xlang2::Actor(framework), mNext(next)
	{
		RegisterHandler(this, &Forwarder::Forward);
	}

private:

	inline void Forward(const int &message, const xlang2::Address /*from*/)
	{
		Send(message - 1, mNext);
	}

	const xlang2::Address mNext;
};

// Derive from a different baseclass first to check we cope with the non-zero offset.
class Version3Replier : public SomeOtherBaseclass, public xlang2::Actor
{
public:

	struct Parameters
	{
	};

	Version3Replier()
	{
		RegisterHandler(this, &Version3Replier::Handler);

		// Allocate memory in the constructor to check the destructor is called.
		mMemory = xlang2::AllocatorManager::Instance().GetAllocator()->Allocate(64);

		// Check the alignment of the final actor type.
		THERON_ASSERT(THERON_ALIGNED(this, 128));
	}

	Version3Replier(const Parameters &/*params*/)
	{
		RegisterHandler(this, &Version3Replier::Handler);

		// Allocate memory in the constructor to check the destructor is called.
		mMemory = xlang2::AllocatorManager::Instance().GetAllocator()->Allocate(64);

		// Check the alignment of the final actor type.
		THERON_ASSERT(THERON_ALIGNED(this, 128));
	}

	~Version3Replier()
	{
		DeregisterHandler(this, &Version3Replier::Handler);

		// Free the memory in the destructor to check the destructor is called.
		// The DefaultAllocator memory allocation checking will fail if we leak.
		xlang2::AllocatorManager::Instance().GetAllocator()->Free(mMemory);
	}

private:

	// Implementation of virtual method in SomeOtherBaseclass.
	inline virtual void DoNothing()
	{
	}

	void Handler(const int &message, const xlang2::Address from)
	{
		Send(message, from);
	}

	void *mMemory;
};

template <class CountType>
const char *Sequencer<CountType>::GOOD = "good";

template <class CountType>
const char *Sequencer<CountType>::BAD = "good";

THERON_REGISTER_MESSAGE(bool);
THERON_REGISTER_MESSAGE(int);
THERON_REGISTER_MESSAGE(unsigned int);
THERON_REGISTER_MESSAGE(float);
THERON_REGISTER_MESSAGE(std::string);
THERON_REGISTER_MESSAGE(VectorMessage);
THERON_REGISTER_MESSAGE(xlang2::Address);
THERON_REGISTER_MESSAGE(cfPointerMessage);
THERON_REGISTER_MESSAGE(fPointerMessage);
THERON_REGISTER_MESSAGE(StringMessage);

THERON_ALIGN_ACTOR(Version3Replier, 128);




UNITTEST_SUITE_BEGIN(xlang2_features)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(test)
		{

		}
		UNITTEST_TEST(ConstructFramework)
		{
			xlang2::Framework framework;
		}

		UNITTEST_TEST(ConstructFrameworkThreadCount)
		{
			xlang2::Framework framework(16);
		}

		UNITTEST_TEST(ConstructFrameworkDefaultParams)
		{
			xlang2::Framework::Parameters params;
			xlang2::Framework framework(params);
		}

		UNITTEST_TEST(ConstructFrameworkParamsThreadCount)
		{
			xlang2::Framework::Parameters params(16);
			xlang2::Framework framework(params);
		}

		UNITTEST_TEST(ConstructFrameworkParamsThreadCountNode)
		{
			xlang2::Framework::Parameters params(16, 0);
			xlang2::Framework framework(params);
		}

		UNITTEST_TEST(ConstructActor)
		{
			xlang2::Framework framework;
			TrivialActor actor(framework);
		}

		UNITTEST_TEST(ConstructMultipleActors)
		{
			xlang2::Framework framework;
			TrivialActor actor0(framework);
			TrivialActor actor1(framework);
		}

		UNITTEST_TEST(ConstructAddress)
		{
			xlang2::Address address;
			CHECK_TRUE_T(address == xlang2::Address::Null(), "Default-constructed address should be null");
		}


		UNITTEST_TEST(CopyAddress)
		{
			xlang2::Framework framework;
			TrivialActor actor(framework);
			const xlang2::Address address(actor.GetAddress());
			const xlang2::Address copied(address);

			CHECK_TRUE_T(address != xlang2::Address::Null(), "Actor address should be null");
			CHECK_TRUE_T(copied != xlang2::Address::Null(), "Copied actor address should be null");
			CHECK_TRUE_T(copied == address, "Address not copied correctly");
		}

		UNITTEST_TEST(AssignAddress)
		{
			xlang2::Framework framework;
			TrivialActor actor(framework);
			const xlang2::Address address(actor.GetAddress());
			xlang2::Address assigned;

			CHECK_TRUE_T(address != xlang2::Address::Null(), "Actor address should be null");

			assigned = address;
			CHECK_TRUE_T(assigned != xlang2::Address::Null(), "Copied actor address should be null");
			CHECK_TRUE_T(assigned == address, "Address not copied correctly");
		}

		UNITTEST_TEST(GetActorFramework)
		{
			xlang2::Framework framework;
			TrivialActor actor(framework);
			xlang2::Framework &actorFramework(actor.GetFramework());
			CHECK_TRUE_T(&actorFramework == &framework, "Actor framework incorrect");
		}

		UNITTEST_TEST(GetActorAddress)
		{
			xlang2::Framework framework;
			TrivialActor actor(framework);
			const xlang2::Address address(actor.GetAddress());
			CHECK_TRUE_T(address != xlang2::Address::Null(), "Actor address should not be null");
		}

		UNITTEST_TEST(RegisterHandler)
		{
			xlang2::Framework framework;
			Registrar<int> actor(framework);
		}

		UNITTEST_TEST(SendHandledMessage)
		{
			xlang2::Framework framework;
			xlang2::Receiver receiver;
			Replier<int> actor(framework);

			framework.Send(int(0), receiver.GetAddress(), actor.GetAddress());

			receiver.Wait();
		}

		UNITTEST_TEST(CreateActorInFunction)
		{
			xlang2::Framework framework;
			TrivialActor actor(framework);
		}

		UNITTEST_TEST(SendMessageToReceiverInFunction)
		{
			xlang2::Framework framework;
			xlang2::Receiver receiver;
			framework.Send(0.0f, receiver.GetAddress(), receiver.GetAddress());
			receiver.Wait();
		}

		UNITTEST_TEST(SendMessageFromNullAddressInFunction)
		{
			xlang2::Framework framework;
			xlang2::Receiver receiver;

			framework.Send(0, xlang2::Address::Null(), receiver.GetAddress());
			receiver.Wait();
		}

		UNITTEST_TEST(SendMessageToActorFromNullAddressInFunction)
		{
			xlang2::Framework framework;
			xlang2::Receiver receiver;
			Signaller signaller(framework);

			framework.Send(receiver.GetAddress(), xlang2::Address::Null(), signaller.GetAddress());
			receiver.Wait();
		}

		UNITTEST_TEST(SendMessageToActorFromReceiverInFunction)
		{
			xlang2::Framework framework;
			xlang2::Receiver receiver;
			Signaller signaller(framework);

			framework.Send(receiver.GetAddress(), receiver.GetAddress(), signaller.GetAddress());
			receiver.Wait();
		}

		UNITTEST_TEST(PushMessageToActorFromNullAddressInFunction)
		{
			xlang2::Framework framework;
			xlang2::Receiver receiver;
			Signaller signaler(framework);

			signaler.Push(receiver.GetAddress(), xlang2::Address::Null());
			receiver.Wait();
		}

		UNITTEST_TEST(PushMessageToActorFromReceiverInFunction)
		{
			xlang2::Framework framework;
			xlang2::Receiver receiver;
			Signaller signaler(framework);

			signaler.Push(receiver.GetAddress(), receiver.GetAddress());
			receiver.Wait();
		}

		UNITTEST_TEST(ReceiveReplyInFunction)
		{
			typedef Replier<float> FloatReplier;

			xlang2::Framework framework;
			xlang2::Receiver receiver;
			FloatReplier actor(framework);

			framework.Send(5.0f, receiver.GetAddress(), actor.GetAddress());

			receiver.Wait();
		}

		UNITTEST_TEST(CatchReplyInFunction)
		{
			typedef Replier<float> FloatReplier;
			typedef Catcher<float> FloatCatcher;

			xlang2::Framework framework;
			FloatReplier actor(framework);

			xlang2::Receiver receiver;
			FloatCatcher catcher;
			receiver.RegisterHandler(&catcher, &FloatCatcher::Catch);

			framework.Send(5.0f, receiver.GetAddress(), actor.GetAddress());

			receiver.Wait();

			CHECK_TRUE_T(catcher.mMessage == 5.0f, "Caught message value wrong");
			CHECK_TRUE_T(catcher.mFrom == actor.GetAddress(), "Caught from address wrong");
		}

		UNITTEST_TEST(SendNonPODMessageInFunction)
		{
			
			typedef Replier<VectorMessage> VectorReplier;
			typedef Catcher<VectorMessage> VectorCatcher;

			xlang2::Framework framework;
			VectorReplier actor(framework);

			xlang2::Receiver receiver;
			VectorCatcher catcher;
			receiver.RegisterHandler(&catcher, &VectorCatcher::Catch);

			VectorMessage vectorMessage;
			vectorMessage.push_back(0);
			vectorMessage.push_back(1);
			vectorMessage.push_back(2);
			framework.Send(vectorMessage, receiver.GetAddress(), actor.GetAddress());

			receiver.Wait();

			CHECK_TRUE_T(catcher.mMessage == vectorMessage, "Reply message is wrong");
		}

		UNITTEST_TEST(SendPointerMessageInFunction)
		{
			typedef Replier<fPointerMessage> PointerReplier;
			typedef Catcher<fPointerMessage> PointerCatcher;

			xlang2::Framework framework;
			PointerReplier actor(framework);

			xlang2::Receiver receiver;
			PointerCatcher catcher;
			receiver.RegisterHandler(&catcher, &PointerCatcher::Catch);

			float a(0.0f);
			fPointerMessage pointerMessage(&a);
			framework.Send(pointerMessage, receiver.GetAddress(), actor.GetAddress());

			receiver.Wait();

			CHECK_TRUE_T(catcher.mMessage == &a, "Reply message is wrong");
		}

		UNITTEST_TEST(SendConstPointerMessageInFunction)
		{
			typedef Replier<cfPointerMessage> PointerReplier;
			typedef Catcher<cfPointerMessage> PointerCatcher;

			xlang2::Framework framework;
			PointerReplier actor(framework);

			xlang2::Receiver receiver;
			PointerCatcher catcher;
			receiver.RegisterHandler(&catcher, &PointerCatcher::Catch);

			float a(0.0f);
			cfPointerMessage pointerMessage(&a);
			framework.Send(pointerMessage, receiver.GetAddress(), actor.GetAddress());

			receiver.Wait();

			CHECK_TRUE_T(catcher.mMessage == &a, "Reply message is wrong");
		}

		UNITTEST_TEST(CreateDerivedActor)
		{
			xlang2::Framework framework;
			StringReplier actor(framework);
		}

		UNITTEST_TEST(SendMessageToDerivedActor)
		{
			typedef Catcher<StringMessage> StringCatcher;

			xlang2::Framework framework;
			StringReplier actor(framework);
    
			xlang2::Receiver receiver;
			StringCatcher catcher;
			receiver.RegisterHandler(&catcher, &StringCatcher::Catch);
    
			const char *testString = "hello";
			StringMessage stringMessage(testString);
			framework.Send(stringMessage, receiver.GetAddress(), actor.GetAddress());

			receiver.Wait();

			CHECK_TRUE_T(catcher.mMessage == stringMessage, "Reply message is wrong");
		}

		UNITTEST_TEST(IncrementCounter)
		{
			typedef Catcher<int> CountCatcher;

			xlang2::Framework framework;
			Counter actor(framework);

			xlang2::Receiver receiver;
			CountCatcher catcher;
			receiver.RegisterHandler(&catcher, &CountCatcher::Catch);

			framework.Send(1, receiver.GetAddress(), actor.GetAddress());
			framework.Send(2, receiver.GetAddress(), actor.GetAddress());
			framework.Send(3, receiver.GetAddress(), actor.GetAddress());
			framework.Send(4, receiver.GetAddress(), actor.GetAddress());
			framework.Send(5, receiver.GetAddress(), actor.GetAddress());
			framework.Send(6, receiver.GetAddress(), actor.GetAddress());

			framework.Send(true, receiver.GetAddress(), actor.GetAddress());

			receiver.Wait();

			CHECK_TRUE_T(catcher.mMessage == 21, "Count is wrong");
		}

		UNITTEST_TEST(ActorTemplate)
		{
			typedef Replier<int> IntReplier;

			xlang2::Framework framework;
			IntReplier actor(framework);

			xlang2::Receiver receiver;
			framework.Send(10, receiver.GetAddress(), actor.GetAddress());

			receiver.Wait();
		}

		UNITTEST_TEST(OneHandlerAtATime)
		{
			typedef Catcher<int> CountCatcher;

			xlang2::Framework framework;
			TwoHandlerCounter actor(framework);

			xlang2::Receiver receiver;
			CountCatcher catcher;
			receiver.RegisterHandler(&catcher, &CountCatcher::Catch);

			framework.Send(2, receiver.GetAddress(), actor.GetAddress());
			framework.Send(0.0f, receiver.GetAddress(), actor.GetAddress());
			framework.Send(2, receiver.GetAddress(), actor.GetAddress());
			framework.Send(0.0f, receiver.GetAddress(), actor.GetAddress());
			framework.Send(2, receiver.GetAddress(), actor.GetAddress());
			framework.Send(0.0f, receiver.GetAddress(), actor.GetAddress());

			// Get the counter value.
			framework.Send(true, receiver.GetAddress(), actor.GetAddress());

			receiver.Wait();

			CHECK_TRUE_T(catcher.mMessage == 9, "Count is wrong");
		}

		UNITTEST_TEST(MultipleHandlersForMessageType)
		{
			typedef Catcher<int> CountCatcher;

			xlang2::Framework framework;
			MultipleHandlerCounter actor(framework);

			xlang2::Receiver receiver;
			CountCatcher catcher;
			receiver.RegisterHandler(&catcher, &CountCatcher::Catch);

			framework.Send(2, receiver.GetAddress(), actor.GetAddress());
			framework.Send(2, receiver.GetAddress(), actor.GetAddress());
			framework.Send(2, receiver.GetAddress(), actor.GetAddress());

			// Get the counter value.
			framework.Send(true, receiver.GetAddress(), actor.GetAddress());

			receiver.Wait();

			CHECK_TRUE_T(catcher.mMessage == 9, "Count is wrong");
		}

		UNITTEST_TEST(MessageArrivalOrder)
		{
			typedef Catcher<const char *> StringCatcher;
			typedef Sequencer<int> IntSequencer;

			xlang2::Framework framework;
			IntSequencer actor(framework);

			xlang2::Receiver receiver;
			StringCatcher catcher;
			receiver.RegisterHandler(&catcher, &StringCatcher::Catch);

			framework.Send(0, receiver.GetAddress(), actor.GetAddress());
			framework.Send(1, receiver.GetAddress(), actor.GetAddress());
			framework.Send(2, receiver.GetAddress(), actor.GetAddress());
			framework.Send(3, receiver.GetAddress(), actor.GetAddress());
			framework.Send(4, receiver.GetAddress(), actor.GetAddress());
			framework.Send(5, receiver.GetAddress(), actor.GetAddress());
			framework.Send(6, receiver.GetAddress(), actor.GetAddress());
			framework.Send(7, receiver.GetAddress(), actor.GetAddress());

			// Get the validity value.
			framework.Send(true, receiver.GetAddress(), actor.GetAddress());

			receiver.Wait();
			CHECK_TRUE_T(catcher.mMessage == IntSequencer::GOOD, "Sequencer status is wrong");

			framework.Send(9, receiver.GetAddress(), actor.GetAddress());

			// Get the validity value.
			framework.Send(true, receiver.GetAddress(), actor.GetAddress());

			receiver.Wait();
			CHECK_TRUE_T(catcher.mMessage == IntSequencer::BAD, "Sequencer status is wrong");
		}

		UNITTEST_TEST(SendAddressAsMessage)
		{
			typedef Catcher<xlang2::Address> AddressCatcher;

			xlang2::Framework framework;
			Signaller actorA(framework);
			Signaller actorB(framework);

			xlang2::Receiver receiver;
			AddressCatcher catcher;
			receiver.RegisterHandler(&catcher, &AddressCatcher::Catch);

			// Send A a message telling it to signal B.
			// A sends the receiver address to B as the signal,
			// causing B to send A's address to the receiver in turn.
			framework.Send(actorB.GetAddress(), receiver.GetAddress(), actorA.GetAddress());

			receiver.Wait();

			CHECK_TRUE_T(catcher.mMessage == actorA.GetAddress(), "Wrong address");
		}

		UNITTEST_TEST(SendMessageToDefaultHandlerInFunction)
		{
			typedef DefaultReplier<float> FloatReplier;
			typedef Catcher<std::string> StringCatcher;

			xlang2::Framework framework;
			FloatReplier actor(framework);

			StringCatcher catcher;
			xlang2::Receiver receiver;
			receiver.RegisterHandler(&catcher, &StringCatcher::Catch);

			// Send an int to the replier, which expects floats but has a default handler.
			framework.Send(52, receiver.GetAddress(), actor.GetAddress());

			receiver.Wait();

			CHECK_TRUE_T(catcher.mMessage == "hello", "Default handler not executed");
		}

		UNITTEST_TEST(RegisterHandlerFromHandler)
		{
			typedef Catcher<std::string> StringCatcher;

			xlang2::Framework framework;
			Switcher actor(framework);

			StringCatcher catcher;
			xlang2::Receiver receiver;
			receiver.RegisterHandler(&catcher, &StringCatcher::Catch);

			framework.Send(std::string("hello"), receiver.GetAddress(), actor.GetAddress());
			receiver.Wait();
			CHECK_TRUE_T(catcher.mMessage == "hello", "Handler not executed");

			framework.Send(std::string("hello"), receiver.GetAddress(), actor.GetAddress());
			receiver.Wait();
			CHECK_TRUE_T(catcher.mMessage == "goodbye", "Handler not executed");

			framework.Send(std::string("hello"), receiver.GetAddress(), actor.GetAddress());
			receiver.Wait();
			CHECK_TRUE_T(catcher.mMessage == "hello", "Handler not executed");

			framework.Send(std::string("hello"), receiver.GetAddress(), actor.GetAddress());
			receiver.Wait();
			CHECK_TRUE_T(catcher.mMessage == "goodbye", "Handler not executed");
		}

		UNITTEST_TEST(CreateActorInConstructor)
		{
			xlang2::Framework framework;

			Recursor::Parameters params;
			params.mCount = 10;
			Recursor actor(framework, params);
		}

		UNITTEST_TEST(SendMessageInConstructor)
		{
			xlang2::Framework framework;
			xlang2::Receiver receiver;

			// Pass the address of the receiver to the actor constructor.
			AutoSender::Parameters params(receiver.GetAddress());
			AutoSender actor(framework, params);

			// Wait for the messages sent by the actor on construction.
			receiver.Wait();
			receiver.Wait();
		}

		UNITTEST_TEST(DeregisterHandlerInConstructor)
		{
			typedef Catcher<int> IntCatcher;

			xlang2::Framework framework;

			xlang2::Receiver receiver;
			IntCatcher catcher;
			receiver.RegisterHandler(&catcher, &IntCatcher::Catch);

			AutoDeregistrar actor(framework);

			// Send the actor a message and check that the first handler doesn't send us a reply.
			framework.Send(0, receiver.GetAddress(), actor.GetAddress());

			receiver.Wait();
			CHECK_TRUE_T(catcher.mMessage == 2, "Received wrong message");
			CHECK_TRUE_T(receiver.Count() == 0, "Received too many messages");
		}

		UNITTEST_TEST(CreateActorInDestructor)
		{
			xlang2::Framework framework;

			TailRecursor::Parameters params;
			params.mCount = 10;
			TailRecursor actor(framework, params);
		}

		UNITTEST_TEST(SendMessageInDestructor)
		{
			xlang2::Framework framework;
			xlang2::Receiver receiver;

			{
				// Pass the address of the receiver ton the actor constructor.
				TailSender::Parameters params(receiver.GetAddress());
				TailSender actor(framework, params);
			}

			// Wait for the messages sent by the actor on destruction.
			receiver.Wait();
			receiver.Wait();
		}

		UNITTEST_TEST(DeregisterHandlerInDestructor)
		{
			// We check that it's safe to deregister a handler in an actor destructor,
			// but since it can't handle messages after destruction, there's little effect.
			xlang2::Framework framework;
			TailDeregistrar actor(framework);
		}

		UNITTEST_TEST(CreateActorInHandler)
		{
			xlang2::Framework framework;
			xlang2::Receiver receiver;
			Nestor actor(framework);

			framework.Send(Nestor::CreateMessage(), receiver.GetAddress(), actor.GetAddress());
			framework.Send(Nestor::DestroyMessage(), receiver.GetAddress(), actor.GetAddress());

			receiver.Wait();
		}

		UNITTEST_TEST(GetNumQueuedMessagesInHandler)
		{
			typedef Catcher<xlang2::uint32_t> CountCatcher;

			xlang2::Framework framework;
			xlang2::Receiver receiver;

			CountCatcher catcher;
			receiver.RegisterHandler(&catcher, &CountCatcher::Catch);
        
			MessageQueueCounter actor(framework);

			// Send the actor two messages.
			framework.Send(0, receiver.GetAddress(), actor.GetAddress());
			framework.Send(0, receiver.GetAddress(), actor.GetAddress());

			// Wait for and check both replies.
			// Race condition decides whether the second message has already arrived.
			// In Theron 4 the count includes the message currently being processed.
			receiver.Wait();
			CHECK_TRUE_T(catcher.mMessage == 1 || catcher.mMessage == 2, "GetNumQueuedMessages failed");

			receiver.Wait();
			CHECK_TRUE_T(catcher.mMessage == 1, "GetNumQueuedMessages failed");
		}

		UNITTEST_TEST(GetNumQueuedMessagesInFunction)
		{
			typedef const char * StringMessage;
			typedef Catcher<xlang2::uint32_t> CountCatcher;

			xlang2::Framework framework;
			xlang2::Receiver receiver;

			StringReplier actor(framework);

			// Send the actor two messages.
			StringMessage stringMessage("hello");
			framework.Send(stringMessage, receiver.GetAddress(), actor.GetAddress());
			framework.Send(stringMessage, receiver.GetAddress(), actor.GetAddress());

			// Race conditions decide how many messages are in the queue when we ask.
			// In Theron 4 the count includes the message currently being processed.
			xlang2::uint32_t numMessages(actor.GetNumQueuedMessages());
			CHECK_TRUE_T(numMessages < 3, "GetNumQueuedMessages failed, expected less than 3 messages");

			receiver.Wait();

			// There's no guarantee that the message handler will finish before the Wait() call returns.
			numMessages = actor.GetNumQueuedMessages();
			CHECK_TRUE_T(numMessages < 3, "GetNumQueuedMessages failed, expected less than 3 messages");

			receiver.Wait();

			numMessages = actor.GetNumQueuedMessages();
			CHECK_TRUE_T(numMessages < 2, "GetNumQueuedMessages failed, expected less than 2 messages");
		}

		UNITTEST_TEST(UseBlindDefaultHandler)
		{
			typedef Accumulator<xlang2::uint32_t> UIntAccumulator;

			xlang2::Framework framework;
			xlang2::Receiver receiver;

			UIntAccumulator accumulator;
			receiver.RegisterHandler(&accumulator, &UIntAccumulator::Catch);
        
			BlindActor actor(framework);

			// Send the actor a uint32_t message, which is the type it secretly expects.
			framework.Send(xlang2::uint32_t(75), receiver.GetAddress(), actor.GetAddress());

			// The actor sends back the value of the message data and the size.
			receiver.Wait();
			receiver.Wait();

			CHECK_TRUE_T(accumulator.Pop() == 75, "Bad blind data");
			CHECK_TRUE_T(accumulator.Pop() == 4, "Bad blind data size");
		}

		UNITTEST_TEST(IsHandlerRegisteredInHandler)
		{
			typedef Accumulator<bool> BoolAccumulator;

			xlang2::Framework framework;
			xlang2::Receiver receiver;

			BoolAccumulator accumulator;
			receiver.RegisterHandler(&accumulator, &BoolAccumulator::Catch);
        
			HandlerChecker actor(framework);
			framework.Send(int(0), receiver.GetAddress(), actor.GetAddress());

			xlang2::uint32_t count(21);
			while (count)
			{
				count -= receiver.Wait(count);
			}

			CHECK_TRUE_T(accumulator.Pop() == true, "Bad registration check result 0");
			CHECK_TRUE_T(accumulator.Pop() == false, "Bad registration check result 1");
			CHECK_TRUE_T(accumulator.Pop() == false, "Bad registration check result 2");

			//RegisterHandler(this, &HandlerChecker::Dummy);
			CHECK_TRUE_T(accumulator.Pop() == true, "Bad registration check result 3");
			CHECK_TRUE_T(accumulator.Pop() == true, "Bad registration check result 4");
			CHECK_TRUE_T(accumulator.Pop() == false, "Bad registration check result 5");

			// DeregisterHandler(this, &HandlerChecker::Dummy);
			CHECK_TRUE_T(accumulator.Pop() == true, "Bad registration check result 6");
			CHECK_TRUE_T(accumulator.Pop() == false, "Bad registration check result 7");
			CHECK_TRUE_T(accumulator.Pop() == false, "Bad registration check result 8");

			// DeregisterHandler(this, &HandlerChecker::Check);
			CHECK_TRUE_T(accumulator.Pop() == false, "Bad registration check result 9");
			CHECK_TRUE_T(accumulator.Pop() == false, "Bad registration check result 10");
			CHECK_TRUE_T(accumulator.Pop() == false, "Bad registration check result 11");

			// RegisterHandler(this, &HandlerChecker::Dummy);
			// RegisterHandler(this, &HandlerChecker::Check);
			// RegisterHandler(this, &HandlerChecker::Check);
			CHECK_TRUE_T(accumulator.Pop() == true, "Bad registration check result 12");
			CHECK_TRUE_T(accumulator.Pop() == true, "Bad registration check result 13");
			CHECK_TRUE_T(accumulator.Pop() == false, "Bad registration check result 14");

			// DeregisterHandler(this, &HandlerChecker::Check);
			CHECK_TRUE_T(accumulator.Pop() == true, "Bad registration check result 15");
			CHECK_TRUE_T(accumulator.Pop() == true, "Bad registration check result 16");
			CHECK_TRUE_T(accumulator.Pop() == false, "Bad registration check result 17");

			// DeregisterHandler(this, &HandlerChecker::Check);
			CHECK_TRUE_T(accumulator.Pop() == false, "Bad registration check result 18");
			CHECK_TRUE_T(accumulator.Pop() == true, "Bad registration check result 19");
			CHECK_TRUE_T(accumulator.Pop() == false, "Bad registration check result 20");
		}

		UNITTEST_TEST(SetFallbackHandler)
		{
			xlang2::Framework framework;
			FallbackHandler fallbackHandler;

			CHECK_TRUE_T(framework.SetFallbackHandler(&fallbackHandler, &FallbackHandler::Handle), "Register failed");
		}

		UNITTEST_TEST(HandleUndeliveredMessageSentInFunction)
		{
			typedef Replier<float> FloatReplier;

			xlang2::Framework framework;
			xlang2::Receiver receiver;

			FallbackHandler fallbackHandler;
			framework.SetFallbackHandler(&fallbackHandler, &FallbackHandler::Handle);

			// Create an actor and let it die but remember its address.
			xlang2::Address staleAddress;

			{
				FloatReplier actor(framework);
				staleAddress = actor.GetAddress();
			}

			// Send a message to the stale address.
			framework.Send(0, receiver.GetAddress(), staleAddress);

			// Wait for the undelivered message to be caught by the registered fallback handler.
			xlang2::uint32_t backoff(0);
			while (fallbackHandler.mAddress != receiver.GetAddress())
			{
				xlang2::Detail::Utils::Backoff(backoff);
			}
		}

		UNITTEST_TEST(HandleUnhandledMessageSentInFunction)
		{
			typedef Replier<xlang2::uint32_t> UIntReplier;

			xlang2::Framework framework;
			xlang2::Receiver receiver;

			FallbackHandler fallbackHandler;
			framework.SetFallbackHandler(&fallbackHandler, &FallbackHandler::Handle);

			// Create a replier that handles only ints, then send it a float.
			UIntReplier replier(framework);
			framework.Send(5.0f, receiver.GetAddress(), replier.GetAddress());

			// Wait for the unhandled message to be caught by the registered fallback handler.
			xlang2::uint32_t backoff(0);
			while (fallbackHandler.mAddress != receiver.GetAddress())
			{
				xlang2::Detail::Utils::Backoff(backoff);
			}
		}

		UNITTEST_TEST(HandleUndeliveredBlindMessageSentInFunction)
		{
			typedef Replier<float> FloatReplier;

			xlang2::Framework framework;
			xlang2::Receiver receiver;

			BlindFallbackHandler fallbackHandler;
			fallbackHandler.mData = 0;
			framework.SetFallbackHandler(&fallbackHandler, &BlindFallbackHandler::Handle);

			// Create an actor and let it die but remember its address.
			xlang2::Address staleAddress;

			{
				FloatReplier actor(framework);
				staleAddress = actor.GetAddress();
			}

			// Send a message to the stale address.
			framework.Send(xlang2::uint32_t(42), receiver.GetAddress(), staleAddress);

			// Wait for the undelivered message to be caught by the registered fallback handler.
			xlang2::uint32_t backoff(0);
			while (fallbackHandler.mData == 0)
			{
				xlang2::Detail::Utils::Backoff(backoff);
			}

			// Check that the undelivered message was handled by the registered fallback handler.
			CHECK_TRUE_T(fallbackHandler.mValue == 42, "Blind fallback handler failed");
			CHECK_TRUE_T(fallbackHandler.mSize == sizeof(xlang2::uint32_t), "Blind fallback handler failed");
			CHECK_TRUE_T(fallbackHandler.mAddress == receiver.GetAddress(), "Blind fallback handler failed");
		}

		UNITTEST_TEST(HandleMessageSentToStaleFrameworkInFunction)
		{
			typedef Replier<float> FloatReplier;

			xlang2::Framework framework0;
			xlang2::Receiver receiver;

			// Register a safe fallback handler in the first framework.
			FallbackHandler fallbackHandler;
			framework0.SetFallbackHandler(&fallbackHandler, &FallbackHandler::Handle);

			// Create an actor in the first framework.
			FloatReplier replier0(framework0);

			// Create an actor in a second framework, then let the framework die but remember the address.
			xlang2::Address staleAddress;

			{
				xlang2::Framework framework1;
				FloatReplier replier1(framework1);
				staleAddress = replier1.GetAddress();
			}

			// Send a message from the first framework to the stale address in the second framework.
			framework0.Send(0.0f, receiver.GetAddress(), staleAddress);

			// Wait for the undelivered message to be caught by the registered fallback handler.
			xlang2::uint32_t backoff(0);
			while (fallbackHandler.mAddress != receiver.GetAddress())
			{
				xlang2::Detail::Utils::Backoff(backoff);
			}
		}

		UNITTEST_TEST(HandleMessageSentToStaleFrameworkInHandler)
		{
			typedef Replier<float> FloatReplier;

			xlang2::Framework framework0;
			xlang2::Receiver receiver;

			// Register a safe fallback handler in the first framework.
			FallbackHandler fallbackHandler;
			framework0.SetFallbackHandler(&fallbackHandler, &FallbackHandler::Handle);

			// Create an actor in the first framework to send a message to an actor in the second.
			// The signaller sends a message to the address it is sent.
			Signaller signaller(framework0);

			// Create an actor in a second framework, then let the framework die but remember the address.
			xlang2::Address staleAddress;

			{
				xlang2::Framework framework1;
				FloatReplier replier(framework1);
				staleAddress = replier.GetAddress();
			}

			// Send a message to the signaller telling it to send a message to the stale address.
			framework0.Send(staleAddress, receiver.GetAddress(), signaller.GetAddress());

			// Wait for the undelivered message to be caught by the registered fallback handler.
			// The undelivered message was sent by the signaller.
			xlang2::uint32_t backoff(0);
			while (fallbackHandler.mAddress != signaller.GetAddress())
			{
				xlang2::Detail::Utils::Backoff(backoff);
			}
		}

		UNITTEST_TEST(SendRegisteredMessage)
		{
			typedef Replier<VectorMessage> IntVectorReplier;
			typedef Catcher<VectorMessage> IntVectorCatcher;

			xlang2::Framework framework;

			xlang2::Receiver receiver;
			IntVectorCatcher catcher;
			receiver.RegisterHandler(&catcher, &IntVectorCatcher::Catch);

			IntVectorReplier replier(framework);

			VectorMessage message;
			message.push_back(0);
			message.push_back(1);
			message.push_back(2);

			framework.Send(message, receiver.GetAddress(), replier.GetAddress());
			receiver.Wait();

			CHECK_TRUE_T(catcher.mMessage.size() == 3, "Bad reply message");
			CHECK_TRUE_T(catcher.mMessage[0] == 0, "Bad reply message");
			CHECK_TRUE_T(catcher.mMessage[1] == 1, "Bad reply message");
			CHECK_TRUE_T(catcher.mMessage[2] == 2, "Bad reply message");
		}

		UNITTEST_TEST(DeriveFromActorFirst)
		{
			typedef Catcher<int> IntCatcher;

			xlang2::Framework framework;

			xlang2::Receiver receiver;
			IntCatcher catcher;
			receiver.RegisterHandler(&catcher, &IntCatcher::Catch);

			ActorFirst actor(framework);

			framework.Send(5, receiver.GetAddress(), actor.GetAddress());
			receiver.Wait();

			CHECK_TRUE_T(catcher.mMessage == 5, "Bad reply message");
		}

		UNITTEST_TEST(DeriveFromActorLast)
		{
			typedef Catcher<int> IntCatcher;

			xlang2::Framework framework;

			xlang2::Receiver receiver;
			IntCatcher catcher;
			receiver.RegisterHandler(&catcher, &IntCatcher::Catch);

			ActorLast actor(framework);

			framework.Send(5, receiver.GetAddress(), actor.GetAddress());
			receiver.Wait();

			CHECK_TRUE_T(catcher.mMessage == 5, "Bad reply message");
		}

		UNITTEST_TEST(SendEmptyMessage)
		{
			typedef Replier<EmptyMessage> EmptyReplier;
			typedef Catcher<EmptyMessage> EmptyCatcher;

			xlang2::Framework framework;

			xlang2::Receiver receiver;
			EmptyCatcher catcher;
			receiver.RegisterHandler(&catcher, &EmptyCatcher::Catch);

			EmptyReplier replier(framework);

			framework.Send(EmptyMessage(), receiver.GetAddress(), replier.GetAddress());
			receiver.Wait();

			CHECK_TRUE_T(&catcher.mMessage != 0, "No reply message");
		}

		UNITTEST_TEST(MultipleFrameworks)
		{
			typedef Catcher<int> IntCatcher;

			xlang2::Receiver receiver;
			IntCatcher catcher;
			receiver.RegisterHandler(&catcher, &IntCatcher::Catch);

			xlang2::Framework framework0;
			xlang2::Framework framework1;
			xlang2::Framework framework2;
			xlang2::Framework framework3;
			xlang2::Framework framework4;

			Forwarder actor0(framework0, receiver.GetAddress());
			Forwarder actor1(framework1, actor0.GetAddress());
			Forwarder actor2(framework2, actor1.GetAddress());
			Forwarder actor3(framework3, actor2.GetAddress());
			Forwarder actor4(framework4, actor3.GetAddress());

			framework0.Send(int(5), receiver.GetAddress(), actor4.GetAddress());

			receiver.Wait();
			CHECK_TRUE_T(catcher.mMessage == 0, "Received wrong message");
			CHECK_TRUE_T(receiver.Count() == 0, "Received too many messages");
		}

		UNITTEST_TEST(ConstructFrameworkWithParameters)
		{
			typedef Replier<int> IntReplier;

			const xlang2::Framework::Parameters params0;
			xlang2::Framework framework0(params0);
			xlang2::Framework framework1(xlang2::Framework::Parameters(8));
			xlang2::Framework framework2(xlang2::Framework::Parameters(12, 0));

			IntReplier replier0(framework0);
			IntReplier replier1(framework1);
			IntReplier replier2(framework2);

			xlang2::Receiver receiver;
			framework0.Send(int(2), receiver.GetAddress(), replier0.GetAddress());
			framework1.Send(int(2), receiver.GetAddress(), replier1.GetAddress());
			framework2.Send(int(2), receiver.GetAddress(), replier2.GetAddress());

			receiver.Wait();
			receiver.Wait();
			receiver.Wait();
		}

		UNITTEST_TEST(ThreadCountApi)
		{
			xlang2::Framework framework;

			xlang2::Detail::Utils::SleepThread(10);

			// Create more worker threads.
			framework.SetMinThreads(32);
			CHECK_TRUE_T(framework.GetMinThreads() >= 32, "GetMinThreads failed");

			xlang2::Detail::Utils::SleepThread(10);

			// Stop most of the threads.
			framework.SetMaxThreads(8);
			CHECK_TRUE_T(framework.GetMaxThreads() <= 8, "GetMaxThreads failed");

			xlang2::Detail::Utils::SleepThread(10);

			// Re-start all of the threads and create some more.
			framework.SetMinThreads(64);
			CHECK_TRUE_T(framework.GetMinThreads() >= 64, "GetMinThreads failed");

			xlang2::Detail::Utils::SleepThread(10);

			// Stop all threads but one.
			framework.SetMaxThreads(1);
			CHECK_TRUE_T(framework.GetMaxThreads() <= 1, "GetMaxThreads failed");

			xlang2::Detail::Utils::SleepThread(10);

			CHECK_TRUE_T(framework.GetPeakThreads() >= 1, "GetPeakThreads failed");
			CHECK_TRUE_T(framework.GetNumThreads() >= 1, "GetNumThreads failed");
			CHECK_TRUE_T(framework.GetPeakThreads() >= framework.GetNumThreads(), "GetPeakThreads failed");
		}

		UNITTEST_TEST(EventCounterApi)
		{
			typedef Replier<int> IntReplier;

			xlang2::Framework framework;
			xlang2::Receiver receiver;

			xlang2::Detail::Utils::SleepThread(10);

			// Check initial values.
			CHECK_TRUE_T(framework.GetCounterValue(xlang2::COUNTER_MESSAGES_PROCESSED) == 0, "GetCounterValue failed");

			xlang2::uint32_t counterValues[32];
			xlang2::uint32_t valueCount(framework.GetPerThreadCounterValues(xlang2::COUNTER_MESSAGES_PROCESSED, counterValues, 32));

			xlang2::uint32_t messagesProcessed(0);
			for (xlang2::uint32_t index = 0; index < valueCount; ++index)
			{
				messagesProcessed += counterValues[index];
			}

			CHECK_TRUE_T(messagesProcessed == 0, "GetPerThreadCounterValues failed");

			IntReplier replier(framework);
			framework.Send(int(0), receiver.GetAddress(), replier.GetAddress());
			receiver.Wait();

			// Check values after some work.
			CHECK_TRUE_T(framework.GetCounterValue(xlang2::COUNTER_MESSAGES_PROCESSED) > 0, "GetCounterValue failed");

			valueCount = framework.GetPerThreadCounterValues(xlang2::COUNTER_MESSAGES_PROCESSED, counterValues, 32);

			messagesProcessed = 0;
			for (xlang2::uint32_t index = 0; index < valueCount; ++index)
			{
				messagesProcessed += counterValues[index];
			}

			CHECK_TRUE_T(messagesProcessed == 1, "GetPerThreadCounterValues failed");

			// Check values after reset.
			framework.ResetCounters();

			CHECK_TRUE_T(framework.GetCounterValue(xlang2::COUNTER_MESSAGES_PROCESSED) == 0, "GetCounterValue failed");

			valueCount = framework.GetPerThreadCounterValues(xlang2::COUNTER_MESSAGES_PROCESSED, counterValues, 32);

			messagesProcessed = 0;
			for (xlang2::uint32_t index = 0; index < valueCount; ++index)
			{
				messagesProcessed += counterValues[index];
			}

			CHECK_TRUE_T(messagesProcessed == 0, "GetPerThreadCounterValues failed");
		}

	}
}
UNITTEST_SUITE_END



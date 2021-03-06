#include "xlang2\x_defines.h"

// Must include xs.h before standard headers to avoid warnings in MS headers!
#if THERON_XS
#include <xs/xs.h>
#endif // THERON_XS

#include "xlang2\x_receiver.h"

#include "xlang2\private\directory\x_staticdirectory.h"
#include "xlang2\private\network\x_index.h"
#include "xlang2\private\network\x_namegenerator.h"
#include "xlang2\private\network\x_string.h"


namespace xlang2
{
	Receiver::Receiver()
		: mName()
		, mAddress()
		, mMessageHandlers()
		, mSpinLock()
		, mMessagesReceived(0)
	{
		Initialize();
	}

	Receiver::~Receiver()
	{
		Release();
	}


	void Receiver::Initialize()
	{
		// Register this receiver, claiming a unique address for this receiver.
		const uint32_t receiverIndex(Detail::StaticDirectory<Receiver>::Register(this));

		if (mName.IsNull())
		{
			char rawName[16];
			Detail::NameGenerator::Generate(rawName, receiverIndex);

			char scopedName[256];
			Detail::NameGenerator::Combine(scopedName,256,rawName,0,"");

			//@TODO(Jurgen): Do we really have to create a string here? We know the length already so we
			//               could just use a fixed size array!
			mName = Detail::String(scopedName);
		}

		// Receivers are identified as a receiver by a framework index of zero.
		// All frameworks have non-zero indices, so all actors have non-zero framework indices.
		const Detail::Index index(0, receiverIndex);
		mAddress = Address(mName, index);

		// Register the receiver at its claimed address.
		Detail::Entry &entry(Detail::StaticDirectory<Receiver>::GetEntry(mAddress.AsInteger()));

		entry.Lock();
		entry.SetEntity(this);
		entry.Unlock();
	}


	void Receiver::Release()
	{
		const Address &address(GetAddress());

		// Deregister the receiver, so that the worker threads will leave it alone.
		Detail::StaticDirectory<Receiver>::Deregister(address.AsInteger());

		mSpinLock.Lock();

		// Free all currently allocated handler objects.
		while (Detail::IReceiverHandler *const handler = mMessageHandlers.Front())
		{
			mMessageHandlers.Remove(handler);
			AllocatorManager::Instance().GetAllocator()->Free(handler);
		}

		mSpinLock.Unlock();
	}


} // namespace xlang2



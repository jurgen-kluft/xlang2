#ifndef THERON_CATCHER_H
#define THERON_CATCHER_H

/**
\file Catcher.h
Utility that catches messages received by a Receiver.
*/

#include <new>

#include "xlang2\x_address.h"
#include "xlang2\x_align.h"
#include "xlang2\x_allocatormanager.h"
#include "xlang2\x_assert.h"
#include "xlang2\x_basictypes.h"
#include "xlang2\x_defines.h"

#include "xlang2\private\containers\x_queue.h"
#include "xlang2\private\threading\x_spinlock.h"

namespace xlang2
{
	/**
	\brief A simple utility class template that catches messages received by a \ref Receiver.

	\tparam MessageType The type of message caught by the catcher.
	*/
	template <class MessageType>
	class Catcher
	{
	public:

		/**
		\brief Default constructor.
		*/
		inline Catcher();

		/**
		\brief Destructor.
		*/
		inline ~Catcher();

		/**
		Returns true if the catcher contains no caught messages.
		*/
		inline bool Empty() const;

		/**
		\brief Pushes a caught message into the queue.

		This function has the signature of a message handler and so can be
		registered as a message handler with a \ref Receiver. Used in this way, it serves
		as a thread-safe FIFO buffer of messages received by the receiver.
		Use this helper to safely receive multiple, successive messages.

		\param message A message instance to be pushed onto the queue.
		\param from The address of the actor (or entity) that sent the message.

		\see Receiver::RegisterHandler
		*/
		inline void Push(const MessageType &message, const Address from);

		/**
		Gets the next caught message without removing it from the queue.

		\param message A message instance into which the caught message is copied.
		\param from An address into which the sender's address is copied.

		\return True, if a caught message was available.
		*/
		inline bool Front(MessageType &message, Address &from);

		/**
		Gets the next caught message, removing it from the queue.

		\param message A message instance into which the caught message is copied.
		\param from An address into which the sender's address is copied.

		\return True, if a caught message was available.
		*/
		inline bool Pop(MessageType &message, Address &from);

	private:

		/**
		\brief Wrapper struct that wraps a message in a queue node.

		If the message type requires non-default alignment then this struct should inherit it.
		*/
		struct Entry : public Detail::Queue<Entry>::Node
		{
			inline Entry(const MessageType &message, const Address from) : mMessage(message), mFrom(from)
			{
			}

			MessageType mMessage;               ///< The queued message.
			Address mFrom;                      ///< The address of the sender.
		};

		mutable Detail::SpinLock mSpinLock;     ///< Thread synchronization primitive.
		Detail::Queue<Entry> mQueue;            ///< Queue of caught messages.
	};


	template <class MessageType>
	inline Catcher<MessageType>::Catcher() : mSpinLock(), mQueue()
	{
	}


	template <class MessageType>
	inline Catcher<MessageType>::~Catcher()
	{
		IAllocator *const allocator(AllocatorManager::Instance().GetAllocator());

		mSpinLock.Lock();

		// Free any left-over entries on the queue.
		while (!mQueue.Empty())
		{
			Entry *const entry(mQueue.Pop());

			// Destruct the entry and free the memory.
			entry->~Entry();
			allocator->Free(entry, sizeof(Entry));
		}

		mSpinLock.Unlock();
	}


	template <class MessageType>
	THERON_FORCEINLINE bool Catcher<MessageType>::Empty() const
	{
		bool empty(false);

		mSpinLock.Lock();
		empty = mQueue.Empty();
		mSpinLock.Unlock();

		return empty;
	}


	template <class MessageType>
	inline void Catcher<MessageType>::Push(const MessageType &message, const Address from)
	{
		IAllocator *const allocator(AllocatorManager::Instance().GetAllocator());

		// Allocate an entry to hold the copy of the message in the queue, with correct alignment.
		const uint32_t entrySize(static_cast<uint32_t>(sizeof(Entry)));
		const uint32_t entryAlignment(static_cast<uint32_t>(THERON_ALIGNOF(Entry)));
		void *const memory(allocator->AllocateAligned(entrySize, entryAlignment));

		if (memory == 0)
		{
			return;
		}

		// Construct the entry in the allocated memory.
		Entry *const entry = new (memory) Entry(message, from);

		// Push the entry onto the queue, locking for thread-safety.
		mSpinLock.Lock();
		mQueue.Push(entry);
		mSpinLock.Unlock();
	}


	template <class MessageType>
	inline bool Catcher<MessageType>::Front(MessageType &message, Address &from)
	{
		Entry *entry(0);

		// Pop an entry off the queue, locking for thread-safety.
		mSpinLock.Lock();
		entry = mQueue.Front();
		mSpinLock.Unlock();

		if (entry)
		{
			// Copy the data from the entry.
			message = entry->mMessage;
			from = entry->mFrom;

			return true;
		}

		return false;
	}


	template <class MessageType>
	inline bool Catcher<MessageType>::Pop(MessageType &message, Address &from)
	{
		IAllocator *const allocator(AllocatorManager::Instance().GetAllocator());
		Entry *entry(0);

		// Pop an entry off the queue, locking for thread-safety.
		mSpinLock.Lock();
		entry = mQueue.Pop();
		mSpinLock.Unlock();

		if (entry)
		{
			// Copy the data from the entry.
			message = entry->mMessage;
			from = entry->mFrom;

			// Destruct the entry and free the memory.
			entry->~Entry();
			allocator->Free(entry, sizeof(Entry));

			return true;
		}

		return false;
	}


} // namespace xlang2


#endif // THERON_CATCHER_H

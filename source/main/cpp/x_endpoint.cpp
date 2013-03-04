#include "xlang2\x_defines.h"

// Must include xs.h before standard headers to avoid warnings in MS headers!
#if THERON_XS
#include <xs/xs.h>
#endif // THERON_XS

#include <new>

#include "xlang2\x_address.h"
#include "xlang2\x_allocatormanager.h"
#include "xlang2\x_assert.h"
#include "xlang2\x_endpoint.h"
#include "xlang2\x_iallocator.h"

#include "xlang2\private\messages\x_messagecreator.h"
#include "xlang2\private\messages\x_messagesender.h"
#include "xlang2\private\network\x_messagefactory.h"
#include "xlang2\private\network\x_networkmessage.h"
#include "xlang2\private\threading\x_utils.h"
#include "xlang2\private\transport\x_inputmessage.h"
#include "xlang2\private\transport\x_inputsocket.h"
#include "xlang2\private\transport\x_outputmessage.h"
#include "xlang2\private\transport\x_outputsocket.h"


namespace xlang2
{
	Detail::SpinLock EndPoint::smContextLock;
	uint32_t EndPoint::smContextRefCount(0);
	Detail::Context *EndPoint::smContext(0);

	Detail::Context *EndPoint::InitializeContext()
	{
		IAllocator *const allocator(AllocatorManager::Instance().GetAllocator());

		smContextLock.Lock();

		if (smContext == 0)
		{
			// Allocate and construct a context structure.
			void *const contextMemory(allocator->Allocate(sizeof(Detail::Context)));

			THERON_ASSERT_MSG(contextMemory, "Failed to allocate endPoint context object");
			smContext = new (contextMemory) Detail::Context();

			smContextRefCount = 0;
		}

		++smContextRefCount;

		smContextLock.Unlock();

		THERON_ASSERT(smContext);
		return smContext;
	}


	void EndPoint::ReleaseContext()
	{
		IAllocator *const allocator(AllocatorManager::Instance().GetAllocator());

		THERON_ASSERT(smContext);
		THERON_ASSERT(smContextRefCount);

		smContextLock.Lock();

		if (--smContextRefCount == 0)
		{
			smContext->~Context();
			allocator->Free(smContext, sizeof(Detail::Context));
			smContext = 0;
		}

		smContextLock.Unlock();
	}


	EndPoint::EndPoint(const char *const name, const char *const location, const Parameters /*params*/)
		: mName(name)
		, mLocation(location)
		, mContext(0)
		, mNameMap()
		, mMessageFactory()
		, mNetworkThread()
		, mRunning(false)
		, mStarted(false)
		, mNetworkLock()
		, mConnectQueue()
		, mSendQueue()
	{
		THERON_ASSERT_MSG(!mName.IsNull(), "Must supply a unique, non-null name for each endpoint");
		THERON_ASSERT_MSG(!mLocation.IsNull(), "Must supply a valid port address for each endpoint");

		// Initialize the per-process context.
		mContext = InitializeContext();

		// Don't start the network thread if networking isn't enabled.
		if (mContext->Enabled())
		{
			// Start the network thread.
			mRunning = true;
			mNetworkThread.Start(NetworkThreadEntryPoint, this);

			// Wait for the network thread to start.
			uint32_t backoff(0);
			while (!mStarted)
			{
				Detail::Utils::Backoff(backoff);
			}
		}
	}


	EndPoint::~EndPoint()
	{
		// No network thread if networking isn't enabled.
		if (mContext->Enabled())
		{
			// Kill the network thread and wait for it to terminate.
			mRunning = false;
			mNetworkThread.Join();
		}

		// Release the per-process context.
		ReleaseContext();
	}


	bool EndPoint::Connect(const char *const address)
	{
		IAllocator *const allocator(AllocatorManager::Instance().GetAllocator());

		if (address == 0)
		{
			return false;
		}

		// Fail if networking isn't enabled.
		if (!mContext->Enabled())
		{
			return false;
		}

		// Allocate and construct a request structure.
		void *const requestMemory(allocator->Allocate(sizeof(ConnectRequest)));
		if (requestMemory == 0)
		{
			return false;
		}

		ConnectRequest *const request = new (requestMemory) ConnectRequest(address);

		// Push it onto the queue to be serviced by the network thread.
		mNetworkLock.Lock();
		mConnectQueue.Push(request);
		mNetworkLock.Unlock();

		return true;
	}


	bool EndPoint::RequestSend(Detail::IMessage *const message, const Detail::String &name)
	{
		IAllocator *const allocator(AllocatorManager::Instance().GetAllocator());

		if (message == 0 || name.IsNull())
		{
			return false;
		}

		// Allocate and construct a request structure.
		void *const requestMemory(allocator->Allocate(sizeof(SendRequest)));
		if (requestMemory == 0)
		{
			return false;
		}

		SendRequest *const request = new (requestMemory) SendRequest(message, name);

		// Push it onto the queue to be serviced by the network thread.
		mNetworkLock.Lock();
		mSendQueue.Push(request);
		mNetworkLock.Unlock();

		return true;
	}


	void EndPoint::NetworkThreadEntryPoint(void *const context)
	{
		// The static entry point function is passed the object pointer as context.
		EndPoint *const endPoint(reinterpret_cast<EndPoint *>(context));
		endPoint->NetworkThreadProc();
	}


	void EndPoint::NetworkThreadProc()
	{
		IAllocator *const allocator(AllocatorManager::Instance().GetAllocator());

		THERON_ASSERT(mContext);
		THERON_ASSERT(!mLocation.IsNull());

		// Create the output socket.
		// This is the socket we use to publish messages sent by local actors.
		// EndPoint objects in other processors or on other hosts create SUB sockets
		// and connect them to the same endpoint.
		void *const outputSocketMemory(allocator->Allocate(sizeof(Detail::OutputSocket)));
		if (outputSocketMemory == 0)
		{
			THERON_FAIL_MSG("Failed to allocate output socket");
		}

		Detail::OutputSocket *const outputSocket = new (outputSocketMemory) Detail::OutputSocket(mContext);

		// Create the input socket.
		// This is the socket we use to subscribe to messages published by other actors.
		// We connect to the endpoints of the publish sockets of the remote processes.
		void *const inputSocketMemory(allocator->Allocate(sizeof(Detail::InputSocket)));
		if (inputSocketMemory == 0)
		{
			THERON_FAIL_MSG("Failed to allocate input socket");
		}

		Detail::InputSocket *const inputSocket = new (inputSocketMemory) Detail::InputSocket(mContext);

		// Bind the output socket to the local endpoint that is this EndPoint's address.
		if (!outputSocket->Bind(mLocation.GetValue()))
		{
			THERON_FAIL_MSG("Failed to bind output socket to endpoint - check network support is enabled");
		}

		// Create a network output message, which we reuse to send network messages within the loop.
		void *const outputMessageMemory(allocator->Allocate(sizeof(Detail::OutputMessage)));
		if (outputMessageMemory == 0)
		{
			THERON_FAIL_MSG("Failed to allocate output message");
		}

		Detail::OutputMessage *const outputMessage = new (outputMessageMemory) Detail::OutputMessage(mContext);

		// Create a network input message, which we reuse to receive network messages within the loop.
		void *const inputMessageMemory(allocator->Allocate(sizeof(Detail::InputMessage)));
		if (inputMessageMemory == 0)
		{
			THERON_FAIL_MSG("Failed to allocate input message");
		}

		Detail::InputMessage *const inputMessage = new (inputMessageMemory) Detail::InputMessage(mContext);

		// Initialize the input message once outside the loop.
		if (!inputMessage->Initialize())
		{
			THERON_FAIL_MSG("Failed to initialize network input message");
		}

		// Signal the main thread that we've started.
		mStarted = true;

		while (mRunning)
		{
			mNetworkLock.Lock();

			// Service connection requests.
			while (!mConnectQueue.Empty())
			{
				ConnectRequest *const request(mConnectQueue.Pop());
				const char *const address(request->mLocation.GetValue());

				mNetworkLock.Unlock();

				// Connect the input socket to the remote host.
				// The same socket can be connected to multiple remote endpoints.
				if (!inputSocket->Connect(address))
				{
					THERON_FAIL_MSG("Failed to connect input socket to remote endpoint");
				}

				mNetworkLock.Lock();

				request->~ConnectRequest();
				allocator->Free(request, sizeof(ConnectRequest));
			}

			// Service send requests.
			while (!mSendQueue.Empty())
			{
				SendRequest *const request(mSendQueue.Pop());

				mNetworkLock.Unlock();

				THERON_ASSERT_MSG(outputSocket, "No output socket found");

				Detail::IMessage *const message(request->mMessage);
				const char *const fromName(message->From().AsString());
				const char *const toName(request->mName.GetValue());
				const char *const messageName(message->TypeName());
				const void *const messageData(message->GetMessageData());
				const uint32_t messageSize(message->GetMessageSize());

				THERON_ASSERT_MSG(messageName, "Can't send unregistered messages to actors addressed by name!");

				// Calculate the size of the network message.
				const uint32_t networkMessageSize = Detail::NetworkMessage::GetSize(
					fromName,
					toName,
					messageName,
					messageData,
					messageSize);

				// Initialize the output message.
				if (!outputMessage->Initialize(networkMessageSize))
				{
					THERON_FAIL_MSG("Failed to initialize network output message");
				}

				// Initialize the XS message block as a network message.
				// This copies the message data into the memory block.
				void *const memoryBlock(outputMessage->Data());
				Detail::NetworkMessage::Initialize(
					memoryBlock,
					fromName,
					toName,
					messageName,
					messageData,
					messageSize);

				// Send the network message.
				if (!outputSocket->Send(outputMessage))
				{
					THERON_FAIL_MSG("Failed to send network output message");
				}

				// Release the output message.
				if (!outputMessage->Release())
				{
					THERON_FAIL_MSG("Failed to release network output message");
				}

				// Destroy the local message.
				// We return the message memory block to the global allocator.
				Detail::MessageCreator::Destroy(allocator, message);

				// Destroy the request structure.
				request->~SendRequest();
				allocator->Free(request, sizeof(SendRequest));

				mNetworkLock.Lock();
			}

			mNetworkLock.Unlock();

			// Read messages from the input socket without blocking.
			while (inputSocket->NonBlockingReceive(inputMessage))
			{
				// Read the data from the received network message.
				const uint32_t networkMessageSize(inputMessage->Size());
				if (networkMessageSize < sizeof(Detail::NetworkMessage))
				{
					THERON_FAIL_MSG("Invalid XS message data size");
				}

				const Detail::NetworkMessage *const networkMessage(reinterpret_cast<Detail::NetworkMessage *>(inputMessage->Data()));
				if (networkMessage == 0)
				{
					THERON_FAIL_MSG("Invalid XS message data pointer");
				}

				const char *const fromName(networkMessage->GetFromName());
				const Detail::String toName(networkMessage->GetToName());
				const Detail::String messageName(networkMessage->GetMessageName());
				const void *const messageData(networkMessage->GetMessageData());
				const uint32_t messageSize(networkMessage->GetMessageSize());

				// Look up the to address index using the received name.
				// If the name isn't known to us we just discard the message.
				Detail::Index toIndex;
				if (Lookup(toName, toIndex))
				{
					// Construct the name-only 'from' address.
					const Address fromAddress(fromName);

					// Try to build a message from the data based on the message name.
					Detail::IMessage *const message = mMessageFactory.Build(
						messageName,
						messageData,
						messageSize,
						fromAddress);

					if (message)
					{
						// Try to deliver the allocated message to an actor in a local framework.
						if (!Detail::MessageSender::DeliverWithinLocalProcess(message, toIndex))
						{
							// Destroy the undelivered message using the global allocator.
							Detail::MessageCreator::Destroy(allocator, message);
						}
					}
				}
			}

			// The network thread spends most of its time asleep.
			Detail::Utils::SleepThread(1);
		}

		mNetworkLock.Lock();

		// Drain the connection request queue.
		while (!mConnectQueue.Empty())
		{
			ConnectRequest *const request(mConnectQueue.Pop());
			request->~ConnectRequest();
			allocator->Free(request, sizeof(ConnectRequest));
		}

		// Drain the send request queue.
		while (!mSendQueue.Empty())
		{
			SendRequest *const request(mSendQueue.Pop());
			request->~SendRequest();
			allocator->Free(request, sizeof(SendRequest));
		}

		mNetworkLock.Unlock();

		// Release the input message used repeatedly within the loop.
		if (!inputMessage->Release())
		{
			THERON_FAIL_MSG("Failed to release network input message");
		}

		// Destroy the output and input messages.
		outputMessage->~OutputMessage();
		allocator->Free(outputMessage, sizeof(Detail::OutputMessage));

		inputMessage->~InputMessage();
		allocator->Free(inputMessage, sizeof(Detail::InputMessage));

		// Destroy the output and input sockets.
		outputSocket->~OutputSocket();
		allocator->Free(outputSocket, sizeof(Detail::OutputSocket));

		inputSocket->~InputSocket();
		allocator->Free(inputSocket, sizeof(Detail::InputSocket));
	}


} // namespace xlang2



//
// This sample shows how to register a default handler for messages of unhandled types.
//
#include "xlang2\x_actor.h"
#include "xlang2\x_framework.h"
#include "xlang2\x_receiver.h"


// Placement new/delete
//void*	operator new(xcore::xsize_t num_bytes, void* mem)			{ return mem; }
//void	operator delete(void* mem, void* )							{ }


// A simple actor with a default message handler.
class SimpleActor : public xlang2::Actor
{
public:
    inline SimpleActor()
    {
        // We only have a handler for messages of type MessageA.
        RegisterHandler(this, &SimpleActor::MessageAHandler);

        // But we also have a default handler which will catch
        // messages of all types not handled by the registered handler.
        SetDefaultHandler(this, &SimpleActor::DefaultHandler);
    }

	// A simple message type.
	class MessageA
	{
	};


	// A second, different, message type.
	class MessageB
	{
	};


	// An user-defined error message type.
	class ErrorMessage
	{
	};

private:

    // Handler for messages of type MessageA.
    inline void MessageAHandler(const MessageA &message, const xlang2::Address from)
    {
        printf("MessageAHandler received message of type MessageA\n");
        Send(message, from);
    }

    // Default message handler, run for messages of unhandled types.
    // The default handler is run for all messages not handled by a registered
    // message handler, irrespective of their type.
    // Note that unlike a conventional handler, the default handler
    // only accepts a 'from' address and not a message value. That's
    // because the type of the message isn't known to us.
    inline void DefaultHandler(const xlang2::Address from)
    {
        printf("DefaultHandler received unknown message from address '%d'\n", from.AsInteger());
        
        // Send an error message back to the recipient.
        // This error message is empty but it doesn't have to be.
        Send(ErrorMessage(), from);
    }
};


int main()
{
    xlang2::Framework framework;
	xlang2::Actor myActor(framework, "DefaultMessageHandler");

    xlang2::Receiver receiver;
    
    // Push a message of type MessageA to the actor.
    // The message type is handled by the actor and will be returned.
    xlang2::Address fromAddress(receiver.GetAddress());
    if (!myActor.Push(SimpleActor::MessageA(), fromAddress))
    {
        printf("Failed to push message of type MessageA\n");
    }

    // Wait for a message to be sent back in response.
    receiver.Wait();

    // Push a message of type MessageB to the actor.
    // This time the actor has no handler registered for the message
    // type, and the message would simply be discarded. However the
    // actor has registered a default message handler, which will catch
    // the unhandled message and send back an error message. Note that the
    // Push() method still returns true, indicating success, because the
    // message was successfully *delivered* to the recipient - even if it
    // was just subsequently ignored or default handled.
    if (!myActor.Push(SimpleActor::MessageB(), fromAddress))
    {
        printf("Failed to push message of type MessageB\n");
    }
    
    // We expect an ErrorMessage in response.
    // If there was no default handler registered, this call to Wait()
    // would block indefinitely, causing the thread to hang!
    receiver.Wait();

    return 0;
}


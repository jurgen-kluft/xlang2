#ifndef THERON_DETAIL_HANDLERS_IFALLBACKHANDLER_H
#define THERON_DETAIL_HANDLERS_IFALLBACKHANDLER_H

#include "xlang2\x_basictypes.h"
#include "xlang2\x_defines.h"

#include "xlang2\private\containers\x_list.h"
#include "xlang2\private\messages\x_imessage.h"

namespace xlang2
{
	namespace Detail
	{
		/**
		Baseclass that allows fallback handlers to be stored in lists.
		*/
		class IFallbackHandler : public List<IFallbackHandler>::Node
		{
		public:

			/**
			Default constructor.
			*/
			THERON_FORCEINLINE IFallbackHandler()
			{
			}

			/**
			Virtual destructor.
			*/
			inline virtual ~IFallbackHandler()
			{
			}

			/**
			Handles the given message.
			*/
			virtual void Handle(const IMessage *const message) const = 0;

		private:

			IFallbackHandler(const IFallbackHandler &other);
			IFallbackHandler &operator=(const IFallbackHandler &other);
		};


	} // namespace Detail
} // namespace xlang2


#endif // THERON_DETAIL_HANDLERS_IFALLBACKHANDLER_H


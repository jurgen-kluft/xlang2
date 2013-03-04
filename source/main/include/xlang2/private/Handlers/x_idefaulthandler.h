#ifndef THERON_DETAIL_HANDLERS_IDEFAULTHANDLER_H
#define THERON_DETAIL_HANDLERS_IDEFAULTHANDLER_H

#include "xlang2\x_basictypes.h"
#include "xlang2\x_defines.h"

#include "xlang2\private\messages\x_imessage.h"

namespace xlang2
{
	class Actor;

	namespace Detail
	{
		/**
		Baseclass that allows default handlers to be stored in lists.
		*/
		class IDefaultHandler
		{
		public:

			/**
			Default constructor.
			*/
			inline IDefaultHandler()
			{
			}

			/**
			Virtual destructor.
			*/
			inline virtual ~IDefaultHandler()
			{
			}

			/**
			Handles the given message.
			*/
			virtual void Handle(Actor *const actor, const IMessage *const message) const = 0;

		private:

			IDefaultHandler(const IDefaultHandler &other);
			IDefaultHandler &operator=(const IDefaultHandler &other);
		};


	} // namespace Detail
} // namespace xlang2


#endif // THERON_DETAIL_HANDLERS_IDEFAULTHANDLER_H

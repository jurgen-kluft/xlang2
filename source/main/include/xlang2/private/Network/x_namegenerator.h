#ifndef THERON_DETAIL_NETWORK_NAMEGENERATOR_H
#define THERON_DETAIL_NETWORK_NAMEGENERATOR_H

#include <stdlib.h>
#include <string.h>

#include "xbase\x_string_std.h"

#include "xlang2\x_assert.h"
#include "xlang2\x_basictypes.h"
#include "xlang2\x_defines.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning (disable:4996)  // function or variable may be unsafe.
#endif //_MSC_VER


namespace xlang2
{
	namespace Detail
	{
		/**
		Generates string names from numbers.
		*/
		class NameGenerator
		{
		public:
			inline static void Generate(char *const buffer, const uint32_t id)
			{
				THERON_ASSERT(buffer);
				xcore::x_sprintf(buffer, 5, "%x", id);
			}

			inline static void Combine(
				char *const buffer,
				const uint32_t bufferSize,
				const char *const rawName,
				const char *const frameworkName,
				const char *const networkName)
			{
				THERON_ASSERT(buffer);
				THERON_ASSERT(rawName);

				if (strlen(rawName) + 1 < bufferSize)
				{
					strcpy(buffer, rawName);
				}

				if (frameworkName && strlen(buffer) + strlen(frameworkName) + 2 < bufferSize)
				{
					strcat(buffer, ".");
					strcat(buffer, frameworkName);
				}

				if (networkName && strlen(buffer) + strlen(networkName) + 2 < bufferSize)
				{
					strcat(buffer, ".");
					strcat(buffer, networkName);
				}
			}
		};


	} // namespace Detail
} // namespace xlang2


#ifdef _MSC_VER
#pragma warning(pop)
#endif //_MSC_VER


#endif // THERON_DETAIL_NETWORK_NAMEGENERATOR_H

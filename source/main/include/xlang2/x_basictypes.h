#ifndef THERON_BASICTYPES_H
#define THERON_BASICTYPES_H

/**
\file BasicTypes.h
\brief Defines basic standard types in a hopefully cross-platform manner.
*/
#include "xbase\x_target.h"
#include "xlang2\x_defines.h"


namespace Theron
{
	// In Boost builds we reuse the convenient Boost integer types.
	typedef xcore::u8 uint8_t;
	typedef xcore::u32 uint32_t;
	typedef xcore::s32 int32_t;
	typedef xcore::u64 uint64_t;

	// Boost doesn't define uintptr_t so we have to define it ourselves.
#if THERON_64BIT
	typedef xcore::u64 uintptr_t;
#else
	typedef xcore::u32 uintptr_t;
#endif

}


#endif // THERON_BASICTYPES_H


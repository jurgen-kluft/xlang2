#include "xbase\x_base.h"
#include "xbase\x_console.h"
#include "xbase\x_allocator.h"
#include "xbase\x_string_std.h"

#include "xunittest\xunittest.h"

UNITTEST_SUITE_LIST(xLangUnitTest);
UNITTEST_SUITE_DECLARE(xLangUnitTest, xlang2_features);

namespace xcore
{
	class UnitTestAllocator : public UnitTest::Allocator
	{
		xcore::x_iallocator*	mAllocator;
	public:
						UnitTestAllocator(xcore::x_iallocator* allocator)	{ mAllocator = allocator; }
		virtual void*	Allocate(xsize_t size)								{ return mAllocator->allocate(size, 4); }
		virtual void	Deallocate(void* ptr)								{ mAllocator->deallocate(ptr); }
	};

	class TestAllocator : public x_iallocator
	{
		x_iallocator*		mAllocator;
	public:
							TestAllocator(x_iallocator* allocator) : mAllocator(allocator) { }

		virtual const char*	name() const										{ return "xbase unittest test heap allocator"; }

		virtual void*		allocate(xsize_t size, u32 alignment)
		{
			UnitTest::IncNumAllocations();
			return mAllocator->allocate(size, alignment);
		}

		virtual void*		reallocate(void* mem, xsize_t size, u32 alignment)
		{
			if (mem==NULL)
				return allocate(size, alignment);
			else 
				return mAllocator->reallocate(mem, size, alignment);
		}

		virtual void		deallocate(void* mem)
		{
			UnitTest::DecNumAllocations();
			mAllocator->deallocate(mem);
		}

		virtual void		release()
		{
			mAllocator->release();
			mAllocator = NULL;
		}
	};
}

xcore::x_iallocator* gTestAllocator = NULL;

bool gRunUnitTest(UnitTest::TestReporter& reporter)
{
	xcore::x_iallocator* systemAllocator = xcore::gCreateSystemAllocator();
	xcore::UnitTestAllocator unittestAllocator( systemAllocator );
	UnitTest::SetAllocator(&unittestAllocator);

	xcore::xconsole::addDefault();

	xcore::TestAllocator testAllocator(systemAllocator);
	gTestAllocator = &testAllocator;

	xbase::x_Init();
	int r = UNITTEST_SUITE_RUN(reporter, xLangUnitTest);
	xbase::x_Exit();

	gTestAllocator->release();

	UnitTest::SetAllocator(NULL);

	return r==0;
}


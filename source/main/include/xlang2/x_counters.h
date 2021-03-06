#ifndef THERON_COUNTERS_H
#define THERON_COUNTERS_H

/**
\file Counters.h
Performance event counters.
*/

namespace xlang2
{
	/**
	\brief Enumerated type that lists event counters available for querying.

	The counters measure threadpool activity levels, so are useful for
	managing the size of the internal thread-pools within \ref Framework "frameworks".

	\note All counters are local to each Framework instance, and count events in
	the queried Framework only.

	\see Framework::GetCounterValue
	*/
	enum Counter
	{
		COUNTER_MESSAGES_PROCESSED = 0,     ///< Number of messages processed by the framework.
		COUNTER_THREADS_PULSED,             ///< Number of times the framework pulsed its threadpool to wake a thread.
		COUNTER_THREADS_WOKEN,              ///< Number of threads actually woken by pulse events.
		MAX_COUNTERS                        ///< Number of counters available for querying.
	};


} // namespace xlang2


#endif // THERON_COUNTERS_H

#ifndef __HEADER_PROCESSORMAPFLAGCOUNTER
#define __HEADER_PROCESSORMAPFLAGCOUNTER

#include "ProcessorMap_Types.h"
#include <functional>

namespace Processors {

	class FlagCounter {
	public:

		typedef std::map<FlagIDType, CounterType> CounterStates;
		typedef CounterStates::iterator CounterStateIter;
		typedef CounterStates::const_iterator CounterStateCIter;
		typedef std::pair<FlagIDType, CounterType> CounterStatePair;

		typedef std::function<void(FlagIDType, bool)> CallbackFunction;

	private:
		CounterStates _Counters;
		CallbackFunction _NotifyCallback;

	public:

		void NotifyCallback(FlagIDType ID, bool Invalidate);

		void Counter_Init(const FlagIDType ID);

		bool Counter_IsValid(const FlagIDType ID) const;

		CounterType Counter_Get(const FlagIDType ID) const;

		void Counter_Set(const FlagIDType ID, const CounterType Counter, bool Invalidate = true);

		void Counter_Increment(const FlagIDType ID, const CounterType Counter, bool Invalidate = true);

		void Counter_Increment(const FlagIDType ID, bool Invalidate = true);

		void Counter_Decrement(const FlagIDType ID, const CounterType Counter = 1, bool Invalidate = true);

		void Counter_Decrement(const FlagIDType ID, bool Invalidate = true);

		void SetCounterObserver(CallbackFunction ObserverFunction);

		void Counter_Reset(const FlagIDType ID, bool Invalidate = true);


		FlagCounter();

		~FlagCounter();

	};
};


#endif
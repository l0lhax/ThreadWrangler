#ifndef __HEADER_PROCESSORMAPGROUPCOUNTER
#define __HEADER_PROCESSORMAPGROUPCOUNTER

#include "ProcessorMap_Flag.h"


namespace Processors {

	class GroupCounterManager {
	public:

		class GroupCounterState {
		public:

			CounterType Counter;
			bool Invalid;
			FlagIDType FlagID;

			GroupCounterState() {
				FlagID = 0;
				Invalid = true;
				Counter = ZeroCounter;
			}
		};

		typedef GroupCounterState * GroupCounterStatePtr;

		typedef std::map<FlagIDType, GroupCounterStatePtr> CounterStates;
		typedef CounterStates::iterator CounterStatesIter;
		typedef CounterStates::const_iterator CounterStatesCIter;
		typedef std::pair<FlagIDType, GroupCounterStatePtr> CounterStatesPair;

	private:
		CounterStates _CounterStates;

	public:

		void Init_Counter(const FlagIDType ID);
		Counters::FlagValidState FlagIsValid(const FlagIDType ID) const;
		CounterType GetCounter(const FlagIDType ID) const;
		void SetCounter(const FlagIDType ID, const CounterType Counter) const;
		void Invalidate(FlagIDType ID, bool Invalid = true) const;
		void InvalidateAll() const;

		GroupCounterManager();

		~GroupCounterManager();

	};
};


#endif
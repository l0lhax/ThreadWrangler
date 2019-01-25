#include "ProcessorMap_FlagCounter.h"

using namespace Processors;

void FlagCounter::NotifyCallback(FlagIDType ID, bool Invalidate) {
	if (_NotifyCallback)
		_NotifyCallback(ID, Invalidate);
}

void FlagCounter::Counter_Init(const FlagIDType ID) {
	CounterStateIter it = _Counters.find(ID);
	if (it != _Counters.end()) {
		it->second = ZeroCounter;
	}
	else {
		_Counters.emplace(CounterStatePair(ID, ZeroCounter));
	}
}

bool FlagCounter::Counter_IsValid(const FlagIDType ID) const {
	CounterStateCIter it = _Counters.find(ID);
	if (it != _Counters.cend()) {
		return true;
	}

	return false;
}

CounterType FlagCounter::Counter_Get(const FlagIDType ID) const {
	CounterStateCIter it = _Counters.find(ID);
	if (it != _Counters.end()) {
		return it->second;
	}

	return ZeroCounter;
}

void FlagCounter::Counter_Set(const FlagIDType ID, const CounterType Counter, bool Invalidate) {
	CounterStateIter it = _Counters.find(ID);
	if (it != _Counters.end()) {

		if (it->second != Counter) {
			it->second = Counter;
			NotifyCallback(ID, Invalidate);
		}

	}
}

void FlagCounter::Counter_Increment(const FlagIDType ID, const CounterType Counter, bool Invalidate) {
	CounterStateIter it = _Counters.find(ID);
	if (it != _Counters.end()) {
		it->second += Counter;

		if (Counter != 0)
			NotifyCallback(ID, Invalidate);

	}
}

void FlagCounter::Counter_Increment(const FlagIDType ID, bool Invalidate) {
	Counter_Increment(ID, 1, Invalidate);
}

void FlagCounter::Counter_Decrement(const FlagIDType ID, const CounterType Counter, bool Invalidate) {

	CounterStateIter it = _Counters.find(ID);
	if (it != _Counters.end()) {
		if (it->second - Counter < 0) {
			it->second = 0;
		}
		else {
			it->second -= Counter;
		}

		if (Counter != 0)
			NotifyCallback(ID, Invalidate);
	}

}

void FlagCounter::Counter_Decrement(const FlagIDType ID, bool Invalidate) {
	Counter_Decrement(ID, 1, Invalidate);
}

void FlagCounter::SetCounterObserver(FlagCounter::CallbackFunction ObserverFunction) {
	_NotifyCallback = ObserverFunction;
}

void FlagCounter::Counter_Reset(const FlagIDType ID, bool Invalidate) {

	CounterStateIter it = _Counters.find(ID);
	if (it != _Counters.end()) {
		if (it->second != 0) {
			it->second = ZeroCounter;

			if (Invalidate) {
				NotifyCallback(ID, Invalidate);
			}
		}
	}

}


FlagCounter::FlagCounter() {
}

FlagCounter::~FlagCounter() {
	_Counters.clear();
}

#include "ProcessorMap_GroupCounter.h"

using namespace Processors;

void GroupCounterManager::Init_Counter(const FlagIDType ID) {

	CounterStatesCIter it = _CounterStates.find(ID);
	if (it != _CounterStates.end()) {
		GroupCounterStatePtr tGroupCounterState = it->second;
		tGroupCounterState->Counter = 0;
		tGroupCounterState->Invalid = true;
	}
	else {
		GroupCounterStatePtr tGroupCounterState = new GroupCounterState();
		_CounterStates.emplace(CounterStatesPair(ID, tGroupCounterState));
	}

}

Counters::FlagValidState GroupCounterManager::FlagIsValid(const FlagIDType ID) const {

	CounterStatesCIter it = _CounterStates.find(ID);
	if (it != _CounterStates.end()) {
		GroupCounterStatePtr tGroupCounterState = it->second;
		return (tGroupCounterState->Invalid) ? Counters::VS_Invalid : Counters::VS_Valid;
	}

	return Counters::VS_DoesNotExist;

}

CounterType GroupCounterManager::GetCounter(const FlagIDType ID) const {

	CounterStatesCIter it = _CounterStates.find(ID);
	if (it != _CounterStates.end()) {
		GroupCounterStatePtr tGroupCounterState = it->second;
		return tGroupCounterState->Counter;
	}

	return ZeroCounter;

}

void GroupCounterManager::SetCounter(const FlagIDType ID, const CounterType Counter) const {
	CounterStatesCIter it = _CounterStates.find(ID);
	if (it != _CounterStates.end()) {
		GroupCounterStatePtr tGroupCounterState = it->second;
		tGroupCounterState->Counter = Counter;
		tGroupCounterState->Invalid = false;
	}
}

void GroupCounterManager::Invalidate(FlagIDType ID, bool Invalid) const {

	if (ID == Flags::FLAG_Reset) {
		InvalidateAll();
	}
	else {
		CounterStatesCIter it = _CounterStates.find(ID);
		if (it != _CounterStates.end()) {
			GroupCounterStatePtr tGroupCounterState = it->second;
			tGroupCounterState->Invalid = Invalid;
		}
	}
}

void GroupCounterManager::InvalidateAll() const {
	for (CounterStatesCIter it = _CounterStates.begin(); it != _CounterStates.end(); ++it) {
		GroupCounterStatePtr tGroupCounterState = it->second;
		tGroupCounterState->Invalid = true;
	}
}

GroupCounterManager::GroupCounterManager() {
}

GroupCounterManager::~GroupCounterManager() {
	for (CounterStatesIter it = _CounterStates.begin(); it != _CounterStates.end(); ++it) {
		GroupCounterStatePtr tGroupCounterState = it->second;
		delete tGroupCounterState;
	}

	_CounterStates.clear();
}

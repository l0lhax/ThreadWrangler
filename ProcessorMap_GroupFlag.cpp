#include "ProcessorMap_GroupFlag.h"

using namespace Processors;

void GroupFlagManager::InitFlag(const FlagIDType ID) {

	GroupFlagStatePtr tGroupFlagState;
	if (!FindGroupFlagState(ID, tGroupFlagState)) {
		tGroupFlagState = new GroupFlagState();
		_FlagStates.emplace(FlagStatesPair(ID, tGroupFlagState));
	}

}

Flags::FlagValidState GroupFlagManager::FlagIsValid(const FlagIDType ID) const {

	GroupFlagStatePtr tGroupFlagState;
	if (FindGroupFlagState(ID, tGroupFlagState)) {
		return (tGroupFlagState->Invalid) ? Flags::VS_Invalid : Flags::VS_Valid;
	}

	return Flags::VS_DoesNotExist;

}

AffinityType GroupFlagManager::GetAffinity(const FlagIDType ID) const {

	GroupFlagStatePtr tGroupFlagState;
	if (FindGroupFlagState(ID, tGroupFlagState)) {
		return tGroupFlagState->Affinity;
	}

	return ZeroAffinity;

}

void GroupFlagManager::SetAffinity(const FlagIDType ID, const AffinityType Affinity) const {

	GroupFlagStatePtr tGroupFlagState;
	if (FindGroupFlagState(ID, tGroupFlagState)) {
		tGroupFlagState->Affinity = Affinity;
		tGroupFlagState->Invalid = false;
	}

}

void GroupFlagManager::Invalidate(FlagIDType ID, bool Invalid) const {

	if (ID == Flags::FLAG_Reset) {
		InvalidateAll();
	}
	else {
		GroupFlagStatePtr tGroupFlagState;
		if (FindGroupFlagState(ID, tGroupFlagState)) {
			tGroupFlagState->Invalid = Invalid;
		}
	}
}

void GroupFlagManager::InvalidateAll() const {
	for (FlagStatesCIter it = _FlagStates.begin(); it != _FlagStates.end(); ++it) {
		GroupFlagStatePtr tGroupFlagState = it->second;
		tGroupFlagState->Invalid = true;
	}
}

bool GroupFlagManager::FindGroupFlagState(const FlagIDType ID, GroupFlagStatePtr & Result) const {
	
	FlagStatesCIter it = _FlagStates.find(ID);
	return (it != _FlagStates.end()) ? Result = it->second : Result = nullptr;

}

GroupFlagManager::GroupFlagManager() {

}

GroupFlagManager::~GroupFlagManager() {
	for (FlagStatesIter it = _FlagStates.begin(); it != _FlagStates.end(); ++it) {
		GroupFlagStatePtr tGroupFlagState = it->second;
		delete tGroupFlagState;
	}

	_FlagStates.clear();
}

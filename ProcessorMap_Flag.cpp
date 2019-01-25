#include "ProcessorMap_Flag.h"

using namespace Processors;

void tFlag::NotifyCallback(FlagIDType ID, bool Invalidate) {
	if (_NotifyCallback)
		_NotifyCallback(ID, Invalidate);
}

tFlag::FlagType tFlag::Flag_IDToFlag(FlagIDType ID) const {
	return (1i64 << ID);
}

void tFlag::Flag_Set(FlagIDType ID, bool Value, bool Invalidate) {

	if (Value != Flag_Get(ID)) {

		FlagType tFlag = (1i64 << ID);
		Value ? _Flags |= tFlag : _Flags &= ~tFlag;

		if (Invalidate) {
			NotifyCallback(ID, Invalidate);
		}

	}

}

void tFlag::SetFlagObserver(CallbackFunction ObserverFunction) {
	_NotifyCallback = ObserverFunction;
}

bool tFlag::Flag_Get(FlagIDType ID) const {
	FlagType tFlag = (1i64 << ID);
	return ((_Flags & tFlag) == tFlag);
}

void tFlag::ClearFlags() {
	_Flags = 0;
	NotifyCallback(Flags::FLAG_Reset, true);
}


tFlag::tFlag() {
	_Flags = 0;
	
}
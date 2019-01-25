#ifndef __HEADER_PROCESSORMAPCANDIDATEFLAG
#define __HEADER_PROCESSORMAPCANDIDATEFLAG

#include <functional>
#include "ProcessorMap_Types.h"

namespace Processors {

	class tFlag {
	public:
		typedef uint8_t ChangeStateType;
		typedef uint64_t FlagType;

		typedef std::function<void(FlagIDType, bool)> CallbackFunction;

	private:
		FlagType _Flags;
		CallbackFunction _NotifyCallback;

	public:

		void NotifyCallback(FlagIDType ID, bool Invalidate);
		FlagType Flag_IDToFlag(FlagIDType ID) const;
		void Flag_Set(FlagIDType ID, bool Value, bool Invalidate = true);
		void SetFlagObserver(CallbackFunction ObserverFunction);
		bool Flag_Get(FlagIDType ID) const;
		void ClearFlags();
		tFlag();

	};

};


#endif
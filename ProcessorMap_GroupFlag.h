#ifndef __HEADER_PROCESSORMAPGROUPFLAG
#define __HEADER_PROCESSORMAPGROUPFLAG

#include "ProcessorMap_Flag.h"

namespace Processors {

	class GroupFlagManager {
	public:

		class GroupFlagState {
		public:

			AffinityType Affinity;
			bool Invalid;
			FlagIDType FlagID;

			GroupFlagState() {
				FlagID = 0;
				Invalid = true;
				Affinity = ZeroAffinity;
			}
		};

		typedef GroupFlagState * GroupFlagStatePtr;

		typedef std::map<FlagIDType, GroupFlagStatePtr> FlagStates;
		typedef FlagStates::iterator FlagStatesIter;
		typedef FlagStates::const_iterator FlagStatesCIter;
		typedef std::pair<FlagIDType, GroupFlagStatePtr> FlagStatesPair;

	private:
		FlagStates _FlagStates;

	public:

		bool FindGroupFlagState(const FlagIDType ID, GroupFlagStatePtr & Result) const;

		void InitFlag(const FlagIDType ID);

		Flags::FlagValidState FlagIsValid(const FlagIDType ID) const;

		AffinityType GetAffinity(const FlagIDType ID) const;

		void SetAffinity(const FlagIDType ID, const AffinityType Affinity) const;

		void Invalidate(FlagIDType ID, bool Invalid = true) const;

		void InvalidateAll() const;

		GroupFlagManager();

		~GroupFlagManager();

	};
};


#endif
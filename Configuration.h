#ifndef __HEADER_CONFIGURATION
#define __HEADER_CONFIGURATION

#include <map>
#include <vector>
#include <stdint.h>
#include <string>
#include "ConfigurationObjects.h"

namespace Config {

	class Configuration {
	private:
		bool _ChangesPending;
		CProcessCont _Processes;
		KeyIDType _LastKeyID;
		bool _FirstRun;
		Configuration();

	public:


		bool IsFirstRun() {
			return _FirstRun;
		}

		static Configuration* Instance()
		{
			static Configuration instance;
			return &instance;
		}

		inline CProcessIter begin() noexcept { return _Processes.begin(); }
		inline CProcessCIter cbegin() const noexcept { return _Processes.cbegin(); }
		inline CProcessIter end() noexcept { return _Processes.end(); }
		inline CProcessCIter cend() const noexcept { return _Processes.cend(); }

		bool FindConfig(const std::wstring & Executable, ProcessConfigPtr & ProcessConfig);


		ProcessConfigPtr AddConfig(KeyIDType KeyID);
		bool FindConfig(KeyIDType KeyID, ProcessConfigPtr & ProcessConfig);

		bool DeleteConfig(KeyIDType KeyID);

		bool SetChangesPending() {
			_ChangesPending = true;
		}

		bool SaveConfiguration();
		bool LoadConfiguration();

		~Configuration();

	};
}

#endif

//Show Dorment
//Show Sleeping
//Show Active

//Sort how
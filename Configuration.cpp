#include "Configuration.h"
#include "pugixml-1.9/src/pugixml.hpp"
#include <Windows.h>
#include <ShlObj.h>
#include <KnownFolders.h>

Config::ProcessConfigPtr Config::Configuration::AddConfig(KeyIDType KeyID) {

	ProcessConfigPtr tProcessConfig = InvalidProcessConfig;

	CProcessIter it = _Processes.find(KeyID);
	if (it == _Processes.end()) {
		tProcessConfig = new cProcessConfig(KeyID);
		tProcessConfig->Defaults();
		tProcessConfig->ChangesPending = true;
		tProcessConfig->AdvChangesPending = true;
		tProcessConfig->Selected = false;

		CProcessContResult Result = _Processes.emplace(KeyID, tProcessConfig);
	}
	else {
		tProcessConfig = it->second;
	}

	return tProcessConfig;
}

bool Config::Configuration::FindConfig(KeyIDType KeyID, ProcessConfigPtr & ProcessConfig) {

	bool bResult = false;

	ProcessConfig = InvalidProcessConfig;

	CProcessIter it = _Processes.find(KeyID);
	if (it != _Processes.end()) {
		ProcessConfig = it->second;
		bResult = true;
	}

	return bResult;

}

bool Config::Configuration::FindConfig(const std::wstring & Executable, ProcessConfigPtr & ProcessConfig) {

	ProcessConfigPtr tProcessConfig = InvalidProcessConfig;

	for (CProcessIter it = _Processes.begin(); it != _Processes.end(); ++it) {
		tProcessConfig = it->second;
		if (_wcsicmp(tProcessConfig->Executable.c_str(), Executable.c_str()) == 0) {
			ProcessConfig = tProcessConfig;
			return true;
		}
	}

	return false;
}

bool Config::Configuration::DeleteConfig(KeyIDType KeyID) {

	bool bResult = false;

	ProcessConfigPtr tProcessConfig = InvalidProcessConfig;
	if (FindConfig(KeyID, tProcessConfig)) {
		//delete it.

		delete tProcessConfig;
		_Processes.erase(KeyID);
		bResult = true;
	}

	return bResult;

}

bool Config::Configuration::SaveConfiguration() {

	/*PWSTR path = NULL;
	HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &path);

	if (SUCCEEDED(hr)) {
		Path.assign(path);
		Path.append(L"\\ThreadWrangler");
		CoTaskMemFree(path);
	}
	else {


	}*/

	std::wstring Path;

	{
		wchar_t tBuffer[MAX_PATH];
		GetModuleFileName(NULL, tBuffer, MAX_PATH);

		Path.assign(tBuffer);
	}

	std::wstring::size_type pos = Path.find_last_of(L"\\/");
	if (pos != std::wstring::npos) {
		Path.resize(pos + 1);
	}

	Path.append(L"config.xml");

	pugi::xml_document doc;
	pugi::xml_node root;
	pugi::xml_parse_result result = doc.load_file(Path.c_str());

	if (!result) {
		auto declarationNode = doc.append_child(pugi::node_declaration);
		declarationNode.append_attribute(L"version") = L"1.0";

		root = doc.append_child(L"ThreadWrangler");
	}
	else {
		root = doc.child(L"ThreadWrangler");
	}

	pugi::xml_node Global = root.child(L"Global");
	if (!Global) {
		Global = root.append_child(L"Global");
	}

	pugi::xml_attribute tAttribute;
	if (!(tAttribute = Global.attribute(L"FirstRun"))) {
		tAttribute = Global.append_attribute(L"FirstRun");
	}
	tAttribute = false;


	for (CProcessIter it = _Processes.begin(); it != _Processes.end(); ++it) {
		ProcessConfigPtr tProcess = it->second;

		//if (tProcess->ChangesPending || tProcess->AdvChangesPending) {



			pugi::xml_node MonitoredProcess = root.find_child_by_attribute(L"ImageName", tProcess->Executable.c_str());
			if (!MonitoredProcess) {

				if (tProcess->DeletePending) 
					continue;

				MonitoredProcess = root.append_child(L"MonitoredProcess");
				MonitoredProcess.append_attribute(L"ImageName") = tProcess->Executable.c_str();

				if (tProcess->Selected) {
					MonitoredProcess.append_attribute(L"Selected") = true;
				}

			}
			else {

				if (tProcess->DeletePending) {
					root.remove_child(MonitoredProcess);
					continue;
				}

				if (tProcess->Selected) {
					if (!(tAttribute = MonitoredProcess.attribute(L"Selected"))) {
						tAttribute = MonitoredProcess.append_attribute(L"Selected") = true;
					}
				}
				else {
					if ((tAttribute = MonitoredProcess.attribute(L"Selected"))) {
						MonitoredProcess.remove_attribute(tAttribute);
					}
				}

			}

			pugi::xml_node GUI = MonitoredProcess.child(L"GUIState");
			if (!GUI) {
				GUI = MonitoredProcess.append_child(L"GUIState");
			}

			if (!(tAttribute = GUI.attribute(L"ShowActive"))) {
				tAttribute = GUI.append_attribute(L"ShowActive");
			}
			tAttribute = tProcess->ShowActiveThreads;

			if (!(tAttribute = GUI.attribute(L"ShowSleeping"))) {
				tAttribute = GUI.append_attribute(L"ShowSleeping");
			}
			tAttribute = tProcess->ShowSleepingThreads;

			if (!(tAttribute = GUI.attribute(L"ShowDorment"))) {
				tAttribute = GUI.append_attribute(L"ShowDorment");
			}
			tAttribute = tProcess->ShowDormentThreads;

			if (!(tAttribute = GUI.attribute(L"SortMode"))) {
				tAttribute = GUI.append_attribute(L"SortMode");
			}
			tAttribute = static_cast<uint32_t>(tProcess->SortMode);



			pugi::xml_node Groups = MonitoredProcess.child(L"Groups");
			if (!Groups) {
				Groups = MonitoredProcess.append_child(L"Groups");
			}

			pugi::xml_node AllocationModes = Groups.child(L"Control");
			if (!AllocationModes) {
				AllocationModes = Groups.append_child(L"Control");
			}

			if (!(tAttribute = AllocationModes.attribute(L"IdealThreadMode"))) {
				tAttribute = AllocationModes.append_attribute(L"IdealThreadMode");
			}
			tAttribute = tProcess->IdealThreadMode;

			if (!(tAttribute = AllocationModes.attribute(L"AffinityMode"))) {
				tAttribute = AllocationModes.append_attribute(L"AffinityMode");
			}
			tAttribute = tProcess->AffinityMode;

			if (!(tAttribute = AllocationModes.attribute(L"Algorithm"))) {
				tAttribute = AllocationModes.append_attribute(L"Algorithm");
			}
			tAttribute = static_cast<uint32_t>(tProcess->Algorithm);


			if (tProcess->GetGroupCount() > 0) {
				for (GroupConfigIter git = tProcess->group_begin(); git != tProcess->group_end(); ++git) {
					GroupConfigPtr tGroupConfig = git->second;

					wchar_t tBuffer[16];
					wsprintf(tBuffer, L"%u", tGroupConfig->GroupID);

					pugi::xml_node tGroup = Groups.find_child_by_attribute(L"Node",L"ID",tBuffer);
					if (!tGroup) {
						tGroup = Groups.append_child(L"Node");
					}

					if (!(tAttribute = tGroup.attribute(L"ID"))) {
						tAttribute = tGroup.append_attribute(L"ID");
					}
					tAttribute = tGroupConfig->GroupID;

					if (!(tAttribute = tGroup.attribute(L"Exclude"))) {
						tAttribute = tGroup.append_attribute(L"Exclude");
					}
					tAttribute = tGroupConfig->ExcludeGroup;

					if (!(tAttribute = tGroup.attribute(L"ExcludeHT"))) {
						tAttribute = tGroup.append_attribute(L"ExcludeHT");
					}
					tAttribute = tGroupConfig->ExcludeHTCores;

					if (!(tAttribute = tGroup.attribute(L"Excluded"))) {
						tAttribute = tGroup.append_attribute(L"Excluded");
					}
					tAttribute = tGroupConfig->Excluded;

					if (!(tAttribute = tGroup.attribute(L"ITC"))) {
						tAttribute = tGroup.append_attribute(L"ITC");
					}
					tAttribute = tGroupConfig->ITC;

					if (!(tAttribute = tGroup.attribute(L"DTC"))) {
						tAttribute = tGroup.append_attribute(L"DTC");
					}
					tAttribute = tGroupConfig->DTC;

				}
			}

	}


	bool SaveResult = doc.save_file(Path.c_str());
		
	return SaveResult;
}

bool Config::Configuration::LoadConfiguration() {

	bool bResult = true;

	std::wstring Path;

	{
		wchar_t tBuffer[MAX_PATH];
		GetModuleFileName(NULL, tBuffer, MAX_PATH);

		Path.assign(tBuffer);
	}

	std::wstring::size_type pos = Path.find_last_of(L"\\/");
	if (pos != std::wstring::npos) {
		Path.resize(pos + 1);
	}

	Path.append(L"config.xml");

	pugi::xml_document doc;
	pugi::xml_node root;
	pugi::xml_attribute tAttribute;
	pugi::xml_parse_result result = doc.load_file(Path.c_str());

	if (result) {
		root = doc.child(L"ThreadWrangler");
	}
	else {
		return false;
	}

	pugi::xml_node Global = root.child(L"Global");
	if (Global) {
		tAttribute = Global.attribute(L"FirstRun");
		if (tAttribute) {
			_FirstRun = tAttribute.as_bool();
		}
	}

	pugi::xml_node MonitoredProcess = root.child(L"MonitoredProcess");
	do {

		if (!MonitoredProcess)
			break;

		Config::ProcessConfigPtr tProcessConfig = AddConfig(_LastKeyID++);

		tAttribute = MonitoredProcess.attribute(L"ImageName");
		if (tAttribute) {
			tProcessConfig->Executable.assign(tAttribute.as_string());
		}
		else {
			continue;
		}

		tAttribute = MonitoredProcess.attribute(L"Selected");
		if (tAttribute) {
			tProcessConfig->Selected = tAttribute.as_bool();
		}

		pugi::xml_node GUIState = MonitoredProcess.child(L"GUIState");
		if (GUIState) {

			tAttribute = GUIState.attribute(L"ShowActive");
			if (tAttribute) {
				tProcessConfig->ShowActiveThreads = tAttribute.as_bool();
			}

			tAttribute = GUIState.attribute(L"ShowSleeping");
			if (tAttribute) {
				tProcessConfig->ShowSleepingThreads = tAttribute.as_bool();
			}

			tAttribute = GUIState.attribute(L"ShowDorment");
			if (tAttribute) {
				tProcessConfig->ShowDormentThreads = tAttribute.as_bool();
			}

			tAttribute = GUIState.attribute(L"SortMode");
			if (tAttribute) {
				tProcessConfig->SortMode = (Algorithms::ViewSort)tAttribute.as_uint();
			}

		}

		pugi::xml_node Groups = MonitoredProcess.child(L"Groups");
		if (Groups) {

			pugi::xml_node Control = Groups.child(L"Control");
			if (Control) {

				tAttribute = Control.attribute(L"IdealThreadMode");
				if (tAttribute) {
					tProcessConfig->IdealThreadMode = tAttribute.as_bool();
				}

				tAttribute = Control.attribute(L"AffinityMode");
				if (tAttribute) {
					tProcessConfig->AffinityMode = tAttribute.as_bool();
				}

				tAttribute = Control.attribute(L"Algorithm");
				if (tAttribute) {
					tProcessConfig->Algorithm = (Algorithms::TSAlgorithm)tAttribute.as_uint();
				}

			}

			pugi::xml_node Node = Groups.child(L"Node");
			do {

				if (!Node)
					break;

				GroupConfigPtr tGroupConfig = InvalidGroupConfigPtr;

				tAttribute = Node.attribute(L"ID");
				if (tAttribute) {
					Processors::GroupIDType GroupID = static_cast<Processors::GroupIDType>(tAttribute.as_uint());
					tGroupConfig = tProcessConfig->GetGroupConfig(GroupID);
				}

				if (tGroupConfig == nullptr) {
					continue;
				}

				tAttribute = Node.attribute(L"Exclude");
				if (tAttribute) {
					tGroupConfig->ExcludeGroup = tAttribute.as_bool();
				}

				tAttribute = Node.attribute(L"ExcludeHT");
				if (tAttribute) {
					tGroupConfig->ExcludeHTCores = tAttribute.as_bool();
				}

				tAttribute = Node.attribute(L"Excluded");
				if (tAttribute) {
					tGroupConfig->Excluded = tAttribute.as_ullong();
				}

				tAttribute = Node.attribute(L"ITC");
				if (tAttribute) {
					tGroupConfig->ITC = tAttribute.as_ullong();
				}

				tAttribute = Node.attribute(L"DTC");
				if (tAttribute) {
					tGroupConfig->DTC = tAttribute.as_ullong();
				}



			} while (Node = Node.next_sibling());


		}
	} while (MonitoredProcess = MonitoredProcess.next_sibling());
	
	//open file
	//iterate, create processes... later processes must be iterated to set the keyid's.

	return bResult;
}

Config::Configuration::Configuration() {
	_LastKeyID = 0;
	_ChangesPending = false;
	_FirstRun = true;
}

Config::Configuration::~Configuration() {

	for (CProcessIter it = _Processes.begin(); it != _Processes.end(); ++it) {
		ProcessConfigPtr tProcessConfig = it->second;

		if (tProcessConfig != nullptr) {
			delete tProcessConfig;
		}
	}

	_Processes.clear();

}
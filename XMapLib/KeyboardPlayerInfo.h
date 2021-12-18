#pragma once
#include "stdafx.h"
namespace sds
{
	/// <summary>
	/// A data structure to hold player information.
	/// A default constructed KeyboardPlayerInfo struct has default values that are usable.
	///	Thread safe.
	/// </summary>
	struct KeyboardPlayerInfo
	{
		//ISO CPP guidelines C.45 followed here: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-default
		std::atomic<int> player_id = 0;
		//default ctor
		KeyboardPlayerInfo() = default;
		//copy constructor
		KeyboardPlayerInfo(const KeyboardPlayerInfo &sp)
		{
			//temporary copies must be made to copy atomic to atomic
			int pid = sp.player_id;
			player_id = pid;
		}
		//assignment
		KeyboardPlayerInfo &operator=(const KeyboardPlayerInfo &sp)
		{
			if (this == &sp)
				return *this;
			//temporary copies must be made to copy atomic to atomic
			int pid= sp.player_id;
			player_id = pid;
			return *this;
		}
		//move constructor
		KeyboardPlayerInfo(KeyboardPlayerInfo &&sp) = delete;
		//Move assignment operator
		KeyboardPlayerInfo &operator=(KeyboardPlayerInfo &&sp) = delete;
		~KeyboardPlayerInfo() = default;
	};
}
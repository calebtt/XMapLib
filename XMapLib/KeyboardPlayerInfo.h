#pragma once
namespace sds
{
	/// <summary>
	/// A data structure to hold player information.
	/// A default constructed KeyboardPlayerInfo struct has default values that are usable.
	///	Thread safe.
	/// </summary>
	struct KeyboardPlayerInfo final
	{
		using PidType = int;
		//ISO CPP guidelines C.45 followed here: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-default
		std::atomic<PidType> player_id{ 0 };
		//default ctor
		KeyboardPlayerInfo() = default;
		//copy constructor
		KeyboardPlayerInfo(const KeyboardPlayerInfo &sp)
		{
			//temporary copies must be made to copy atomic to atomic
			const PidType pid = sp.player_id;
			player_id = pid;
		}
		//assignment
		KeyboardPlayerInfo &operator=(const KeyboardPlayerInfo &sp)
		{
			if (this == &sp)
				return *this;
			//temporary copies must be made to copy atomic to atomic
			const PidType pid= sp.player_id;
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
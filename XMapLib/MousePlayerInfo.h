#pragma once
namespace sds
{
	/// <summary>
	/// Used as a data structure to hold player information, includes thumbstick deadzone information.
	/// A default constructed MousePlayerInfo struct has default values that are usable.
	/// </summary>
	struct MousePlayerInfo final
	{
		using PidType = int;
		using DzType = int;
		//ISO CPP guidelines C.45 followed here: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-default
		std::atomic<DzType> left_polar_dz{ XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE }; // left stick polar radius dz
		std::atomic<DzType> right_polar_dz{ XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE }; // right stick polar radius dz
		std::atomic<PidType> player_id{ 0 };
		//default ctor
		MousePlayerInfo() = default;
		//copy constructor
		MousePlayerInfo(const MousePlayerInfo &sp)
		{
			//temporary copies should be made to copy atomic to atomic
			const DzType ldz = sp.left_polar_dz;
			const DzType rdz = sp.right_polar_dz;
			const PidType pid = sp.player_id;
			left_polar_dz = ldz;
			right_polar_dz = rdz;
			player_id = pid;
		}
		//assignment
		MousePlayerInfo &operator=(const MousePlayerInfo &sp)
		{
			if (this == &sp)
				return *this;
			//temporary copies should be made to copy atomic to atomic
			const DzType ldz = sp.left_polar_dz;
			const DzType rdz = sp.right_polar_dz;
			const PidType pid = sp.player_id;
			left_polar_dz = ldz;
			right_polar_dz = rdz;
			player_id = pid;
			return *this;
		}
		//move constructor
		MousePlayerInfo(MousePlayerInfo&& sp) = delete;
		//Move assignment operator
		MousePlayerInfo &operator=(MousePlayerInfo &&sp) = delete;
		~MousePlayerInfo() = default;
	};
}
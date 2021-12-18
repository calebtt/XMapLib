#pragma once
#include "stdafx.h"
namespace sds
{
	/// <summary>
	/// Used as a data structure to hold player information, includes thumbstick deadzone information.
	/// A default constructed MousePlayerInfo struct has default values that are usable.
	/// </summary>
	struct MousePlayerInfo
	{
		//ISO CPP guidelines C.45 followed here: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-default
		std::atomic<int> left_x_dz = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE; // left stick X axis dz
		std::atomic<int> left_y_dz = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE; // left stick Y axis dz
		std::atomic<int> right_x_dz = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE; // right stick X axis dz
		std::atomic<int> right_y_dz = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE; // right stick Y axis dz
		std::atomic<int> player_id = 0;
		//default ctor
		MousePlayerInfo() = default;
		//copy constructor
		MousePlayerInfo(const MousePlayerInfo &sp)
		{
			//temporary copies must be made to copy atomic to atomic
			int ldzx, ldzy, rdzx, rdzy, pid;
			ldzx = sp.left_x_dz;
			ldzy = sp.left_y_dz;
			rdzx = sp.right_x_dz;
			rdzy = sp.right_y_dz;
			pid = sp.player_id;
			left_x_dz = ldzx;
			left_y_dz = ldzy;
			right_x_dz = rdzx;
			right_y_dz = rdzy;
			player_id = pid;
		}
		//assignment
		MousePlayerInfo &operator=(const MousePlayerInfo &sp)
		{
			if (this == &sp)
				return *this;
			//temporary copies must be made to copy atomic to atomic
			int ldzx, ldzy, rdzx, rdzy, pid;
			ldzx = sp.left_x_dz;
			ldzy = sp.left_y_dz;
			rdzx = sp.right_x_dz;
			rdzy = sp.right_y_dz;
			pid = sp.player_id;
			left_x_dz = ldzx;
			left_y_dz = ldzy;
			right_x_dz = rdzx;
			right_y_dz = rdzy;
			player_id = pid;
			return *this;
		}
		//move constructor
		MousePlayerInfo(MousePlayerInfo &&sp) = delete;
		//Move assignment operator
		MousePlayerInfo &operator=(MousePlayerInfo &&sp) = delete;
		~MousePlayerInfo() = default;
	};
}
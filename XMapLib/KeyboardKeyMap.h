#pragma once
#include "stdafx.h"
#include "DelayManager.h"

namespace sds
{
	/// <summary>
	/// Utility class for holding controller to keyboard maps.
	/// </summary>
	struct KeyboardKeyMap
	{
		using ClockType = std::chrono::high_resolution_clock;
		using PointInTime = std::chrono::time_point<ClockType>;
		enum class ActionType : int
		{
			NONE = 0,
			KEYDOWN = XINPUT_KEYSTROKE_KEYDOWN,
			KEYREPEAT = XINPUT_KEYSTROKE_REPEAT,
			KEYUP = XINPUT_KEYSTROKE_KEYUP
		};
		//Struct members
		int SendingElementVK = 0;
		int MappedToVK = 0;
		bool UsesRepeat = true;
		ActionType LastAction = ActionType::NONE;
		Utilities::DelayManager LastSentTime = KeyboardSettings::MICROSECONDS_DELAY_KEYREPEAT;
		//Ctor
		KeyboardKeyMap(const int controllerElementVK, const int keyboardMouseElementVK, const bool useRepeat)
			: SendingElementVK(controllerElementVK), MappedToVK(keyboardMouseElementVK), UsesRepeat(useRepeat)
		{
		}
		KeyboardKeyMap() = default;
		KeyboardKeyMap(const KeyboardKeyMap& other) = default;
		KeyboardKeyMap(KeyboardKeyMap&& other) = default;
		KeyboardKeyMap& operator=(const KeyboardKeyMap& other) = default;
		KeyboardKeyMap& operator=(KeyboardKeyMap&& other) = default;
		~KeyboardKeyMap() = default;
		friend std::ostream& operator<<(std::ostream& os, const ActionType& obj)
		{
			return os << static_cast<int>(obj);
		}
		friend std::ostream& operator<<(std::ostream& os, const KeyboardKeyMap& obj)
		{
			return os
				<< "SendingElementVK: " << obj.SendingElementVK << "\n"
				<< "MappedToVK: " << obj.MappedToVK << " AKA: " << std::quoted(std::string("") + static_cast<char>(obj.MappedToVK)) << "\n"
				<< "LastAction: " << obj.LastAction << "\n"
				<< "LastSentTime: " << obj.LastSentTime << "\n";
		}
	};
}
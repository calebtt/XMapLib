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
		int SendingElementVK = 0; // VK of controller button
		int MappedToVK = 0; // VK of mapped-to input (key or mouse button)
		bool UsesRepeat = true; // Uses the key-repeat behavior when held down
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
			return os << "SendingElementVK: " << obj.SendingElementVK << "\n"
				<< "MappedToVK: " << obj.MappedToVK << " AKA: " << std::quoted(std::string("") + static_cast<char>(obj.MappedToVK)) << "\n"
				<< "LastAction: " << obj.LastAction << "\n"
				<< "LastSentTime: " << obj.LastSentTime << "\n";
		}
		/// <summary>
		/// Operator<< overload for std::string specialization,
		///	returns relevant map details.
		/// </summary>
		friend std::string& operator<<(std::string& os, const KeyboardKeyMap& obj)
		{
			os += "SendingElementVK:" + std::to_string(obj.SendingElementVK) + " ";
			os += "MappedToVK:" + std::to_string(obj.MappedToVK) + " ";
			os += "BoolUsesRepeat:" + std::to_string(obj.UsesRepeat);
			return os;
		}
		friend bool operator==(const KeyboardKeyMap& lhs, const KeyboardKeyMap& rhs)
		{
			return lhs.SendingElementVK == rhs.SendingElementVK
				&& lhs.MappedToVK == rhs.MappedToVK;
		}
		friend bool operator!=(const KeyboardKeyMap& lhs, const KeyboardKeyMap& rhs)
		{
			return !(lhs == rhs);
		}
	};
}
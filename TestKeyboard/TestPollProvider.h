#pragma once
#include "pch.h"

namespace TestKeyboard
{
	struct TestPollProvider
	{
		unsigned short Vk{};

		auto GetDownState() -> sds::ControllerStateWrapper
		{
			return GetDownState(Vk);
		}
		auto GetDownState(const unsigned short newVk) -> sds::ControllerStateWrapper
		{
			return sds::ControllerStateWrapper{ .VirtualKey = newVk, .KeyDown = true, .KeyUp = false, .KeyRepeat = false };
		}
		auto GetUpState() -> sds::ControllerStateWrapper
		{
			return GetUpState(Vk);
		}
		auto GetUpState(const unsigned short newVk) -> sds::ControllerStateWrapper
		{
			return sds::ControllerStateWrapper{ .VirtualKey = newVk, .KeyDown = false, .KeyUp = true, .KeyRepeat = false };
		}
		auto GetRepeatState() -> sds::ControllerStateWrapper
		{
			return GetRepeatState(Vk);
		}
		auto GetRepeatState(const unsigned short newVk) -> sds::ControllerStateWrapper
		{
			return sds::ControllerStateWrapper{ .VirtualKey = newVk, .KeyDown = false, .KeyUp = false, .KeyRepeat = true };
		}
		auto GetNoState() -> sds::ControllerStateWrapper
		{
			return GetNoState(Vk);
		}
		auto GetNoState(const unsigned short newVk) -> sds::ControllerStateWrapper
		{
			return sds::ControllerStateWrapper{ .VirtualKey = newVk, .KeyDown = false, .KeyUp = false, .KeyRepeat = false };
		}
	};
}

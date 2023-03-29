#pragma once
#include "KeyboardActionTranslator.h"

struct TestPollProvider
{
	unsigned short Vk{};

	auto GetDownState() const noexcept
	{
		return sds::ControllerStateWrapper{ .VirtualKey = Vk, .KeyDown = true, .KeyUp = false, .KeyRepeat = false };
	}
	auto GetUpState() const noexcept
	{
		return sds::ControllerStateWrapper{ .VirtualKey = Vk, .KeyDown = false, .KeyUp = true, .KeyRepeat = false };
	}
	auto GetRepeatState() const noexcept
	{
		return sds::ControllerStateWrapper{ .VirtualKey = Vk, .KeyDown = false, .KeyUp = false, .KeyRepeat = true };
	}
	auto GetNoState() const noexcept
	{
		return sds::ControllerStateWrapper{ .VirtualKey = Vk, .KeyDown = false, .KeyUp = false, .KeyRepeat = false };
	}
};
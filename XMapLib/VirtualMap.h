#pragma once
#include "stdafx.h"

/// <summary>
/// A collection of platform dependent virtual keycode manipulation functions.
/// </summary>
namespace sds::Utilities::VirtualMap
{
	/// <summary>
	///	Utility function to map a Virtual Keycode to a char
	///	</summary>
	/// <returns>printable char value or 0 on error</returns>
	[[nodiscard]] inline char GetCharFromVK(const unsigned int vk) noexcept
	{
		return static_cast<char>(MapVirtualKeyA(vk, MAPVK_VK_TO_CHAR));
	}
	/// <summary>
	/// Utility function to map a Virtual Keycode to a scancode
	/// </summary>
	/// <returns>scancode value or 0 on error</returns>
	[[nodiscard]] inline unsigned short GetScanCodeFromVK(const unsigned int vk) noexcept
	{
		return static_cast<unsigned short> (MapVirtualKeyA(vk, MAPVK_VK_TO_VSC));
	}
}


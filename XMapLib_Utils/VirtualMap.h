#pragma once
#include "pch.h"

namespace sds::Utilities
{
	namespace VirtualMap
	{
		using ScanCodeType = unsigned short;
		using VirtualKeyType = unsigned int;
		using PrintableType = char;
		/// <summary> Utility function to map a Virtual Keycode to a char </summary>
		///	<returns> printable char value or 0 on error </returns>
		[[nodiscard]]
		inline
		auto GetCharFromVK(const VirtualKeyType vk) noexcept -> PrintableType
		{
			return static_cast<PrintableType>(MapVirtualKeyA(vk, MAPVK_VK_TO_CHAR));
		}
		/// <summary> Utility function to map a char to a Virtual Keycode </summary>
		/// <returns> printable char value or 0 on error </returns>
		[[nodiscard]] 
		inline
		auto GetVKFromChar(const PrintableType letter) noexcept -> VirtualKeyType
		{
			return static_cast<VirtualKeyType>(VkKeyScanA(letter));
		}
	}
}
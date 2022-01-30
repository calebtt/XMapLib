#pragma once
#include "stdafx.h"

namespace sds::Utilities
{
	class VirtualMap
	{
	public:
		using ScanCodeType = unsigned short;
		using VirtualKeyType = unsigned int;
		using PrintableType = char;
		/// <summary>Utility function to map a Virtual Keycode to a char</summary>
		/// <returns>printable char value or 0 on error</returns>
		[[nodiscard]] static PrintableType GetCharFromVK(const VirtualKeyType vk) noexcept
		{
			return static_cast<PrintableType>(MapVirtualKeyA(vk, MAPVK_VK_TO_CHAR));
		}
	};
}
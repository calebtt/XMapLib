#pragma once
#include "../XMapLib/KeyboardMapper.h"
#include <type_traits>
//helper
template<typename Ret> requires std::is_enum_v<sds::StickMap>&& std::is_integral_v<Ret>
[[nodiscard]] inline constexpr Ret toNum(sds::StickMap m) noexcept
{
	using enum_type = std::underlying_type_t<sds::StickMap>;
	return static_cast<Ret>(static_cast<enum_type>(m));
}
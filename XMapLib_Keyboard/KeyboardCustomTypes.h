#pragma once
#include "KeyboardLibIncludes.h"

namespace sds::detail
{
	using Fn_t = std::function<void()>;
	using OptFn_t = std::optional<Fn_t>;
	using NanosDelay_t = std::chrono::nanoseconds;
	using OptNanosDelay_t = std::optional<NanosDelay_t>;
	using GrpVal_t = int;
	using OptGrp_t = std::optional<GrpVal_t>;
	using VirtualKey_t = int32_t;
	using TriggerValue_t = uint8_t;
	using ThumbstickValue_t = int16_t;

	// TODO this might be replaced with std::vector if the user doesn't have boost.
	template<typename T>
	using SmallVector_t = boost::container::small_vector<T, 32>;
}
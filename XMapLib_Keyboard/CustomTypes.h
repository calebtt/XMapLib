#pragma once
#include "LibIncludes.h"

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

	template<typename T>
	using StaticVector_t = std::vector<T>;
}
#pragma once
#include "LibIncludes.h"

namespace sds::detail
{
	using Fn_t = std::function<void()>;
	using OptFn_t = std::optional<Fn_t>;
	using Delay_t = std::chrono::nanoseconds;
	using OptDelay_t = std::optional<Delay_t>;
	using GrpVal_t = int;
	using OptGrp_t = std::optional<GrpVal_t>;
}
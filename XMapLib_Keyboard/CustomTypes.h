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
}
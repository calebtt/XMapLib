#pragma once
#include <iostream>
#include <syncstream>
/// <summary>
/// One function called to log errors, to "cerr" at the moment.
///	Can be disabled easily or redirected here.
/// </summary>
/// <param name="s"></param>
namespace sds::Utilities
{
	[[msvc::noinline]]
	inline void LogError(std::string_view s) noexcept
	{
		if (!s.empty())
		{
			//osyncstream can be used with concurrency to avoid garbled output,
			//as long as each thread has it's own osyncstream object.
			std::osyncstream sout(std::cerr);
			sout << s << std::endl;
		}
	}
}

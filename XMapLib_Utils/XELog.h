#pragma once
#include <iostream>
#include <syncstream>
#include <magic_enum.hpp>
/// <summary>
/// One function called to log errors, to "cerr" at the moment.
///	Can be disabled easily or redirected here. Also contains using
///	declarations for pointer to LogError() func.
/// </summary>
/// <param name="s"></param>
namespace sds::Utilities
{
	enum class XELogLevel
	{
		TRACE,
		DEBUG,
		INFORMATION,
		WARNING,
		ERR,
		CRITICAL
	};

	/// <summary> Type alias for pointer to the LogError(std::string) function. </summary>
	using XELogPtr = std::function<void(std::string)>;

	/// <summary> Error logging function, thread safe. </summary>
	//[[msvc::noinline]]
	inline
	void LogError(const std::string s) noexcept
	{
		if (!s.empty())
		{
			//osyncstream can be used with concurrency to avoid garbled output,
			//as long as each thread has it's own osyncstream object.
			std::osyncstream sout(std::cerr);
			sout << s << std::endl;
			sout.emit();
		}
	}
}

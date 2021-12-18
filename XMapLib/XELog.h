#pragma once
#include <iostream>
#include <string>
#include <concepts>

/// <summary>
/// One function called to log errors, to "cerr" at the moment.
///	Can be disabled easily or redirected here.
/// </summary>
/// <param name="s"></param>
namespace sds::Utilities::XELog
{
	void LogError(std::string_view s)
	{
		if(!s.empty())
			std::cerr << s << std::endl;
	}
}

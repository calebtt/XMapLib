#pragma once
#include <map>
namespace sds::Utilities::MapFunctions
{
	/// <summary>
	/// Verifies that the key value is in the map{T,H} and sets a reference arg to the value
	///	if found.
	/// </summary>
	/// <param name="keyValue">key value to check</param>
	///	<param name="curMap">map of T,H to check the key</param>
	///	<param name="retVal">will be set to the value the key points to, if found</param>
	/// <returns>true if found, false otherwise</returns>
	template<class T, class H>
	bool IsInMap(const T keyValue, const std::map<T, H>& curMap, H& retVal)
	{
		auto it = curMap.find(keyValue);
		if(it != curMap.end())
		{
			retVal = it->second;
			return true;
		}
		else
		{
			return false;
		}
	}
}
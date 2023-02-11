#pragma once
#include "framework.h"

namespace sds::Utilities
{
	//hash function obj for std::pair, uses XOR and std::hash for the built-in types to combine
	//them into a single hash for using a std::pair as a key in the unordered_map.
	struct PairHasher
	{
		template <class T1, class T2>
		std::size_t operator() (const std::pair<T1, T2>& pair) const
		{
			return std::hash<T1>()(pair.first)
				^ std::hash<T2>()(pair.second);
		}
	};


}
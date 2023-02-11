#pragma once
#include "LibIncludes.h"

// Contains helper functions and using declarations for constructing smart pointer types,
// aliasing them here will make switching to a new (possibly better) smart pointer type in the future
// much easier.

namespace sds
{
	/// <summary> Alias for the <b>shared</b> pointer smart pointer type resulting from a call to
	/// <b>MakeSharedSmart()</b></summary>
	template<typename T>
	using SharedPtrType = std::shared_ptr<T>;

	/// <summary> Alias for the <b>unique</b> pointer smart pointer type resulting from a call to
	///	<b>MakeUniqueSmart()</b></summary>
	template<typename T>
	using UniquePtrType = std::unique_ptr<T>;

	/// <summary> Factory func for making <b>shared</b> smart pointer type. (makes it easier to change to a new type if desired.) </summary>
	template<typename T>
	SharedPtrType<T> MakeSharedSmart(auto ... args)
	{
		return std::make_shared<T>(args...);
	}

	/// <summary> Factory func for making <b>unique</b> smart pointer type. (makes it easier to change to a new type if desired.) </summary>
	template<typename T>
	UniquePtrType<T> MakeUniqueSmart(auto ... args)
	{
		return std::make_unique<T>(args...);
	}
}
#pragma once
#include <memory>

// Contains helper functions and using declarations for constructing smart pointer types,
// aliasing them here will make switching to a new (possibly better) smart pointer type in the future
// much easier.

namespace sds
{
	/**
	 * \brief Alias for the <b>shared</b> pointer smart pointer type resulting from a call to <b>MakeSharedSmart()</b>
	 */
	template<typename T>
	using SharedPtrType = std::shared_ptr<T>;

	/**
	 * \brief Alias for the <b>unique</b> pointer smart pointer type resulting from a call to <b>MakeUniqueSmart()</b>
	 */
	template<typename T>
	using UniquePtrType = std::unique_ptr<T>;

	/**
	 * \brief Factory func for making <b>shared</b> smart pointer type. (makes it easier to change to a new type if desired.)
	 */
	template<typename T>
	SharedPtrType<T> MakeSharedSmart(auto&& ... args)
	{
		return std::make_shared<T>(std::forward<decltype(args)>(args)...);
	}

	/**
	* \brief Factory func for making <b>unique</b> smart pointer type. (makes it easier to change to a new type if desired.)
	*/
	template<typename T>
	UniquePtrType<T> MakeUniqueSmart(auto&& ... args)
	{
		return std::make_unique<T>(std::forward<decltype(args)>(args)...);
	}
}
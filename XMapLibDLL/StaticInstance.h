#pragma once
#include "pch.h"
#include "../XMapLib/MouseMapper.h"
#include "../XMapLib/KeyboardMapper.h"
#include "helperfuncs.h"
// Namespace for some static instance stuff.
namespace Stins
{
	using LockType = std::scoped_lock <std::mutex>;
	/// <summary>
	/// Struct with some data members used in the course of managing access to the
	///	static instances of <c>MouseMapper</c> and <c>KeyboardMapper</c>.
	/// </summary>
	struct MapStuff
	{
		std::mutex accessBlocker;
		std::shared_ptr<sds::STRunner> threadPoolPtr;
		sds::KeyboardMapper kbd;
		sds::MouseMapper mmp;
		// Buffer large enough to hold any map string possible, with static allocation.
		std::array<char, 1'048'576> mapInfoFormatted{};
		// ctor
		MapStuff()
		: threadPoolPtr(std::make_shared<sds::STRunner>(true, ErrorCallb)),
		kbd(threadPoolPtr, {}, ErrorCallb),
		mmp(threadPoolPtr, {}, ErrorCallb)
		{
			threadPoolPtr->StartThread();
		}

		static void ErrorCallb(const std::string msg) noexcept
		{
			MessageBoxA(nullptr, msg.c_str(), "", MB_OK);
		}
	};
}
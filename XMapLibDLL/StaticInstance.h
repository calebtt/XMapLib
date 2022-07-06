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
		//std::unique_ptr<sds::KeyboardMapper> kbd;
		//std::unique_ptr<sds::MouseMapper> mmp;
		std::string mapInfoFormatted;
		// ctor
		MapStuff()
		: threadPoolPtr(std::make_shared<sds::STRunner>(true, ErrorCallb)),
		kbd(threadPoolPtr),
		mmp(threadPoolPtr)
		{
			threadPoolPtr->StartThread();
			//ErrorCallb("Ctor called.");
		}

		static void ErrorCallb(const std::string msg) noexcept
		{
			MessageBoxA(nullptr, msg.c_str(), "", MB_OK);
		}
		//[[nodiscard]] std::unique_ptr<sds::KeyboardMapper> CreateKeyMapper(const std::shared_ptr<sds::STRunner>& runner) const
		//{
		//	using namespace sds;
		//	KeyboardSettingsPack ksp;
		//	return std::make_unique<KeyboardMapper>(runner, ksp, ErrorCallb);
		//}
		//[[nodiscard]] std::unique_ptr<sds::MouseMapper> CreateMouseMapper(const std::shared_ptr<sds::STRunner>& runner) const
		//{
		//	using namespace sds;
		//	MouseSettingsPack msp;
		//	return std::make_unique<MouseMapper>(runner, msp, ErrorCallb);
		//}
	};
}
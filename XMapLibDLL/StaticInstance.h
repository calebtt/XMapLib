#pragma once
#include "pch.h"
#include "../XMapLib/MouseMapper.h"
#include "../XMapLib/KeyboardMapper.h"
#include "../impcool_sol/immutable_thread_pool/ThreadPool.h"
#include "../impcool_sol/immutable_thread_pool/ThreadUnitPlus.h"
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
	private:
		static constexpr std::size_t MapInfoBufferSize{ 4'096 };
		inline static constexpr sds::MouseSettingsPack m_msp{};
		inline static constexpr sds::KeyboardSettingsPack m_ksp{};
	public:
		std::mutex accessBlocker;
		std::shared_ptr<impcool::ThreadUnitPlus> threadPoolPtr{ std::make_shared<impcool::ThreadUnitPlus>() };
		sds::KeyboardMapper<> KeyboardMapperInstance{ threadPoolPtr, m_ksp };
		sds::MouseMapper<> MouseMapperInstance{ threadPoolPtr, m_msp };
		// Buffer large enough to hold any map string possible, with static allocation.
		std::array<char, MapInfoBufferSize> mapInfoFormatted{};
		// ctor
		MapStuff()
		{
			threadPoolPtr->CreateThread();
		}

		static void ErrorCallb(const std::string msg) noexcept
		{
			MessageBoxA(nullptr, msg.c_str(), "", MB_OK);
		}
	};
}
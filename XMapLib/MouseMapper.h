#pragma once
#include "stdafx.h"
#include "ThumbstickToDelay.h"
#include "ThumbDzInfo.h"
#include "Utilities.h"
#include "ControllerStatus.h"
#include "STMouseMapping.h"
#include "../impcool_sol/immutable_thread_pool/ThreadPool.h"
#include "../impcool_sol/immutable_thread_pool/ThreadUnitPlus.h"

namespace sds
{
	/// <summary><c>MouseMapper</c> [[Main class for external use]] Handles achieving smooth, expected mouse movements.
	/// The class has an internal <c>STMouseMapping</c> instance that fetches controller information via
	/// the OS API call wrapper class <c>MousePoller</c>. It also has public functions for getting and
	/// setting the sensitivity as well as setting which thumbstick to use. </summary>
	///	<remarks>If a <code>std::shared_ptr{STRunner}</code> is not passed into the constructor,
	///	one will NOT be created for use. </remarks>
	template<class LogFnType = std::function<void(std::string)>>
	class MouseMapper
	{
		// Thread pool class, our work functors get added to here and called in succession on a separate thread
		// for performance reasons.
		SharedPtrType<impcool::ThreadUnitPlus> m_statRunner;
		// Mouse settings pack, needed for iscontrollerconnected func args and others.
		const MouseSettingsPack m_mouseSettingsPack;
		// data wrapper class, added to thread pool STRunner, performs the polling, calculation, and mouse moving.
		SharedPtrType<STMouseMapping<>> m_stmapper{
		MakeSharedSmart<STMouseMapping<>>(m_mouseSettingsPack.settings.SENSITIVITY_DEFAULT, StickMap::RIGHT_STICK) };
	public:
		/// <summary>Ctor allows setting a custom MousePlayerInfo</summary>
		MouseMapper(
			const SharedPtrType<impcool::ThreadUnitPlus> &statRunner,
			const MouseSettingsPack settings = {}, 
			const LogFnType logFn = nullptr
		)
		: m_statRunner(statRunner),
		  m_mouseSettingsPack(settings)
	{
			// lambda for logging
			auto LogIfAvailable = [&](const char* msg)
			{
				if (logFn != nullptr)
					logFn(msg);
				else
					throw std::exception(msg);
			};
			// if statRunner is nullptr, log error and return
			if (m_statRunner == nullptr)
			{
				LogIfAvailable("Exception: In MouseMapper::MouseMapper(...): statRunner shared_ptr was null!");
				return;
			}
			SharedPtrType<STMouseMapping<>> tempMapper = m_stmapper;
			m_statRunner->PushInfiniteTaskBack(
				[tempMapper]()
				{
					tempMapper->operator()();
				}
			);
		}
		MouseMapper(const MouseMapper& other) = delete;
		MouseMapper(MouseMapper&& other) = delete;
		MouseMapper& operator=(const MouseMapper& other) = delete;
		MouseMapper& operator=(MouseMapper&& other) = delete;
		~MouseMapper() = default;

		/// <summary> Returns a copy of the shared_ptr to the STRunner thread instance. May be null. </summary>
		auto GetSTRunner() const noexcept
		{
			return m_statRunner;
		}

		/// <summary>Use this function to establish one stick or the other as the one controlling the mouse movements.
		/// Set to NEITHER_STICK for no thumbstick mouse movement. Options are RIGHT_STICK, LEFT_STICK, NEITHER_STICK
		///	This will start processing if the stick is something other than "NEITHER" </summary>
		///	<remarks>**Arbitrary values outside of the enum constants will not be processed successfully.**</remarks>
		/// <param name="info"> a StickMap enum</param>
		void SetStick(const StickMap info) noexcept
		{
			if(m_stmapper != nullptr)
				m_stmapper->SetStick(info);
		}
		/// <summary><c>GetStick()</c> A getter for the current <code>StickMap</code> enum data member.</summary>
		/// <returns><c>StickMap</c> enum class denoting which controller thumbstick is set for processing,
		/// or default constructed StickMap on error. </returns>
		StickMap GetStick() const
		{
			if (m_stmapper == nullptr)
				return {};
			return m_stmapper->GetStick();
		}
		/// <summary><c>SetSensitivity(...)</c> Setter for sensitivity value.</summary>
		/// <returns> returns a <c>std::string</c> containing an error message
		/// if there is an error, empty string otherwise. </returns>
		std::string SetSensitivity(const int new_sens) const noexcept
		{
			if (m_stmapper == nullptr)
				return "Error in sds::XinMouseMapper::SetSensitivity(), m_stmapper is null.";
			if (!m_mouseSettingsPack.settings.IsValidSensitivityValue(new_sens))
			{
				return "Error in sds::XinMouseMapper::SetSensitivity(), int new_sens out of range.";
			}
			m_stmapper->SetSensitivity(new_sens);
			return "";
		}
		/// <summary><c>GetSensitivity()</c> Getter for sensitivity value</summary>
		///	<returns>current sensitivity value, or -1 if there is no valid STMapping object (as in called during destruction).</returns>
		[[nodiscard]] int GetSensitivity() const noexcept
		{
			if (m_stmapper == nullptr)
				return -1;
			return m_stmapper->GetSensitivity();
		}
		///<summary><c>IsControllerConnected()</c> returns true if the current <c>player_id</c> reports a controller connected.</summary>
		[[nodiscard]] bool IsControllerConnected() const noexcept
		{
			return ControllerStatus::IsControllerConnected(m_mouseSettingsPack.playerInfo.player_id);
		}
		///// <summary><c>IsRunning()</c> returns true if both the <c>STMouseMapping</c> is enabled and the <c>STRunner</c> thread pool thread is running. </summary>
		/////	<returns>true if STMapping obj and STRunner obj are both running, or false if called during destruction.</returns>
		//[[nodiscard]] bool IsRunning() const noexcept
		//{
		//	if (m_stmapper == nullptr || m_statRunner == nullptr)
		//		return false;
		//	return m_statRunner->IsRunning();
		//}
		/// <summary><c>IsRunning()</c> returns true if the <c>STMouseMapping</c> is enabled. </summary>
		///	<returns>true if STMapping obj running, or false if called during destruction.</returns>
		[[nodiscard]] bool IsRunning() const noexcept
		{
			//TODO bug probably here.
			//return !m_statRunner->CreateThread();
			if (m_stmapper == nullptr || m_statRunner == nullptr)
				return false;
			return true;
		}
		/// <summary><c>Start()</c> enables processing on the function objects added to the <c>STRunner</c> thread pool.
		/// Does not start the <c>STRunner</c> thread! </summary>
		void Start() noexcept
		{
			if(m_stmapper != nullptr)
				m_stmapper->Start();
		}
		/// <summary><c>Stop()</c> toggles the Enabled status of the <c>STMouseMapping</c> data wrapper instance to "false". </summary>
		void Stop() noexcept
		{
			if(m_stmapper != nullptr)
				m_stmapper->Stop();
		}
	private:
	};
}

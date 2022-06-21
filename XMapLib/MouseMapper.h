#pragma once
#include "stdafx.h"
#include "ThumbstickToDelay.h"
#include "ThumbDzInfo.h"
#include "Utilities.h"
#include "ControllerStatus.h"
#include "STRunner.h"
#include "STMouseMapping.h"

namespace sds
{
	/// <summary><c>MouseMapper</c> [[Main class for external use]] Handles achieving smooth, expected mouse movements.
	/// The class has an internal <c>STMouseMapping</c> instance that fetches controller information via
	/// the OS API call wrapper class <c>MousePoller</c>. It also has public functions for getting and
	/// setting the sensitivity as well as setting which thumbstick to use. </summary>
	///	<remarks>If a <code>std::shared_ptr{STRunner}</code> is not passed into the constructor,
	///	one will be created for use. </remarks>
	class MouseMapper
	{
		// Logging function, optionally passed in by the user.
		const Utilities::XELogPtr m_logFn;
		// Thread pool class, our work functors get added to here and called in succession on a separate thread
		// for performance reasons.
		SharedPtrType<STRunner> m_statRunner;
		// Mouse settings pack, needed for iscontrollerconnected func args and others.
		const MouseSettingsPack m_mouseSettingsPack;
		// data wrapper class, added to thread pool STRunner, performs the polling, calculation, and mouse moving.
		SharedPtrType<STMouseMapping> m_stmapper;

		//StickMap m_stickmap_info{ StickMap::NEITHER_STICK };
		//int m_mouse_sensitivity{ MouseSettings::SENSITIVITY_DEFAULT };
	public:
		/// <summary>Ctor allows setting a custom MousePlayerInfo</summary>
		MouseMapper(const SharedPtrType<STRunner> &statRunner = nullptr, 
			const MouseSettingsPack settings = {}, 
			Utilities::XELogPtr logFn = nullptr)
		: m_logFn(logFn),
		  m_statRunner(statRunner),
		  m_mouseSettingsPack(settings)
	{
			//TODO fix construction of stmapper, add sensitivity and stick args to ctor here
			auto LogIfAvailable = [&](const char* msg)
			{
				if (m_logFn != nullptr)
					m_logFn(msg);
			};
			// if statrunner is nullptr, log error and return
			if (m_statRunner == nullptr)
			{
				LogIfAvailable("Exception: In MouseMapper::MouseMapper(...): STRunner shared_ptr was null!");
				return;
			}
			// otherwise, add the mouse mapping obj to the STRunner thread for processing 
			m_stmapper = MakeSharedSmart<STMouseMapping>(settings.settings.SENSITIVITY_DEFAULT, StickMap::RIGHT_STICK, settings, logFn);
			m_statRunner->AddDataWrapper(m_stmapper);
		}
		MouseMapper(const MouseMapper& other) = delete;
		MouseMapper(MouseMapper&& other) = delete;
		MouseMapper& operator=(const MouseMapper& other) = delete;
		MouseMapper& operator=(MouseMapper&& other) = delete;
		~MouseMapper() = default;

		/// <summary>Use this function to establish one stick or the other as the one controlling the mouse movements.
		/// Set to NEITHER_STICK for no thumbstick mouse movement. Options are RIGHT_STICK, LEFT_STICK, NEITHER_STICK
		///	This will start processing if the stick is something other than "NEITHER"
		///	**Arbitrary values outside of the enum constants will not be processed successfully.**</summary>
		/// <param name="info"> a StickMap enum</param>
		void SetStick(const StickMap info) noexcept
		{
			m_stmapper->SetStick(info);
		}
		/// <summary><c>GetStick()</c> A getter for the current <code>StickMap</code> enum data member.</summary>
		/// <returns><c>StickMap</c> enum class denoting which controller thumbstick is set for processing.</returns>
		StickMap GetStick() const
		{
			return m_stmapper->GetStick();
		}
		/// <summary><c>SetSensitivity(...)</c> Setter for sensitivity value.</summary>
		/// <returns> returns a <c>std::string</c> containing an error message
		/// if there is an error, empty string otherwise. </returns>
		std::string SetSensitivity(const int new_sens) noexcept
		{
			if (!m_mouseSettingsPack.settings.IsValidSensitivityValue(new_sens))
			{
				return "Error in sds::XinMouseMapper::SetSensitivity(), int new_sens out of range.";
			}
			m_stmapper->SetSensitivity(new_sens);
			//m_mouse_sensitivity = new_sens;
			return "";
		}
		/// <summary><c>GetSensitivity()</c> Getter for sensitivity value</summary>
		[[nodiscard]] int GetSensitivity() const noexcept
		{
			return m_stmapper->GetSensitivity();
		}
		///<summary><c>IsControllerConnected()</c> returns true if the current <c>player_id</c> reports a controller connected.</summary>
		[[nodiscard]] bool IsControllerConnected() const noexcept
		{
			return ControllerStatus::IsControllerConnected(m_mouseSettingsPack.playerInfo.player_id);
		}
		///<summary><c>IsRunning()</c> returns true if both the <c>STMouseMapping</c> is enabled and the <c>STRunner</c> thread pool thread is running. </summary>
		[[nodiscard]] bool IsRunning() const noexcept
		{
			return m_stmapper->IsEnabled() && m_statRunner->IsRunning();
		}
		/// <summary><c>Start()</c> enables processing on the function objects added to the <c>STRunner</c> thread pool.
		/// Does not start the <c>STRunner</c> thread! </summary>
		void Start() noexcept
		{
			m_stmapper->Start();
		}
		/// <summary><c>Stop()</c> toggles the Enabled status of the <c>STMouseMapping</c> data wrapper instance to "false". </summary>
		void Stop() noexcept
		{
			m_stmapper->Stop();
		}
	private:
	};
}

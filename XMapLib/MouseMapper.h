#pragma once
#include "stdafx.h"
#include "MouseMoveThread.h"
#include "ThumbstickToDelay.h"
#include "MouseInputPoller.h"
#include "ThumbDzInfo.h"
#include "Utilities.h"
namespace sds
{
	/// <summary>
	/// Handles achieving smooth, expected mouse movements.
	/// This class starts a running thread that is used to process the XINPUT_STATE structure and use those values to determine if it should move the mouse cursor, and if so how much.
	/// The class has an internal MouseInputPoller() instance that fetches controller information via the XInputGetState() function and associated lib.
	/// It also has public functions for getting and setting the sensitivity as well as setting which thumbstick to use.
	/// </summary>
	class MouseMapper
	{
		using InternalType = int;
		using LambdaRunnerType = sds::CPPRunnerGeneric<InternalType>;
		using ScopedLockType = LambdaRunnerType::ScopedLockType;
		std::atomic<StickMap> m_stickmap_info{ StickMap::NEITHER_STICK };
		std::atomic<SHORT> m_thread_x{ 0 };
		std::atomic<SHORT> m_thread_y{0};
		std::atomic<int> m_mouse_sensitivity{ MouseSettings::SENSITIVITY_DEFAULT };
		sds::MousePlayerInfo m_local_player{};
		MouseSettings m_ms{};
		sds::MouseInputPoller m_poller{};
		std::unique_ptr<LambdaRunnerType> m_workThread{};
		void InitWorkThread() noexcept
		{
			m_workThread =
				std::make_unique<LambdaRunnerType>
				([this](const auto stopCondition, const auto mut, auto protectedData) { workThread(stopCondition, mut, protectedData); });
		}
	public:
		/// <summary>Ctor for default configuration</summary>
		MouseMapper() noexcept
		{
			InitWorkThread();
		}
		/// <summary>Ctor allows setting a custom MousePlayerInfo</summary>
		explicit MouseMapper(const sds::MousePlayerInfo& player, MouseSettings ms = {}) noexcept
		: m_local_player(player),
		m_ms(ms)
		{
			InitWorkThread();
		}
		explicit MouseMapper(sds::MousePlayerInfo&& player, MouseSettings ms = {}) noexcept
		: m_local_player(player),
		m_ms(ms)
		{
			InitWorkThread();
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
			if (m_stickmap_info != info)
			{
				Stop();
				m_stickmap_info = info;
				if (info != StickMap::NEITHER_STICK)
				{
					Start();
				}
				else
				{
					Stop();
				}
			}
		}
		StickMap GetStick() const
		{
			return m_stickmap_info;
		}
		/// <summary>Setter for sensitivity value.</summary>
		/// <returns> returns a std::string containing an error message
		/// if there is an error, empty string otherwise. </returns>
		std::string SetSensitivity(const int new_sens) noexcept
		{
			if (!MouseSettings::IsValidSensitivityValue(new_sens))
			{
				return "Error in sds::XinMouseMapper::SetSensitivity(), int new_sens out of range.";
			}
			Stop();
			m_mouse_sensitivity = new_sens;
			Start();
			return "";
		}
		/// <summary>Getter for sensitivity value</summary>
		[[nodiscard]] int GetSensitivity() const noexcept
		{
			return m_mouse_sensitivity;
		}
		[[nodiscard]] bool IsControllerConnected() const noexcept
		{
			return m_poller.IsControllerConnected();
		}
		[[nodiscard]] bool IsRunning() const noexcept
		{
			bool workRunning = false;
			if (m_workThread != nullptr)
				workRunning = m_workThread->IsRunning();
			return m_poller.IsRunning() && workRunning;
		}
		void Start() noexcept
		{
			m_poller.Start();
			if(m_workThread != nullptr)
				m_workThread->StartThread();
		}
		void Stop() noexcept
		{
			m_poller.Stop();
			if(m_workThread != nullptr)
				m_workThread->StopThread();
		}
	protected:
		/// <summary>Worker thread, protected visibility, gets updated data from ProcessState() function to use.
		/// Accesses the std::atomic m_thread_x and m_thread_y members. chrono lib instances may throw exceptions.</summary>
		void workThread(const auto stopCondition, const auto, auto)
		{
			using sds::Utilities::ToA;
			ThumbstickToDelay delayCalculator(this->GetSensitivity(), m_stickmap_info);
			ThumbDzInfo deadzoneInfoCalculator(m_local_player, m_stickmap_info);
			MouseMoveThread mover;
			//thread main loop
			while (!(*stopCondition))
			{
				ProcessState(m_poller.GetUpdatedState());
				//store the returned delay from axisthread for each axis
				//then pass the delays on to MouseMoveThread, along with some information like
				//is X or Y negative, and if the axis is moving
				const SHORT tx = m_thread_x;
				const SHORT ty = m_thread_y;
				const auto [xDelay, yDelay] = delayCalculator.GetDelaysFromThumbstickValues(tx, ty);
				const bool ixp = tx > 0;
				const bool iyp = ty > 0;
				const std::pair<bool,bool> isBeyondDz = deadzoneInfoCalculator.IsBeyondDeadzone(tx, ty);
				mover.UpdateState(xDelay, yDelay, ixp, iyp, isBeyondDz.first, isBeyondDz.second);
				//debugging info
				//std::stringstream ss;
				//ss << "[X]:[" << xDelay << "] [Y]:[" << yDelay << "]\n";
				//Utilities::LogError(ss.str().c_str());
				std::this_thread::sleep_for(std::chrono::milliseconds(4));
			}
		}
	private:
		/// <summary>Updates local atomic values with XINPUT_STATE info from the MouseInputPoller</summary>
		void ProcessState(const XINPUT_STATE& state) noexcept
		{
			if (m_stickmap_info == StickMap::NEITHER_STICK)
				return;
			if (m_stickmap_info == StickMap::RIGHT_STICK)
			{
				//Give worker thread new values.
				m_thread_x = state.Gamepad.sThumbRX;
				m_thread_y = state.Gamepad.sThumbRY;
			}
			else if(m_stickmap_info == StickMap::LEFT_STICK)
			{
				m_thread_x = state.Gamepad.sThumbLX;
				m_thread_y = state.Gamepad.sThumbLY;
			}
		}
	};
}

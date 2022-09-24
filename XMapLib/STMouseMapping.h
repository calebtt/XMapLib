#pragma once
#include "stdafx.h"
#include "ThumbstickToDelay.h"
#include "MousePoller.h"
#include "MouseMover.h"
#include "MouseMoveInfoPacket.h"

namespace sds
{
	/// <summary> <c>STMouseMapping</c> It's a wrapper around <c>MousePoller</c> , <c>ThumbstickToDelay</c> , and <c>MouseMover</c> that is added to the
	/// <c>STRunner</c> thread pool. <para>Used in main class for use <c>MouseMapper</c>.</para></summary>
	/// <remarks>Mouse simulation function object that polls for controller input and processes it as mouse movements. It is ran on the STRunner thread pool.
	///	Remember that the <c>m_is_enabled</c> member of the base (<c>STDataWrapper</c>) toggles on/off the processing of operator()()</remarks>
	template<class LogFnType = std::function<void(std::string)>>
	struct STMouseMapping
	{
	private:
		// mouse sensitivity value
		std::atomic<int> m_mouseSensitivity;
		// which controller thumbstick is chosen
		std::atomic<StickMap> m_stickmap_info{ StickMap::RIGHT_STICK };
		// Mouse settings aggregate struct
		const MouseSettingsPack m_msp;
		// Polling object, to retrieve MousePoller::ThumbstickStateWrapper wrapped thumbstick values.
		MousePoller m_mousePoller;
		// Delay calculator, uses ThumbstickStateWrapper wrapped thumbstick values to calculate microsecond delay values.
		ThumbstickToDelay<LogFnType> m_delayCalc;
		// Mouse mover object, performs the actual mouse move with info from poller and calc.
		MouseMover m_mover;
		// Deadzone calculator, provides information such as "is the axis value beyond the deadzone?".
		ThumbDzInfo m_dzInfo;
		std::atomic<bool> m_is_enabled{ true };
	public:
		~STMouseMapping()
		{
			Stop();
		}
		// Giant constructor that needs lots of information.
		STMouseMapping(const int sensitivity,
			const StickMap whichStick,
			const MouseSettingsPack msp = {},
			const LogFnType fn = nullptr)
		: m_mouseSensitivity(sensitivity),
		m_stickmap_info(whichStick),
		m_msp(msp),
		m_mousePoller(fn),
		m_delayCalc(sensitivity, whichStick, msp, fn),
		m_dzInfo(whichStick, msp, fn)
		{
			
		}
		/// <summary>Worker thread function called in a loop on the STRunner's thread. Updates with delays which are used by the MouseMoveThread.
		/// The MouseMoveThread is time critical and not included in the thread pool maintained by an STRunner instance. </summary>
		void operator()()
		{
			// Delay update func.
			using sds::Utilities::ToA;
			if (m_is_enabled)
			{
				// get state from poller.
				const ThumbStateWrapper tempState = m_mousePoller.GetUpdatedState(m_msp.playerInfo.player_id);
				// choose correct x,y pair
				const auto xValue = m_stickmap_info == StickMap::RIGHT_STICK ? tempState.RightThumbX : tempState.LeftThumbX;
				const auto yValue = m_stickmap_info == StickMap::RIGHT_STICK ? tempState.RightThumbY : tempState.LeftThumbY;
				// calculate delays based on cartesian x,y pair
				const std::pair<int, int> delayPair = m_delayCalc.GetDelaysFromThumbstickValues(xValue, yValue);
				// get info for which cartesian values are beyond their respective deadzone
				const std::pair<bool, bool> activatedPair = m_dzInfo.IsBeyondDeadzone(xValue, yValue);
				// build mouse move info packet
				const MouseMoveInfoPacket mmip
				{
					.x_delay = std::get<0>(delayPair),
					.y_delay = std::get<1>(delayPair),
					.is_x_positive = (xValue > 0),
					.is_y_positive = (yValue > 0),
					.is_beyond_dz_x = activatedPair.first,
					.is_beyond_dz_y = activatedPair.second
				};
				// [send it!]
				m_mover.PerformMove(mmip);
			}
		}
		[[nodiscard]] bool IsRunning() const
		{
			return m_is_enabled;
		}
		void Start() noexcept
		{
			this->m_is_enabled = true;
		}
		void Stop() noexcept
		{
			this->m_is_enabled = false;
		}
		void SetSensitivity(const int newSens) noexcept
		{
			m_delayCalc.SetSensitivity(newSens);
		}
		int GetSensitivity() const noexcept
		{
			return m_delayCalc.GetSensitivity();
		}
		/// <summary> Sets the controller stick for processing, will enable
		///	processing if not NEITHER_STICK </summary>
		void SetStick(StickMap newStick) noexcept
		{
			m_stickmap_info = newStick;
			m_delayCalc.SetStick(newStick);
			//enable or disable processing
			m_is_enabled = m_stickmap_info != StickMap::NEITHER_STICK;
		}
		StickMap GetStick() const noexcept
		{
			return m_delayCalc.GetStick();
		}
	private:

	};
}
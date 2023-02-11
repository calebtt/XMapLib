#pragma once
#include "LibIncludes.h"
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
		MouseSettingsPack m_msp;
		// Polling object, to retrieve MousePoller::ThumbstickStateWrapper wrapped thumbstick values.
		MousePoller m_mousePoller;
		// Delay calculator, uses ThumbstickStateWrapper wrapped thumbstick values to calculate microsecond delay values.
		ThumbstickToDelay<ReadRadiusScaleValues, LogFnType> m_delayCalc;
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
		m_delayCalc(sensitivity, msp, fn),
		m_dzInfo(whichStick, msp)
		{
			
		}

		void getDelaysAndSend(const ThumbStateWrapper& tempState) noexcept
		{
			// choose correct x,y pair
			const auto xValue = m_stickmap_info == StickMap::RIGHT_STICK ? tempState.RightThumbX : tempState.LeftThumbX;
			const auto yValue = m_stickmap_info == StickMap::RIGHT_STICK ? tempState.RightThumbY : tempState.LeftThumbY;
			// calculate delays based on cartesian x,y pair
			const auto[xDelay, yDelay] = m_delayCalc.GetDelaysFromThumbstickValues(xValue, yValue);
			// get info for which cartesian values are beyond their respective deadzone
			const auto[xAct, yAct] = m_dzInfo.IsBeyondDeadzone(xValue, yValue);
			// build mouse move info packet
			const MouseMoveInfoPacket mmip
			{
				.x_delay = xDelay,
				.y_delay = yDelay,
				.is_x_positive = (xValue > 0),
				.is_y_positive = (yValue > 0),
				.is_beyond_dz_x = xAct,
				.is_beyond_dz_y = yAct
			};
			// [send it!]
			m_mover.PerformMove(mmip);
		}

		/// <summary>Worker thread function called in a loop on the STRunner's thread. Updates with delays which are used by the MouseMoveThread.
		/// The MouseMoveThread is time critical and not included in the thread pool maintained by an STRunner instance. </summary>
		void operator()()
		{
			// Delay update func.
			using sds::Utilities::ToA;
			if (m_is_enabled)
			{
				// get state from poller, if success then get delays and send.
				if (const auto tempState = m_mousePoller.GetUpdatedState(m_msp.playerInfo.player_id); tempState.has_value())
					getDelaysAndSend(tempState.value());
			}
		}
		[[nodiscard]]
		bool IsRunning() const noexcept
		{
			return m_is_enabled;
		}
		void Start() noexcept
		{
			m_is_enabled = true;
		}
		void Stop() noexcept
		{
			m_is_enabled = false;
		}
		void SetSensitivity(const int newSens) noexcept
		{
			m_delayCalc.SetSensitivity(newSens);
		}
		int GetSensitivity() const noexcept
		{
			return m_delayCalc.GetSensitivity();
		}
		/**
		 * \brief Sets the controller stick for processing, will enable processing if not NEITHER_STICK
		 * \param newStick Stick for which the processing should occur.
		 */
		void SetStick(StickMap newStick) noexcept
		{
			m_stickmap_info = newStick;
			//enable or disable processing
			const bool toEnable = m_stickmap_info != StickMap::NEITHER_STICK;
			m_delayCalc.SetActive(toEnable);
			m_is_enabled = toEnable;
		}
		auto GetStick() const noexcept -> StickMap
		{
			return m_stickmap_info;
		}
	private:

	};
}
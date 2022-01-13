#pragma once
#include "MouseSettings.h"
#include "Utilities.h"

namespace sds
{
	/// <summary>
	/// A singular thread responsible for sending mouse movements using
	///	two different axis delay values being updated while running.
	/// </summary>
	class MouseMoveThread
	{
		using InternalType = int;
		using LambdaRunnerType = sds::CPPLambdaRunner<InternalType>;
		using lock = LambdaRunnerType::ScopedLockType;
		std::atomic<size_t> m_x_axis_delay = 1;
		std::atomic<size_t> m_y_axis_delay = 1;
		std::atomic<bool> m_is_x_moving = false;
		std::atomic<bool> m_is_y_moving = false;
		std::atomic<bool> m_is_x_positive = false;
		std::atomic<bool> m_is_y_positive = false;
		std::unique_ptr<LambdaRunnerType> m_workThread;
		void InitWorkThread() noexcept
		{
			m_workThread =
				std::make_unique<LambdaRunnerType>
				([this](sds::LambdaArgs::LambdaArg1& stopCondition, sds::LambdaArgs::LambdaArg2& mut, auto& protectedData) { workThread(stopCondition, mut, protectedData); });
		}
	public:
		MouseMoveThread()
		{
			InitWorkThread();
			m_workThread->StartThread();
		}
		~MouseMoveThread()
		{
			m_workThread->StopThread();
		}
		MouseMoveThread(const MouseMoveThread& other) = delete;
		MouseMoveThread(MouseMoveThread&& other) = delete;
		MouseMoveThread& operator=(const MouseMoveThread& other) = delete;
		MouseMoveThread& operator=(MouseMoveThread&& other) = delete;
		/// <summary>
		/// Called to update mouse mover thread with new microsecond delay values,
		///	and whether the axis to move should move positive or negative.
		/// </summary>
		void UpdateState(const size_t x, const size_t y, const bool isXPositive, const bool isYPositive, const bool isXMoving, const bool isYMoving)
		{
			m_x_axis_delay = x;
			m_y_axis_delay = y;
			m_is_x_positive = isXPositive;
			m_is_y_positive = isYPositive;
			m_is_x_moving = isXMoving;
			m_is_y_moving = isYMoving;
		}
	protected:
		void workThread(sds::LambdaArgs::LambdaArg1& stopCondition, sds::LambdaArgs::LambdaArg2&, InternalType&) const noexcept
		{
			using namespace std::chrono;
			Utilities::SendMouseInput keySend;
			Utilities::DelayManager xTime(MouseSettings::MICROSECONDS_MAX);
			Utilities::DelayManager yTime(MouseSettings::MICROSECONDS_MAX);
			//A loop with no delay, that checks each delay value
			//against a timepoint, and performs the move for that axis if it beyond the timepoint
			//and in that way, will perform the single pixel move with two different variable time delays.
			bool isXM = false;
			bool isYM = false;
			while (!stopCondition)
			{
				const bool isXPos = m_is_x_positive;
				const bool isYPos = m_is_y_positive;
				const size_t xDelay = m_x_axis_delay;
				const size_t yDelay = m_y_axis_delay;
				const bool isXPast = xTime.IsElapsed();
				const bool isYPast = yTime.IsElapsed();
				if (isXM || isYM)
				{
					int xVal = 0;
					int yVal = 0;
					if (isXPast && m_is_x_moving)
					{
						xVal = (isXPos ? MouseSettings::PIXELS_MAGNITUDE : (-MouseSettings::PIXELS_MAGNITUDE));
						xTime.Reset(xDelay);
					}
					if (isYPast && m_is_y_moving)
					{
						yVal = (isYPos ? -MouseSettings::PIXELS_MAGNITUDE : (MouseSettings::PIXELS_MAGNITUDE)); // y is inverted
						yTime.Reset(yDelay);
					}
					keySend.SendMouseMove(xVal, yVal);
				}
				isXM = m_is_x_moving;
				isYM = m_is_y_moving;
			}
		}

	};
}

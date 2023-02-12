#pragma once
#include "LibIncludes.h"
#include "MouseSettingsPack.h"

namespace sds
{
	/// <summary> Manages checking if cartesian thumbstick values are beyond a given
	///	deadzone value, and holds internal state regarding the deadzone being activated,
	///	as it will scale down after activation. </summary>
	class ThumbDzInfo
	{
		using PolarMagInt = decltype(MouseSettings::DEADZONE_DEFAULT);
		using AltDzFloat = decltype(MouseSettings::ALT_DEADZONE_MULT_DEFAULT);
		bool m_is_deadzone_activated{ false };
		PolarMagInt m_polar_magnitude_deadzone;
		// TODO, a single errant value returned from the hardware will turn alt-deadzone mode on incorrectly.
		// It will be more noticeable with a small multiplier value. Could implement a count or something.
		AltDzFloat m_alt_deadzone_multiplier;
		MouseSettingsPack m_msp;
	private:
		/**
		 * \brief Used to validate polar deadzone arg value
		 * \param sm current stick it's activated for
		 * \param msp mouse settings pack, needs a dz validation fn and some dz vals
		 * \return returns a validated deadzone value
		 */
		[[nodiscard]]
		static
		constexpr
		auto ValidatePolarDz(
			const StickMap sm,
			const MouseSettingsPack msp = {})  noexcept
		{
			//error checking deadzone arg range, because it might crash the program if the
			//delay returned is some silly value
			const int cdz = sm == StickMap::LEFT_STICK ? msp.playerInfo.left_polar_dz : msp.playerInfo.right_polar_dz;
			if (msp.settings.IsValidDeadzoneValue(cdz))
				return cdz;
			return msp.settings.DEADZONE_DEFAULT;
		}
	public:
		explicit ThumbDzInfo(const StickMap sm, const MouseSettingsPack msp = {})
		:
		m_polar_magnitude_deadzone(ValidatePolarDz(sm, msp)),
		m_alt_deadzone_multiplier(msp.settings.ALT_DEADZONE_MULT_DEFAULT),
		m_msp(msp)
		{ }

		/**
		 * \brief Takes a cartesian value and returns true if equal or over deadzone
		 * \param cartesianX x stick value in cartesian space
		 * \param cartesianY y stick value in cartesian space
		 * \return returns a pair of bools wherein the first bool is 'x' the second is 'y'
		 */
		[[nodiscard]]
		auto IsBeyondDeadzone(const int cartesianX, const int cartesianY) noexcept
			-> std::pair<bool, bool>
		{
			using std::abs;
			using Utilities::ToA;
			// Get polar X and Y magnitudes, and theta angle from cartesian X and Y
			const auto fullInfo = PolarTransformMag<>{ cartesianX, -cartesianY, m_msp.settings.PolarRadiusValueMax }.get();
			const auto& [xPolarMag, yPolarMag] = fullInfo.adjusted_magnitudes;
			//get positive polar values
			const auto absX = abs(xPolarMag);
			const auto absY = abs(yPolarMag);
			// Test for deadzone
			const auto dzCurrent = GetDeadzoneCurrentValue();
			assert(dzCurrent > 0);
			const bool isBeyondX = absX >= dzCurrent;
			const bool isBeyondY = absY >= dzCurrent;
			if (isBeyondX || isBeyondY)
				m_is_deadzone_activated = true;

			//[&]()
			//{
			//	std::cerr << std::format("xPolarMag:{0}, yPolarMag:{1}, absX:{2}, absY:{3} dzCurrent:{4}, isBeyondX:{5}, isBeyondY{6}\n",
			//		xPolarMag, yPolarMag, absX, absY, dzCurrent, isBeyondX, isBeyondY);
			//}();

			return { isBeyondX, isBeyondY };
		}

		/**
		 * \brief Returns the polar rad dz, or the alternate if the dz is already activated.
		 */
		[[nodiscard]]
		constexpr
		int GetDeadzoneCurrentValue() const noexcept
		{
			using sds::Utilities::ToA;
			if (m_is_deadzone_activated)
				return ToA<int>(ToA<float>(m_polar_magnitude_deadzone) * m_alt_deadzone_multiplier);
			return m_polar_magnitude_deadzone;
		}
	};
}


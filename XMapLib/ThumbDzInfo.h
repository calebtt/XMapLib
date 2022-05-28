#pragma once
#include "stdafx.h"
#include "Arithmetic.h"
#include "PolarCalc.h"

namespace sds
{
	/// <summary> Manages checking is cartesian thumbstick values are beyond a given
	///	deadzone value, and holds internal state regarding the deadzone being activated
	///	as it will scale down after activation. </summary>
	class ThumbDzInfo
	{
		using PolarMagInt = decltype(MouseSettings::DEADZONE_DEFAULT);
		using AltDzFloat = decltype(MouseSettings::ALT_DEADZONE_MULT_DEFAULT);
		bool m_is_deadzone_activated{ false };
		const PolarMagInt m_polar_magnitude_deadzone;
		const AltDzFloat m_alt_deadzone_multiplier;
		PolarCalc m_pc;
	private:
		/// <summary> Used to validate polar deadzone arg value. </summary>
		[[nodiscard]] static constexpr auto ValidatePolarDz(
			const MousePlayerInfo& mpi,
			const StickMap sm, 
			const MouseSettings &ms)  noexcept
		{
			//error checking deadzone arg range, because it might crash the program if the
			//delay returned is some silly value
			const int cdz = sm == StickMap::LEFT_STICK ? mpi.left_polar_dz : mpi.right_polar_dz;
			if (ms.IsValidDeadzoneValue(cdz))
				return cdz;
			return ms.DEADZONE_DEFAULT;
		}
		/// <summary> Used to validate alt deadzone multiplier arg. </summary>
		[[nodiscard]] static constexpr auto ValidateAltMultiplier(const MouseSettings &ms)  noexcept
		{
			return ms.ALT_DEADZONE_MULT_DEFAULT;
		}
	public:
		ThumbDzInfo(const MousePlayerInfo &mpi, const StickMap sm, const MouseSettings& ms = {})
		: m_polar_magnitude_deadzone(ValidatePolarDz(mpi, sm, ms)),
		m_alt_deadzone_multiplier(ValidateAltMultiplier(ms)),
		m_pc(ms.ThumbstickValueMax)
		{ }
		///<summary>Takes a cartesian value and returns true if equal or over deadzone. </summary>
		[[nodiscard]] std::pair<bool,bool> IsBeyondDeadzone(const int cartesianX, const int cartesianY) noexcept
		{
			using Utilities::ToA;
			using Utilities::ConstAbs;
			// Get polar X and Y magnitudes, and theta angle from cartesian X and Y
			const auto fullInfo = m_pc.ComputePolarCompleteInfo(cartesianX, -cartesianY);
			const auto& [xPolarMag, yPolarMag] = fullInfo.adjusted_magnitudes;
			//get positive polar values
			const auto absX = ConstAbs(xPolarMag);
			const auto absY = ConstAbs(yPolarMag);
			// Test for deadzone
			const auto dzCurrent = GetDeadzoneCurrentValue();
			const bool isBeyondX = absX >= dzCurrent;
			const bool isBeyondY = absY >= dzCurrent;
			if (isBeyondX || isBeyondY)
				m_is_deadzone_activated = true;
			return { isBeyondX, isBeyondY };
		}
		///<summary> Returns the polar rad dz, or the alternate if the dz is already activated.</summary>
		[[nodiscard]] constexpr int GetDeadzoneCurrentValue() const noexcept
		{
			using sds::Utilities::ToA;
			if (m_is_deadzone_activated)
				return ToA<int>(ToA<float>(m_polar_magnitude_deadzone) * m_alt_deadzone_multiplier);
			return m_polar_magnitude_deadzone;
		}
	};
}


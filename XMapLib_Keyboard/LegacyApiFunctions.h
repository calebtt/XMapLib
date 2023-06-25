#pragma once
#include "LibIncludes.h"

namespace sds
{
	/**
	 * \brief WARNING: Single Threaded!
	 * \param playerId Most commonly 0 for a single device connected.
	 * \return Platform/API specific state structure.
	 */
	[[nodiscard]]
	inline
	auto GetLegacyApiStateUpdate(const int playerId = 0) noexcept -> std::optional<XINPUT_STATE>
	{
		static XINPUT_STATE controllerState;
		controllerState = {};
		const auto resultCode = XInputGetState(playerId, &controllerState);
		if (resultCode == ERROR_SUCCESS)
			return controllerState;
		return {};
	}

	/**
	 * \brief WARNING: Single Threaded! Values of 1 in the returned buffer mean set.
	 * \param playerId Most commonly 0 for a single device connected.
	 * \return The button buffer type specified in KeyboardSettings, values of 0 mean not-set, while 1 means set.
	 */
	[[nodiscard]]
	inline
	auto GetWrappedLegacyApiStateUpdate(const int playerId = 0) noexcept -> std::optional<ControllerStateUpdateWrapper<>>
	{
		if (const auto controllerStateUpdate = GetLegacyApiStateUpdate(playerId))
		{
			return ControllerStateUpdateWrapper<>{*controllerStateUpdate};
		}
		return {};
	}
}
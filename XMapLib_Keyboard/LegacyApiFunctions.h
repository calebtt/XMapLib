#pragma once
#include "LibIncludes.h"

namespace sds
{
	/**
	 * \brief Calls the OS API function(s).
	 * \param playerId Most commonly 0 for a single device connected.
	 * \return Platform/API specific state structure.
	 */
	[[nodiscard]]
	inline
	auto GetLegacyApiStateUpdate(const int playerId = 0) noexcept -> XINPUT_STATE
	{
		XINPUT_STATE controllerState{};
		const auto resultCode = XInputGetState(playerId, &controllerState);
		if (resultCode == ERROR_SUCCESS)
			return controllerState;
		return {};
	}

	/**
	 * \brief Gets a wrapped controller state update.
	 * \param playerId Most commonly 0 for a single device connected.
	 * \return Wrapper for the controller state buffer.
	 */
	[[nodiscard]]
	inline
	auto GetWrappedLegacyApiStateUpdate(const int playerId = 0) noexcept -> ControllerStateUpdateWrapper<>
	{
		const auto controllerStateUpdate = GetLegacyApiStateUpdate(playerId);
		return ControllerStateUpdateWrapper<>{controllerStateUpdate};
	}
}
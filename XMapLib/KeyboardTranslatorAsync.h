#pragma once
#include "KeyboardPoller.h"
#include "stdafx.h"
#include "KeyboardTranslator.h"

namespace sds
{
	/// <summary> Wraps the synchronous logic of the class KeyboardTranslator for use
	///	asynchronously, it is a good arrangement to reduce the complexity. Possibly an 'adapter' pattern.
	/// <para>--Contains the logic for determining if a key press or mouse click should occur, uses sds::Utilities::SendKeyInput m_key_send to send the input.
	///	Function ProcessKeystroke(KeyboardPoller::KeyStateWrapper &stroke) is used to process a controller input structure. Handles key-repeat behavior as well.
	///	</para>
	///	</summary>
	class KeyboardTranslatorAsync
	{
		using MutexType = std::mutex;
		using LockType = std::scoped_lock <MutexType>;

		KeyboardTranslator m_translator;
		MutexType m_translatorMutex;
	public:
		explicit KeyboardTranslatorAsync(const KeyboardSettingsPack& ksp = {})
			: m_translator(ksp)
		{
		}
		/// <summary> Main function for use, processes <c>KeyboardPoller::KeyStateWrapper</c> into key presses and mouse clicks. </summary>
		///	<param name="stroke">A KeyStateWrapper containing controller button press information. </param>
		void ProcessKeystroke(const KeyboardPoller::KeyStateWrapper& stroke)
		{
			LockType tempLock(m_translatorMutex);
			m_translator.ProcessKeystroke(stroke);
		}
		/// <summary>Call this function to send key-ups for any in-progress key presses.</summary>
		void CleanupInProgressEvents()
		{
			LockType tempLock(m_translatorMutex);
			m_translator.CleanupInProgressEvents();
		}
		/// <summary>
		/// Adds a controller key map for processing. </summary>
		/// <param name="w">the controller to keystroke mapping detail</param>
		/// <returns>error message on error, empty string otherwise</returns>
		std::string AddKeyMap(const KeyboardKeyMap w)
		{
			LockType tempLock(m_translatorMutex);
			return m_translator.AddKeyMap(w);
		}
		/// <summary> Clears the internal controller button to key mappings. </summary>
		void ClearMaps() noexcept
		{
			CleanupInProgressEvents();
			LockType tempLock(m_translatorMutex);
			m_translator.ClearMaps();
		}
		/// <summary>Returns a copy of the internal controller button to key mappings. </summary>
		[[nodiscard]]
		std::vector<KeyboardKeyMap> GetMaps() noexcept
		{
			LockType tempLock(m_translatorMutex);
			return m_translator.GetMaps();
		}
	};
}


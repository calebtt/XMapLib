#pragma once
#include "LibIncludes.h"
#include <cassert>
#include "ThumbstickToDelay.h"
#include "ThumbDzInfo.h"
#include "STMouseMapping.h"

namespace sds
{
	/// <summary><c>MouseMapper</c> [[Main class for external use]] Handles achieving smooth, expected mouse movements.
	/// The class has an internal <c>STMouseMapping</c> instance that fetches controller information via
	/// the OS API call wrapper class <c>MousePoller</c>. It also has public functions for getting and
	/// setting the sensitivity as well as setting which thumbstick to use. </summary>
	///	<remarks>If a <code>std::shared_ptr{STRunner}</code> is not passed into the constructor,
	///	one will NOT be created for use. </remarks>
	template<typename ThreadUnit_t = imp::ThreadUnitPlusPlus>
	class MouseMapper
	{
		// Thread task pool class, our work functors get added to here and called in succession on a separate thread
		// for performance reasons.
		SharedPtrType<ThreadUnit_t> m_statRunner;
		// Mouse settings pack, needed for iscontrollerconnected func args and others.
		MouseSettingsPack m_mouseSettingsPack;
		// Combined object used for most of the mouse input processing, added to thread pool, performs the polling, calculation, and mouse moving.
		SharedPtrType<STMouseMapping<>> m_stmapper;
	public:
		/// <summary>Ctor allows setting a custom MouseSettingsPack</summary>
		MouseMapper(
			const SharedPtrType<ThreadUnit_t> &statRunner,
			const MouseSettingsPack &settings = {}
		)
		: m_statRunner(statRunner),
		  m_mouseSettingsPack(settings)
		{
			assert(m_statRunner != nullptr);
			if(m_statRunner == nullptr)
			{
				throw std::exception();
			}
			// Construct an STMouseMapping object, copy into class data member for lifetime control and access to it's functions.
			m_stmapper = MakeSharedSmart<STMouseMapping<>>(m_mouseSettingsPack.settings.SensitivityValue, m_mouseSettingsPack.settings.SelectedStick);
			// Get the task source, push a new task.
			auto tempSource = m_statRunner->GetTaskSource();
			// Note, we are capturing by value the shared_ptr!
			tempSource.PushInfiniteTaskBack([fnMapper = m_stmapper]()
				{
					fnMapper->operator()();
				});
			// Finally, set the task source with our additional task added to the end.
			m_statRunner->SetTaskSource(tempSource);
		}
		MouseMapper(const MouseMapper& other)
			: m_statRunner{other.m_statRunner},
			  m_mouseSettingsPack{other.m_mouseSettingsPack},
			  m_stmapper{other.m_stmapper} {}
		MouseMapper(MouseMapper&& other) noexcept
			: m_statRunner{std::move(other.m_statRunner)},
			  m_mouseSettingsPack{std::move(other.m_mouseSettingsPack)},
			  m_stmapper{std::move(other.m_stmapper)} {}
		auto operator=(MouseMapper other) -> MouseMapper&
		{
			using std::swap;
			swap(*this, other);
			return *this;
		}
		~MouseMapper() = default;
	public:
		/// <summary> Returns a copy of the shared_ptr to the STRunner thread instance. May be null. </summary>
		auto GetThreadUnit() const noexcept
		{
			return m_statRunner;
		}
		/// <summary>Use this function to establish one stick or the other as the one controlling the mouse movements.
		/// Set to NEITHER_STICK for no thumbstick mouse movement. Options are RIGHT_STICK, LEFT_STICK, NEITHER_STICK
		///	This will start processing if the stick is something other than "NEITHER" </summary>
		///	<remarks>**Arbitrary values outside of the enum constants will not be processed successfully.**</remarks>
		/// <param name="info"> a StickMap enum</param>
		void SetStick(const StickMap info) const noexcept
		{
			if(m_stmapper != nullptr)
				m_stmapper->SetStick(info);
		}
		/// <summary><c>GetStick()</c> A getter for the current <code>StickMap</code> enum data member.</summary>
		/// <returns><c>StickMap</c> enum class denoting which controller thumbstick is set for processing,
		/// or default constructed StickMap on error. </returns>
		auto GetStick() const noexcept -> std::optional<StickMap>
		{
			if (m_stmapper == nullptr)
				return {};
			return m_stmapper->GetStick();
		}
		/// <summary><c>SetSensitivity(...)</c> Setter for sensitivity value.</summary>
		/// <returns> returns a <c>std::string</c> containing an error message
		/// if there is an error, empty string otherwise. </returns>
		auto SetSensitivity(const int new_sens) const noexcept -> std::string
		{
			if (m_stmapper == nullptr)
				return "Error in sds::MouseMapper::SetSensitivity(...), m_stmapper is null.";
			if (!m_mouseSettingsPack.settings.IsValidSensitivityValue(new_sens))
			{
				return "Error in sds::MouseMapper::SetSensitivity(...), int new_sens out of range.";
			}
			m_stmapper->SetSensitivity(new_sens);
			return "";
		}
		/// <summary><c>GetSensitivity()</c> Getter for sensitivity value</summary>
		///	<returns>current sensitivity value, or -1 if there is no valid STMapping object (as in called during destruction).</returns>
		[[nodiscard]]
		int GetSensitivity() const noexcept
		{
			if (m_stmapper == nullptr)
				return -1;
			return m_stmapper->GetSensitivity();
		}
		/// <summary><c>IsControllerConnected()</c> returns true if the current <c>player_id</c> reports a controller connected.</summary>
		[[nodiscard]]
		bool IsControllerConnected() const noexcept
		{
			return ControllerStatus::IsControllerConnected(m_mouseSettingsPack.playerInfo.player_id);
		}
		/// <summary><c>IsRunning()</c> returns true if the <c>STMouseMapping</c> is enabled. </summary>
		///	<returns>true if STMapping obj running, or false if called during destruction.</returns>
		[[nodiscard]]
		bool IsRunning() const noexcept
		{
			if (m_stmapper == nullptr || m_statRunner == nullptr)
				return false;
			return m_stmapper->IsRunning();
		}
		/// <summary><c>Start()</c> enables processing on the function objects added to the <c>STRunner</c> thread pool.
		/// Does not start the <c>STRunner</c> thread! </summary>
		void Start() const noexcept
		{
			if(m_stmapper != nullptr)
				m_stmapper->Start();
		}
		/// <summary><c>Stop()</c> toggles the Enabled status of the <c>STMouseMapping</c> data wrapper instance to "false". </summary>
		void Stop() const noexcept
		{
			if(m_stmapper != nullptr)
				m_stmapper->Stop();
		}
	};
}

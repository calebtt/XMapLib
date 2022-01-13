#pragma once
#include "CPPLambdaBase.h"
namespace sds
{
	/// <summary>
	/// class for modifying data concurrently
	/// Instantiate with the type you would like to have mutex protected access to
	/// within a lambda on it's own thread.
	///	Template type "InternalData" must be default constructable!
	///	Instantiation requires a lambda of a certain form: function{void(atomic{bool}&, mutex&, InternalData&)}
	/// </summary>
	template <class InternalData>
	class CPPLambdaRunner : public CPPLambdaBase<InternalData>
	{
	public:
		using LambdaType = typename CPPLambdaBase<InternalData>::LambdaType;
		using ScopedLockType = typename CPPLambdaBase<InternalData>::ScopedLockType;

		CPPLambdaRunner(LambdaType lambdaToRun) : CPPLambdaBase<InternalData>((lambdaToRun)) { }
		/// <summary>
		/// Utility function to update the InternalData with mutex locking thread safety.
		/// </summary>
		/// <param name="state">InternalData obj to be copied to the internal one.</param>
		void UpdateState(const InternalData& state)
		{
			ScopedLockType tempLock(this->m_state_mutex);
			this->m_local_state = state;
		}
		/// <summary>
		/// Returns a copy of the internal InternalData obj with mutex locking thread safety.
		/// </summary>
		InternalData GetCurrentState()
		{
			ScopedLockType tempLock(this->m_state_mutex);
			return this->m_local_state;
		}
	};
}
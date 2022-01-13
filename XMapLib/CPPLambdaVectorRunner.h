#pragma once
#include "stdafx.h"
#include <thread>
#include <mutex>
#include <functional>
#include <vector>
#include "CPPLambdaRunner.h"

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
	class CPPLambdaVectorRunner : public CPPLambdaBase<InternalData>
	{
	public:
		using LambdaType = typename CPPLambdaBase<InternalData>::LambdaType;
		using ScopedLockType = typename CPPLambdaBase<InternalData>::ScopedLockType;

		CPPLambdaVectorRunner(LambdaType lambdaToRun) : CPPLambdaBase<InternalData>((lambdaToRun)) { }
		void AddState(const InternalData& state)
		{
			ScopedLockType tempLock(this->m_state_mutex);
			this->m_local_state.push_back(state);
		}
		auto GetAndClearCurrentStates()
		{
			ScopedLockType tempLock(this->m_state_mutex);
			auto temp = this->m_local_state;
			this->m_local_state.clear();
			return temp;
		}
	};
}
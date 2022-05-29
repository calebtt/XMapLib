#pragma once
#include "stdafx.h"
namespace sds
{
	/// <summary> Reads polar radius scaling values (for cartesian X,Y) from config file.
	///	The reason to do this is that the values from the controller are somewhat.. bad.
	///	With this configuration file it's possible to apply a scaling value based on the
	///	polar theta angle of the X,Y pair. This is perhaps the best solution available,
	///	and one that is likely mirrored in professional game applications--if you have
	///	ever seen a screen that asks the player to slowly rotate the thumbstick at max
	///	depression, that would be useful for building these scaling values.
	///	Thread-safe. </summary>
	class ReadRadiusScaleValues
	{
	public:
		using FloatType = double;
		using RangeType = std::vector<FloatType>;
		using MutexType = std::mutex;
		using LockType = std::scoped_lock<MutexType>;
	private:
		// Mutex shared between instances protecting access to the file resource,
		// and the cache container.
		inline static MutexType m_fileMutex;
		// Local shared cache of previous successful read of config file.
		inline static RangeType m_staticScalingValuesCopy;
		// Program settings struct for mouse infrastructure.
		const MouseSettings& m_mouseSettings;
	public:
		ReadRadiusScaleValues(const MouseSettings &ms) : m_mouseSettings(ms) { }
		/// <summary> Reads floating point values used to scale the thumbstick values to a proper
		///	circular polar radius. This is done because the values from the hardware are just plain bad.
		/// The index into the vector is the integral part of the polar theta times 100 for the first quadrant.
		/// Apply to all quadrants despite containing only values for quadrant 1 (positive values). </summary>
		/// <returns>Empty vector on error reading or parsing file. vector of double otherwise.</returns>
		[[nodiscard]] RangeType GetScalingValues()
		{
			// Mutex protected access, values should be cached.
			LockType tempLock(m_fileMutex);
			// Check for pre-existing successful read of config file (cache).
			if (!m_staticScalingValuesCopy.empty())
				return m_staticScalingValuesCopy;
			RangeType scalingValues;
			std::ifstream inFile(m_mouseSettings.SCALING_VALUES_FNAME.data());
			std::string currentLine;
			std::stringstream lineStream;
			while(std::getline(inFile, currentLine))
			{
				lineStream.clear();
				lineStream.str(currentLine);
				std::string token;
				FloatType value;
				//parse value
				lineStream >> token;
				lineStream >> value;
				//if line stringstream error state or contains more characters, fail.
				if(lineStream.bad() || !lineStream.eof())
					return {};
				//otherwise, emplace it into the vector
				scalingValues.emplace_back(value);
			}
			//if file stream in error state or not at the end of the file, fail.
			if(inFile.bad() || !inFile.eof())
				return {};
			m_staticScalingValuesCopy = scalingValues;
			return scalingValues;
		}
	};
}

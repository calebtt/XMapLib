#pragma once
#include "LibIncludes.h"
#include "ScalingDefaults.h"

namespace sds
{
	/// <summary>Reads polar radius scaling values (for cartesian X,Y) from config file.
	///	The reason to do this is that the values from the controller are somewhat.. bad.
	///	With this configuration file it's possible to apply a scaling value based on the
	///	polar theta angle of the X,Y pair. <para> This is perhaps the best solution available,
	///	and one that is likely mirrored in professional game applications--if you have
	///	ever seen a screen that asks the player to slowly rotate the thumbstick at max
	///	depression, that would be useful for building these scaling values.</para></summary>
	///	<remarks> Thread-safe</remarks>
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
		// Scaling values file name.
		std::string m_fileName;
	public:
		ReadRadiusScaleValues(const std::string &fName) : m_fileName(fName)
		{
			if (fName.empty())
			{
				LockType tempLock(m_fileMutex);
				m_staticScalingValuesCopy.assign(ScalingDefaults::DefaultScalingValues.cbegin(), ScalingDefaults::DefaultScalingValues.cend());
			}
		}
		/// <summary> Reads floating point values from a config file. It is used to scale the thumbstick values to a proper
		///	circular polar radius. This is done because the values from the hardware are just plain bad.
		/// The index into the vector is the integral part of (polar theta * 100) for the first quadrant.
		/// Extend to all quadrants despite containing only values for quadrant 1 (positive [x,y] values). </summary>
		/// <returns>Empty vector on error reading or parsing file. vector of double otherwise.</returns>
		[[nodiscard]]
		auto GetScalingValues() const -> RangeType
		{
			// Mutex protected access, values should be cached.
			LockType tempLock(m_fileMutex);
			// Check for pre-existing successful read of config file (cache).
			if (!m_staticScalingValuesCopy.empty())
				return m_staticScalingValuesCopy;
			RangeType scalingValues;
			std::ifstream inFile(m_fileName);
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

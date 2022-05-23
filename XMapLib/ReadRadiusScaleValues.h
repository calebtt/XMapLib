#pragma once
#include "stdafx.h"
namespace sds
{
	class ReadRadiusScaleValues
	{
		using MutexType = std::mutex;
		using LockType = std::scoped_lock<MutexType>;
		inline static MutexType m_fileMutex;
	public:
		/// <summary> Reads floating point values used to scale the thumbstick values to a proper
		///	circular polar radius. This is done because the values from the hardware are just plain bad.
		/// The index into the vector is the integral part of the polar theta times 100 for the first quadrant.
		/// Apply to all quadrants despite containing only values for quadrant 1 (positive values). </summary>
		/// <returns>Empty vector on error reading or parsing file. vector of double otherwise.</returns>
		static std::vector<double> GetScalingValues()
		{
			LockType tempLock(m_fileMutex);
			std::vector<double> scalingValues;
			std::ifstream inFile(MouseSettings::SCALING_VALUES_FNAME.data());
			std::string line;
			std::stringstream ss;
			while(std::getline(inFile, line))
			{
				ss.clear();
				ss.str(line);
				std::string token;
				double value;
				ss >> token;
				ss >> value;
				scalingValues.emplace_back(value);
				if(ss.bad() || !ss.eof())
				{
					return std::vector<double>{};
				}
			}
			if(inFile.bad() || !inFile.eof())
				return std::vector<double>{};
			return scalingValues;
		}
	};
}

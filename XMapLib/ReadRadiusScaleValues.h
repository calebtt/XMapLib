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
			std::string currentLine;
			std::stringstream lineStream;
			while(std::getline(inFile, currentLine))
			{
				lineStream.clear();
				lineStream.str(currentLine);
				std::string token;
				double value;
				//parse value
				lineStream >> token;
				lineStream >> value;
				//if line stringstream error state or contains more characters, fail.
				if(lineStream.bad() || !lineStream.eof())
					return std::vector<double>{};
				//otherwise, emplace it into the vector
				scalingValues.emplace_back(value);
			}
			//if file stream in error state or not at the end of the file, fail.
			if(inFile.bad() || !inFile.eof())
				return std::vector<double>{};
			return scalingValues;
		}
	};
}

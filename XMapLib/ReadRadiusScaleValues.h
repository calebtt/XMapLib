#pragma once
#include "stdafx.h"
namespace sds
{
	class ReadRadiusScaleValues
	{
	public:
		using FloatType = double;
		using RangeType = std::vector<FloatType>;
		using MutexType = std::mutex;
		using LockType = std::scoped_lock<MutexType>;
	private:
		inline static MutexType m_fileMutex;
	public:
		/// <summary> Reads floating point values used to scale the thumbstick values to a proper
		///	circular polar radius. This is done because the values from the hardware are just plain bad.
		/// The index into the vector is the integral part of the polar theta times 100 for the first quadrant.
		/// Apply to all quadrants despite containing only values for quadrant 1 (positive values). </summary>
		/// <returns>Empty vector on error reading or parsing file. vector of double otherwise.</returns>
		[[nodiscard]] static RangeType GetScalingValues()
		{
			LockType tempLock(m_fileMutex);
			RangeType scalingValues;
			std::ifstream inFile(MouseSettings::SCALING_VALUES_FNAME.data());
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
			return scalingValues;
		}
	};
}

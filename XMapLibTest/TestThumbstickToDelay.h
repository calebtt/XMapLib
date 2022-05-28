#pragma once
#include "pch.h"
#include "CppUnitTest.h"
#include "../XMapLib/ThumbstickToDelay.h"

namespace XMapLibTest
{
	using namespace Microsoft::VisualStudio::CppUnitTestFramework;
	TEST_CLASS(TestThumbstickToDelay)
	{
	public:
		TEST_METHOD(TestMagToDelay)
		{
			using namespace sds;
			using namespace std;
			Logger::WriteMessage("Begin TestMagToDelay()");
			ThumbstickToDelay ttd(100, StickMap::RIGHT_STICK);
			static constexpr size_t PolarRadiusMax{ 39'000 };
			auto DoCompareTest = [&](const auto firstVal, const auto secondVal, const auto firstShouldBe, const auto secondShouldBe)
			{
				//test polar radius magnitude values
				const auto res = ttd.ConvertMagnitudesToDelays(firstVal, secondVal);
				//assert pair returned are the same
				std::wstringstream ws;
				ws << "Element of pair returned at mag value X:" << firstVal << " not " << firstShouldBe << '\n';
				Assert::AreEqual(res.first, static_cast<decltype(res.first)>(firstShouldBe), ws.str().c_str());
				ws << "Element of pair returned at mag value Y:" << secondVal << " not " << secondShouldBe << '\n';
				Assert::AreEqual(res.second, static_cast<decltype(res.second)>(secondShouldBe), ws.str().c_str());
			};
			//test polar radius max magnitude values
			DoCompareTest(PolarRadiusMax, PolarRadiusMax, MouseSettings::MICROSECONDS_MIN, MouseSettings::MICROSECONDS_MIN);
			//test out of range values are truncated to fit
			DoCompareTest(PolarRadiusMax+100, PolarRadiusMax+100, MouseSettings::MICROSECONDS_MIN, MouseSettings::MICROSECONDS_MIN);
			//test some extreme values
			DoCompareTest(std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max(), MouseSettings::MICROSECONDS_MIN, MouseSettings::MICROSECONDS_MIN);
			DoCompareTest(std::numeric_limits<size_t>::min(), std::numeric_limits<size_t>::min(), MouseSettings::MICROSECONDS_MAX, MouseSettings::MICROSECONDS_MAX);
			Logger::WriteMessage("End TestMagToDelay()");
		}
		TEST_METHOD(TestGetScalingMultiplier)
		{
			using namespace sds;
			using namespace std;
			Logger::WriteMessage("Begin TestGetScalingMultiplier()");
			ThumbstickToDelay ttd(100, StickMap::RIGHT_STICK);
			static constexpr size_t PolarRadiusMax{ 39'000 };
			//test theta 0
			const auto zeroResult = ttd.GetScalingMultiplier(0);
			if(zeroResult < 0.0 || zeroResult > 1.0)
				Assert::Fail(L"Scaling mult out of bounds 0 to 1");
			//test theta 157 (first quadrant max)
			const auto maxResult = ttd.GetScalingMultiplier(157);
			if (maxResult < 0.0 || maxResult > 1.0)
				Assert::Fail(L"Scaling mult out of bounds 0 to 1");
			//test some out of bounds values ( should still return in bounds result, is pretty fault tolerant )
			const auto oobResult = ttd.GetScalingMultiplier(999);
			if (maxResult < 0.0 || maxResult > 1.0)
				Assert::Fail(L"Scaling mult out of bounds 0 to 1");
			Logger::WriteMessage("End TestGetScalingMultiplier()");
		}
	};

}
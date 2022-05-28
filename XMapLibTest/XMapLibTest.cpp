#include "pch.h"

#include <cassert>

#include "CppUnitTest.h"
#include "TestMapper.h"
#include "TestMouse.h"
#include "TestMapFunctions.h"
#include "TestThumbstickToDelay.h"
#include "../XMapLib/MouseSettings.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace XMapLibTest
{
	TEST_CLASS(TestAssert)
	{
	public:
		//KeyboardSettings doesn't have relevant values to assert here.
		TEST_METHOD(TestAssertSettings)
		{
			using namespace sds;
			using namespace std;
			Logger::WriteMessage("Begin TestAssertSettings()");
			//Assert that certain constants are certain values, to not invalidate the tests.
			Assert::AreEqual(MouseSettings::MICROSECONDS_MAX, 24'000);
			Assert::AreEqual(MouseSettings::MICROSECONDS_MIN, 1'100);
			Assert::AreEqual(MouseSettings::SENSITIVITY_MAX, 100);
			Assert::AreEqual(MouseSettings::SENSITIVITY_MIN, 1);
			Logger::WriteMessage("End TestAssertSettings()");
		}
	};
}

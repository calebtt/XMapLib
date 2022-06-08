#include "pch.h"

#include <cassert>

#include "CppUnitTest.h"
#include "TestMapper.h"
#include "TestMouse.h"
#include "TestMapFunctions.h"
#include "TestThumbstickToDelay.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace XMapLibTest
{
	TEST_CLASS(TestAssert)
	{
	public:
		// This is used to assert some settings remain unmodified for the purpose of the rest of the unit testing.
		// KeyboardSettings doesn't have relevant values to assert here.
		TEST_METHOD(TestAssertSettings)
		{
			using namespace sds;
			Logger::WriteMessage("Begin TestAssertSettings()");
			//Assert that certain constants are certain values, to not invalidate the tests.
			Assert::AreEqual(MouseSettings::MICROSECONDS_MAX, 32'000);
			Assert::AreEqual(MouseSettings::MICROSECONDS_MIN, 1'500);
			Assert::AreEqual(MouseSettings::SENSITIVITY_MAX, 100);
			Assert::AreEqual(MouseSettings::SENSITIVITY_MIN, 1);
			Logger::WriteMessage("End TestAssertSettings()");
		}
	};
}

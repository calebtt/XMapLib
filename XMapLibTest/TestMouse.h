#pragma once
#include "pch.h"
#include "CppUnitTest.h"
#include "../XMapLib/MouseMapper.h"

namespace XMapLibTest
{
	using namespace Microsoft::VisualStudio::CppUnitTestFramework;
	TEST_CLASS(TestMouse)
	{

		
	public:
		// Used for testing mousemapper's ability to set sensitivity values, and report erroneous ones.
		TEST_METHOD(TestSetSensitivity)
		{
			auto LogFn = [](const std::string msg) { Logger::WriteMessage(msg.c_str()); };
			std::shared_ptr<sds::STRunner> srsp = std::make_shared<sds::STRunner>(false, LogFn);
			sds::MouseMapper mouse(srsp, {}, LogFn);

			//test some good cases
			Assert::IsTrue(mouse.SetSensitivity(1).empty());
			Assert::IsTrue(mouse.SetSensitivity(100).empty());
			Assert::IsTrue(mouse.SetSensitivity(50).empty());
			//test some edge cases
			Assert::IsFalse(mouse.SetSensitivity(-1).empty());
			Assert::IsFalse(mouse.SetSensitivity(0).empty());
			Assert::IsFalse(mouse.SetSensitivity(101).empty());
			//test some edge cases for integer value extremes
			constexpr const long long IntMax = std::numeric_limits<int>::max();
			constexpr const long long IntMin = std::numeric_limits<int>::min();
			Assert::IsFalse(mouse.SetSensitivity(IntMax).empty());
			Assert::IsFalse(mouse.SetSensitivity(IntMin).empty());
			//test that valid values are being set
			mouse.SetSensitivity(1);
			Assert::IsTrue(mouse.GetSensitivity() == 1);
			mouse.SetSensitivity(100);
			Assert::IsTrue(mouse.GetSensitivity() == 100);
		}
	};
}

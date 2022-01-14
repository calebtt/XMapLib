#pragma once
#include "pch.h"
#include "CppUnitTest.h"
#include "../XMapLib/MouseMapper.h"

namespace XMapLibTest
{
	using namespace Microsoft::VisualStudio::CppUnitTestFramework;
	TEST_CLASS(TestMouse)
	{

		sds::MouseMapper mouse;
	public:
		TEST_METHOD(TestSetSensitivity)
		{
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

		}
	};
}

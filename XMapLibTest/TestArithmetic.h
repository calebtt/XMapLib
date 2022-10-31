#pragma once
#include "pch.h"
#include "CppUnitTest.h"

namespace XMapLibTest
{
	using namespace Microsoft::VisualStudio::CppUnitTestFramework;
	TEST_CLASS(TestArithmetic)
	{
	public:
		TEST_METHOD(TestConstAbs)
		{
			using namespace std;
			using namespace sds::Utilities;
			Logger::WriteMessage("Begin TestConstAbs()");
			constexpr int cInt = 1;

			// Sanity checking constexpr int '1' is '1'
			constexpr auto res1 = ConstAbs(cInt);
			Assert::AreEqual(res1, 1);

			// runtime int '1' is '1'
			const int t1 = 1;
			const auto res2 = ConstAbs(t1);

			// constexpr int-min is int-max
			constexpr auto res3 = ConstAbs(std::numeric_limits<int>::min());
			Assert::AreEqual(res3, std::numeric_limits<int>::max());

			// constexpr int-min is int-max
			constexpr auto iminminus = std::numeric_limits<int>::min() + 1;
			constexpr auto imaxminus = std::numeric_limits<int>::max();
			constexpr auto res4 = ConstAbs(iminminus);
			Assert::AreEqual(res4, imaxminus);

			Logger::WriteMessage("End TestConstAbs()");
		}
	};

}
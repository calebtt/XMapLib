#include "pch.h"
#include "CppUnitTest.h"
#include "../XMapLib/SensitivityMap.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace XMapLibTest
{
	TEST_CLASS(XMapLibTest)
	{
		const int SENS_MIN = 1;
		const int SENS_MAX = 100;
		const int DELAY_MIN = 500;
		const int DELAY_MAX = 18000;
		const int DELAY_MINMAX = 1500;
	public:
		constexpr bool IsWithin(const auto testVal, const auto result, const auto within)
		{
			return ((result > (testVal - within)) && (result < (testVal + within)));
		}
		TEST_METHOD(TestSensitivityMap)
		{
			using namespace sds;
			using namespace std;
			auto TestAndPrint = [this](const auto sensitivity, const auto key_elem, const auto value_elem, std::wstring message)
			{
				SensitivityMap mp;
				const map<int, int> sensMap = mp.BuildSensitivityMap(sensitivity, SENS_MIN, SENS_MAX, DELAY_MIN, DELAY_MAX, DELAY_MINMAX);
				const int firstResult = sensMap.at(key_elem);
				message += L" Map built with sens:" + std::to_wstring(sensitivity);
				message += L" To test index:" + std::to_wstring(key_elem);
				message += L" Expecting key elem:" + std::to_wstring(value_elem);
				message += L" [Result]:" + std::to_wstring(firstResult);
				Assert::IsTrue(IsWithin(firstResult, value_elem, 200), message.c_str());
			};
			TestAndPrint(100, SENS_MIN, DELAY_MAX, L"[TEST1]");
			TestAndPrint(50, SENS_MAX, 1000, L"[TEST2]");
			TestAndPrint(1, SENS_MAX, 1500, L"[TEST3]");
			TestAndPrint(100, SENS_MAX, DELAY_MIN, L"[TEST4]");
		}
		TEST_METHOD(TestSensitivityMinimum)
		{
			using namespace sds;
			using namespace std;
			auto TestAndPrint = [this](const auto sensitivity, const auto expected_result, std::wstring message)
			{
				SensitivityMap mp;
				const int result = mp.SensitivityToMinimum(sensitivity, SENS_MIN, SENS_MAX, DELAY_MIN, DELAY_MAX);
				message += L" Result built with sens:" + std::to_wstring(sensitivity);
				message += L" Expecting result:" + std::to_wstring(expected_result);
				message += L" [Result]:" + std::to_wstring(result);
				Assert::IsTrue(IsWithin(result, expected_result, 200), message.c_str());
			};
			TestAndPrint(100, DELAY_MIN, L"[TEST1]");
			TestAndPrint(50, ((DELAY_MAX-DELAY_MIN)/2.0)+DELAY_MIN, L"[TEST2]");
			TestAndPrint(1, DELAY_MAX, L"[TEST3]");
		}
	};
}

#pragma once
#include "pch.h"
#include "CppUnitTest.h"
#include "../XMapLib/MapFunctions.h"

namespace XMapLibTest
{
	using namespace Microsoft::VisualStudio::CppUnitTestFramework;
	TEST_CLASS(TestMapFunctions)
	{
	public:
		TEST_METHOD(TestSearchMap)
		{
			using namespace sds;
			using namespace std;
			Logger::WriteMessage("Begin TestSetMapInfo()");
			const wstring one{ L"one" };
			const wstring two{ L"two" };
			const wstring three{ L"three" };
			const map<wstring, int> testMap{ {one,1}, {two,2}, {three,3} };
			auto TestFound = [&](const wstring key, const int value)
			{
				
				int outValue = 0;
				const wstring message = L"asserting key " + key + L" is found in the map.";
				std::wstringstream ws;
				ws << L"asserting value " << value << L" is found in the map.";
				const wstring valueMessage = ws.str();
				Assert::IsTrue(Utilities::MapFunctions::IsInMap(key, testMap, outValue), message.c_str());
				Assert::AreEqual(outValue, value, valueMessage.c_str());
			};
			TestFound(one, 1);
			TestFound(two, 2);
			TestFound(three, 3);
			const wstring notFound = L"not found key";
			int result = 0;
			Assert::IsFalse(Utilities::MapFunctions::IsInMap(notFound, testMap, result), L"Testing key definitely not in map returns false.");
			Assert::AreEqual(result, 0, L"Testing reference arg still 0 after not found in map.");
			Logger::WriteMessage("End TestSetMapInfo()");
		}
	};

}
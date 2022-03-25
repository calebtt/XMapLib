#pragma once
#include "pch.h"
#include "CppUnitTest.h"

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
				const wstring message = L"asserting key " + key + L" is found in the map.";
				std::wstringstream ws;
				ws << L"asserting value " << value << L" is found in the map.";
				const wstring valueMessage = ws.str();
				Assert::IsTrue(testMap.contains(key), message.c_str());
				Assert::AreEqual(testMap.at(key), value, valueMessage.c_str());
			};
			TestFound(one, 1);
			TestFound(two, 2);
			TestFound(three, 3);
			const wstring notFound = L"not found key";
			Assert::IsFalse(testMap.contains(notFound), L"Testing key definitely not in map returns false.");
			Logger::WriteMessage("End TestSetMapInfo()");
		}
	};

}
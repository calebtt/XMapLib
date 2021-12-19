#pragma once
#include "pch.h"
#include "CppUnitTest.h"
#include "../XMapLib/KeyboardMapper.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace XMapLibTest
{
	TEST_CLASS(TestMapper)
	{
	public:
		TEST_METHOD(TestSetMapInfo)
		{
			using namespace sds;
			using namespace std;
			Logger::WriteMessage("Begin TestSetMapInfo()");

			//valid characters that may be included.
			const std::string InputAlphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-=,./`[];\'"
				+ std::string("!@#$%^&*()_+?{}~");
			KeyboardMapper mp;
			auto testMapFunctionTrue = [&mp](const auto s)
			{
				std::string ert = "Testmap [input]: ";
				ert += s;
				ert += " [expect result no error message]: ";
				KeyboardKeyMap key(VK_PAD_DPAD_DOWN, s, true);
				std::string ertt = mp.AddMap(key);
				std::string composite = ert + ertt;
				std::wstring errMsg;
				std::copy(composite.begin(), composite.end(), std::back_inserter(errMsg));
				Assert::IsTrue(ertt.empty(), errMsg.c_str());
			};
			auto testMapFunctionFalse = [&mp](const auto s)
			{
				std::string ert = "Testmap [input]: ";
				ert += s;
				ert += " [expect result no error message]: ";
				KeyboardKeyMap key(VK_PAD_DPAD_DOWN, s, true);
				std::string ertt = mp.AddMap(key);
				std::string composite = ert + ertt;
				std::wstring errMsg;
				std::copy(composite.begin(), composite.end(), std::back_inserter(errMsg));
				Assert::IsFalse(ertt.empty(), errMsg.c_str());
			};
			//Test known good string case
			std::for_each(InputAlphabet.begin(), InputAlphabet.end(), testMapFunctionTrue);

			//test some edge cases and malformed input
			testMapFunctionFalse((static_cast<char>(0)));
			testMapFunctionFalse((static_cast<char>(-1)));
			testMapFunctionFalse((static_cast<char>(256)));
			testMapFunctionFalse((static_cast<char>(255)));
			Logger::WriteMessage("End TestSetMapInfo()");
		}
	};

}
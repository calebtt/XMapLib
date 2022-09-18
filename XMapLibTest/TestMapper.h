#pragma once
#include "pch.h"
#include "CppUnitTest.h"
#include "../XMapLib/TPrior.h"
#include "../XMapLib/KeyboardMapper.h"

namespace XMapLibTest
{
	using namespace Microsoft::VisualStudio::CppUnitTestFramework;
	TEST_CLASS(TestMapper)
	{
	public:
		// Used for testing KeyboardMapper's ability to add maps and report erroneous ones.
		TEST_METHOD(TestSetMapInfo)
		{
			//Testing KeyboardKeyMap
			using namespace sds;
			Logger::WriteMessage("Begin TestSetMapInfo()");

			//valid characters that may be included.
			const std::string InputAlphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-=,./`[];\'"
				+ std::string("!@#$%^&*()_+?{}~");
			KeyboardMapper mp(std::make_shared<impcool::ThreadUnitPlus>());
			auto testMapFunction = [&mp](const auto s, const bool testTrue = true)
			{
				std::string ert = "Testmap [input]: ";
				ert += s;
				ert += " [expect result no error message]: ";
				const KeyboardKeyMap key(VK_PAD_DPAD_DOWN, s, true);
				const std::string ertt = mp.AddMap(key);
				std::string composite = ert + ertt;
				std::wstring errMsg;
				std::copy(composite.begin(), composite.end(), std::back_inserter(errMsg));
				if(testTrue)
					Assert::IsTrue(ertt.empty(), errMsg.c_str());
				else
					Assert::IsFalse(ertt.empty(), errMsg.c_str());
			};
			//Test known good string case
			std::for_each(InputAlphabet.begin(), InputAlphabet.end(), testMapFunction);

			//test some edge cases and malformed input
			testMapFunction((static_cast<char>(0)), false);
			testMapFunction((static_cast<char>(-1)), false);
			testMapFunction((static_cast<char>(256)), false);
			testMapFunction((static_cast<char>(255)), false);
			Logger::WriteMessage("End TestSetMapInfo()");
		}
	};

}
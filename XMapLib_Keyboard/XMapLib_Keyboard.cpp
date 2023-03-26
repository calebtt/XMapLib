// XMapLib_Keyboard.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "LibIncludes.h"
#include <iostream>
#include <string>
#include <format>

#include "KeyboardActionTranslator.h"
#include "KeyboardMapper.h"

inline
auto MainLoop()
{
    using namespace std::chrono_literals;
    using std::cout;
    GetterExit ge{};
    while (ge())
    {
        std::this_thread::sleep_for(2s);
        cout << "Still running...\n";
    }
}

inline
auto GetThreadUnit()
{
    using TUnit_t = imp::ThreadUnitPlusPlus;
    using SharedTUnit_t = std::shared_ptr<TUnit_t>;
    SharedTUnit_t threadUnit = std::make_shared<TUnit_t>();
    return threadUnit;
}

inline
auto GetMappings()
{
    using namespace sds;
    std::vector<sds::CBActionMap> mappings;
    CBActionMap tm{
        .Vk = VK_PAD_A,
        .UsesRepeat = false,
        .ExclusivityGrouping = {},
        .OnDown = []() { std::cout << "Action:[Down]\n"; },
        .OnUp = []() { std::cout << "Action:[Up]\n"; },
        .OnRepeat = []() { std::cout << "Action:[Repeat]\n"; },
        .OnReset = []() { std::cout << "Action:[Reset]\n"; },
        .CustomRepeatDelay = {},
        .LastAction = {}
    };
    mappings.emplace_back(tm);
    return mappings;
}

inline
auto GetTestControllerState()
{
    static std::size_t currentIndex{};
    const std::array stateList =
    {
        sds::ControllerStateWrapper{ VK_PAD_A, true, false, false },
        sds::ControllerStateWrapper{ VK_PAD_A, false, false, true }
    	,sds::ControllerStateWrapper{ VK_PAD_A, false, true, false }
    };
    currentIndex++;
    if (currentIndex > stateList.size())
        return sds::ControllerStateWrapper{};
    return stateList[currentIndex - 1];
}

inline
auto TestCaller(sds::TranslationResult& tr)
{
    CallAndUpdateTranslationResult(tr);
    return std::array{tr};
}

inline
auto RunTestWithDriverStates()
{
    using namespace sds;
    using namespace std::chrono_literals;
    using std::ranges::for_each, std::cout;
    constexpr std::size_t TestCount{ 3 };

	KeyboardActionTranslator tra{ GetMappings() };
    // It seems as though if all you are doing is feeding the output of one object to another object, they don't need
    // to be dependent on one another at all. This design also makes testing the functionality quite simple, just change
    // the source of input to the translator component to a source for test data.
    // For the test, we will just manually modify the state for now (no mapper obj).
    auto jvt = std::views::join(
        std::array{
        	TestCaller(tra(GetTestControllerState()).at(0)),
            TestCaller(tra(GetTestControllerState()).at(0)),
            TestCaller(tra(GetTestControllerState()).at(0)),
            TestCaller(tra(GetTestControllerState()).at(0))
        });
    cout << "Dumping intermediate translation result buffers...\n";
    for_each(jvt, [&](const sds::TranslationResult& e) { cout << e << '\n'; });
    cout << "Dumping cleanup actions buffer: ";
    const auto cleanupActions = tra.GetCleanupActions();
    for_each(cleanupActions, [&](const sds::TranslationResult& e) { cout << e << '\n'; });
}

int main()
{
    RunTestWithDriverStates();
}
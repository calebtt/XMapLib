// XMapLib_Keyboard.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "LibIncludes.h"
#include <iostream>
#include <string>
#include <format>

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
    using TUnit_t = imp::ThreadUnitFP;
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
        .UsesRepeat = true,
        .ExclusivityGrouping = {},
        .OnDown = []() {std::cout << "Down "; },
        .OnUp = []() { std::cout << "Up "; },
        .OnRepeat = []() { std::cout << "Repeat "; },
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
        sds::ControllerStateWrapper{ VK_PAD_A, false, false, true },
        sds::ControllerStateWrapper{ VK_PAD_A, false, true, false }
    };
    currentIndex++;
    if (currentIndex > stateList.size())
        return sds::ControllerStateWrapper{};
    return stateList[currentIndex - 1];
}

int main()
{
    using namespace sds;
    using namespace sds::Utilities;
    using std::ranges::for_each;
    constexpr std::size_t TestCount{ 3 };

    auto threadUnit = std::make_shared<imp::ThreadUnitFP>();
    KeyboardPoller keyPoller{ 0 };
    CBActionTranslator tra{ GetMappings() };
    // The call to the translator closure type, passed the poller's output.
    const auto updates = tra(keyPoller());
    for (std::size_t i{}; i < TestCount; ++i)
    {
        // It seems as though if all you are doing is feeding the output of one object to another object, they don't need
        // to be dependent on one another at all. This design also makes testing the functionality quite simple, just change
        // the source of input to the translator component to a source for test data.
        const auto testUpdate = tra(GetTestControllerState());
        for_each(testUpdate, [&](const sds::TranslationResult& e) { std::cout << e << '\n'; });
    }
}
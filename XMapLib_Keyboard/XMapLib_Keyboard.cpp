// XMapLib_Keyboard.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "LibIncludes.h"
#include <iostream>

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
auto GetMapperType()
{
    std::shared_ptr<sds::KeyboardPoller> keyPoller = std::make_shared<sds::KeyboardPoller>(0);
    sds::CBActionTranslator tra(GetMappings());

    return sds::KeyboardMapper{ keyPoller, std::move(tra) };
}



int main()
{
    using namespace sds;
    using namespace sds::Utilities;

	// Construct the thread upon which the polling, translation, and mapping will occur.
    auto threadUnit = std::make_shared<imp::ThreadUnitFP>();
    auto keyMapper = GetMapperType();


    const auto updates = keyMapper.GetUpdate();
    std::cout << "Update size: " << updates.size() << '\n';

}
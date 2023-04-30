// XMapLib_Keyboard.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "LibIncludes.h"
#include <iostream>
#include <string>
#include <format>

#include "KeyboardActionTranslator.h"
#include "KeyboardMapper.h"
#include "KeyboardPoller.h"

auto GetDriverMappings()
{
    std::vector<sds::CBActionMap> mapBuffer
    {
        sds::CBActionMap{
            .Vk = VK_PAD_A,
            .UsesRepeat = true,
            .OnDown = []() { std::cout << "[PAD_A]=[DOWN]\n"; },
            .OnUp = []() { std::cout << "[PAD_A]=[UP]\n"; },
            .OnRepeat = []() { std::cout << "[PAD_A]=[REPEAT]\n"; }
        },
        sds::CBActionMap{
            .Vk = VK_PAD_B,
            .UsesRepeat = true,
            .OnDown = []() { std::cout << "[PAD_B]=[DOWN]\n"; },
            .OnUp = []() { std::cout << "[PAD_B]=[UP]\n"; },
            .OnRepeat = []() { std::cout << "[PAD_B]=[REPEAT]\n"; }
        },
        sds::CBActionMap{
            .Vk = VK_PAD_LTHUMB_UP,
            .UsesRepeat = true,
            .ExclusivityGrouping = 101,
            .OnDown = []() { std::cout << "[LTHUMB_UP]=[DOWN]\n"; },
            .OnUp = []() { std::cout << "[LTHUMB_UP]=[UP]\n"; },
            .OnRepeat = []() { std::cout << "[LTHUMB_UP]=[REPEAT]\n"; }
        },
        sds::CBActionMap{
            .Vk = VK_PAD_LTHUMB_DOWN,
            .UsesRepeat = true,
            .ExclusivityGrouping = 101,
            .OnDown = []() { std::cout << "[LTHUMB_DOWN]=[DOWN]\n"; },
            .OnUp = []() { std::cout << "[LTHUMB_DOWN]=[UP]\n"; },
            .OnRepeat = []() { std::cout << "[LTHUMB_DOWN]=[REPEAT]\n"; }
        },
        sds::CBActionMap{
            .Vk = VK_PAD_LTHUMB_RIGHT,
            .UsesRepeat = true,
            .ExclusivityGrouping = 101,
            .OnDown = []() { std::cout << "[LTHUMB_RIGHT]=[DOWN]\n"; },
            .OnUp = []() { std::cout << "[LTHUMB_RIGHT]=[UP]\n"; },
            .OnRepeat = []() { std::cout << "[LTHUMB_RIGHT]=[REPEAT]\n"; }
        },
        sds::CBActionMap{
            .Vk = VK_PAD_LTHUMB_LEFT,
            .UsesRepeat = true,
            .ExclusivityGrouping = 101,
            .OnDown = []() { std::cout << "[LTHUMB_LEFT]=[DOWN]\n"; },
            .OnUp = []() { std::cout << "[LTHUMB_LEFT]=[UP]\n"; },
            .OnRepeat = []() { std::cout << "[LTHUMB_LEFT]=[REPEAT]\n"; }
        }
    };
    return mapBuffer;
}

// Crude mechanism to keep the loop running until [enter] is pressed.
struct GetterExitCallable
{
    bool IsDone{ false };
    void GetExitSignal()
    {
        std::string buf;
        std::getline(std::cin, buf);
        IsDone = true;
    }
};
// Test driver program for keyboard mapping
int main()
{
    using namespace std::chrono_literals;

    auto mapBuffer = GetDriverMappings();
    sds::KeyboardActionTranslator translator(std::move(mapBuffer));

    GetterExitCallable gec;
    auto exitFuture = std::async(std::launch::async, [&]() { gec.GetExitSignal(); });
    while(!gec.IsDone)
    {
        sds::KeyboardPoller kp(0);
        const auto translation = translator(kp.GetUpdatedState());
        translation();
        std::this_thread::sleep_for(10ms);
    }
    std::cout << "Performing cleanup actions...\n";
    const auto cleanupTranslation = translator.GetCleanupActions();
    for (auto& elem : cleanupTranslation)
        elem();

    exitFuture.wait();
}
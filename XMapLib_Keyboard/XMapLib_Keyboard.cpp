// XMapLib_Keyboard.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "LibIncludes.h"
#include <iostream>
#include <string>
#include <format>

#include "KeyboardActionTranslator.h"
#include "KeyboardMapper.h"
#include "KeyboardPoller.h"
#include "../XMapLib_Utils/nanotime.h"

inline
auto GetEpochTimestamp()
{
    const auto currentTime = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(currentTime.time_since_epoch());
}

auto GetDriverMappings()
{
    using std::vector, sds::CBActionMap, std::cout;
    vector mapBuffer
    {
        CBActionMap{
            .Vk = VK_PAD_A,
            .UsesRepeatBehavior = true,
            .OnDown = [&]() { std::cout << std::format("[PAD_A]=[DOWN] @{}\n",GetEpochTimestamp()); },
            .OnUp = []() { std::cout << std::format("[PAD_A]=[UP] @{}\n", GetEpochTimestamp()); },
            .OnRepeat = []() { std::cout << std::format("[PAD_A]=[REPEAT] @{}\n", GetEpochTimestamp()); },
            //.OnReset = []() { std::cout << std::format("[PAD_A]=[RESET] @{}\n", GetEpochTimestamp()); }
            //.CustomRepeatDelay = std::chrono::seconds{1},
            .PriorToRepeatDelay = std::chrono::milliseconds{500},
        },
        CBActionMap{
            .Vk = VK_PAD_B,
            .UsesRepeatBehavior = false,
            .SendsFirstRepeatOnly = true,
            .OnDown = []() { std::cout << std::format("[PAD_B]=[DOWN] @{}\n", GetEpochTimestamp()); },
            .OnUp = []() { std::cout << std::format("[PAD_B]=[UP] @{}\n", GetEpochTimestamp()); },
            .OnRepeat = []() { std::cout << std::format("[PAD_B]=[REPEAT] @{}\n", GetEpochTimestamp()); },
            .OnReset = []() { std::cout << std::format("[PAD_B]=[RESET] @{}\n", GetEpochTimestamp()); },
        },
        CBActionMap{
            .Vk = VK_PAD_LTHUMB_UP,
            .UsesRepeatBehavior = true,
            .ExclusivityGrouping = 101,
            .OnDown = []() { std::cout << std::format("[LTHUMB_UP]=[DOWN] @{}\n", GetEpochTimestamp()); },
            .OnUp = []() { std::cout << std::format("[LTHUMB_UP]=[UP] @{}\n", GetEpochTimestamp()); },
            .OnRepeat = []() { std::cout << std::format("[LTHUMB_UP]=[REPEAT] @{}\n", GetEpochTimestamp()); }
        },
        CBActionMap{
            .Vk = VK_PAD_LTHUMB_DOWN,
            .UsesRepeatBehavior = true,
            .ExclusivityGrouping = 101,
            .OnDown = []() { std::cout << std::format("[LTHUMB_DOWN]=[DOWN] @{}\n", GetEpochTimestamp()); },
            .OnUp = []() { std::cout << std::format("[LTHUMB_DOWN]=[UP] @{}\n", GetEpochTimestamp()); },
            .OnRepeat = []() { std::cout << std::format("[LTHUMB_DOWN]=[REPEAT] @{}\n", GetEpochTimestamp()); }
        },
        CBActionMap{
            .Vk = VK_PAD_LTHUMB_RIGHT,
            .UsesRepeatBehavior = true,
            .ExclusivityGrouping = 101,
            .OnDown = []() { std::cout << std::format("[LTHUMB_RIGHT]=[DOWN] @{}\n", GetEpochTimestamp()); },
            .OnUp = []() { std::cout << std::format("[LTHUMB_RIGHT]=[UP] @{}\n", GetEpochTimestamp()); },
            .OnRepeat = []() { std::cout << std::format("[LTHUMB_RIGHT]=[REPEAT] @{}\n", GetEpochTimestamp()); }
        },
        CBActionMap{
            .Vk = VK_PAD_LTHUMB_LEFT,
            .UsesRepeatBehavior = true,
            .ExclusivityGrouping = 101,
            .OnDown = []() { std::cout << std::format("[LTHUMB_LEFT]=[DOWN] @{}\n", GetEpochTimestamp()); },
            .OnUp = []() { std::cout << std::format("[LTHUMB_LEFT]=[UP] @{}\n", GetEpochTimestamp()); },
            .OnRepeat = []() { std::cout << std::format("[LTHUMB_LEFT]=[REPEAT] @{}\n", GetEpochTimestamp()); }
        }
    };
    return mapBuffer;
}

// Crude mechanism to keep the loop running until [enter] is pressed.
struct GetterExitCallable
{
    std::atomic<bool> IsDone{ false };
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
    sds::KeyboardPlayerInfo kpi{};
    sds::KeyboardPoller kp(kpi.player_id);

    GetterExitCallable gec;
    const auto exitFuture = std::async(std::launch::async, [&]() { gec.GetExitSignal(); });
    //constexpr std::uint32_t NumStatesPerIter{ 100 };
    while(!gec.IsDone)
    {
        const auto translation = translator(kp.GetUpdatedState());
        translation();
        nanotime_sleep(1'000'000);
    }
    std::cout << "Performing cleanup actions...\n";
    const auto cleanupTranslation = translator.GetCleanupActions();
    for (auto& elem : cleanupTranslation)
        elem();

    exitFuture.wait();
}
// XMapLib_Keyboard.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "LibIncludes.h"
#include <iostream>
#include <string>
#include <format>

#include "KeyboardActionTranslator.h"
#include "KeyboardMapper.h"
#include "KeyboardPollerController.h"
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
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = 111,
            .OnDown = [&]() { std::cout << std::format("[PAD_A]=[DOWN] @{}\n",GetEpochTimestamp()); },
            .OnUp = []() { std::cout << std::format("[PAD_A]=[UP] @{}\n", GetEpochTimestamp()); },
            .OnRepeat = []() { std::cout << std::format("[PAD_A]=[REPEAT] @{}\n", GetEpochTimestamp()); },
            //.OnReset = []() { std::cout << std::format("[PAD_A]=[RESET] @{}\n", GetEpochTimestamp()); }
            //.CustomRepeatDelay = std::chrono::seconds{1},
            .DelayBeforeFirstRepeat = std::chrono::milliseconds{500},
        },
        CBActionMap{
            .Vk = VK_PAD_B,
            .UsesInfiniteRepeat = false,
            .SendsFirstRepeatOnly = true,
            .ExclusivityGrouping = 111,
            .OnDown = []() { std::cout << std::format("[PAD_B]=[DOWN] @{}\n", GetEpochTimestamp()); },
            .OnUp = []() { std::cout << std::format("[PAD_B]=[UP] @{}\n", GetEpochTimestamp()); },
            .OnRepeat = []() { std::cout << std::format("[PAD_B]=[REPEAT] @{}\n", GetEpochTimestamp()); },
            .OnReset = []() { std::cout << std::format("[PAD_B]=[RESET] @{}\n", GetEpochTimestamp()); },
            .DelayBeforeFirstRepeat = std::chrono::seconds{2}
        },
        CBActionMap{
            .Vk = VK_PAD_LTHUMB_UP,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = 101,
            .OnDown = []() { std::cout << std::format("[LTHUMB_UP]=[DOWN] @{}\n", GetEpochTimestamp()); },
            .OnUp = []() { std::cout << std::format("[LTHUMB_UP]=[UP] @{}\n", GetEpochTimestamp()); },
            .OnRepeat = []() { std::cout << std::format("[LTHUMB_UP]=[REPEAT] @{}\n", GetEpochTimestamp()); }
        },
        CBActionMap{
            .Vk = VK_PAD_LTHUMB_DOWN,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = 101,
            .OnDown = []() { std::cout << std::format("[LTHUMB_DOWN]=[DOWN] @{}\n", GetEpochTimestamp()); },
            .OnUp = []() { std::cout << std::format("[LTHUMB_DOWN]=[UP] @{}\n", GetEpochTimestamp()); },
            .OnRepeat = []() { std::cout << std::format("[LTHUMB_DOWN]=[REPEAT] @{}\n", GetEpochTimestamp()); }
        },
        CBActionMap{
            .Vk = VK_PAD_LTHUMB_RIGHT,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = 101,
            .OnDown = []() { std::cout << std::format("[LTHUMB_RIGHT]=[DOWN] @{}\n", GetEpochTimestamp()); },
            .OnUp = []() { std::cout << std::format("[LTHUMB_RIGHT]=[UP] @{}\n", GetEpochTimestamp()); },
            .OnRepeat = []() { std::cout << std::format("[LTHUMB_RIGHT]=[REPEAT] @{}\n", GetEpochTimestamp()); }
        },
        CBActionMap{
            .Vk = VK_PAD_LTHUMB_LEFT,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = 101,
            .OnDown = []() { std::cout << std::format("[LTHUMB_LEFT]=[DOWN] @{}\n", GetEpochTimestamp()); },
            .OnUp = []() { std::cout << std::format("[LTHUMB_LEFT]=[UP] @{}\n", GetEpochTimestamp()); },
            .OnRepeat = []() { std::cout << std::format("[LTHUMB_LEFT]=[REPEAT] @{}\n", GetEpochTimestamp()); }
        },
        CBActionMap{
            .Vk = VK_PAD_RSHOULDER,
            .UsesInfiniteRepeat = false,
            .OnDown = []() { system("cls"); std::cout << "Cleared.\n"; }
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
    sds::KeyboardPlayerInfo playerInfo{};
    sds::KeyboardPollerController controllerPoller(playerInfo.player_id);

    GetterExitCallable gec;
    const auto exitFuture = std::async(std::launch::async, [&]() { gec.GetExitSignal(); });
    while(!gec.IsDone)
    {
        const auto translation = translator(controllerPoller());
        translation();
        nanotime_sleep(1'000'000);
    }
    std::cout << "Performing cleanup actions...\n";
    const auto cleanupTranslation = translator.GetCleanupActions();
    for (auto& cleanupAction : cleanupTranslation)
        cleanupAction();

    exitFuture.wait();
}
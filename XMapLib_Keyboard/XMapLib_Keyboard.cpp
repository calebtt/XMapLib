// XMapLib_Keyboard.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "LibIncludes.h"
#include <iostream>
#include <string>
#include <format>

#include "KeyboardActionTranslator.h"
#include "ControllerButtonToActionMap.h"
#include "KeyboardPollerController.h"
#include "../XMapLib_Utils/nanotime.h"
#include "../XMapLib_Utils/SendMouseInput.h"

inline
auto GetEpochTimestamp()
{
    const auto currentTime = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(currentTime.time_since_epoch());
}

auto GetDriverButtonMappings()
{
    using std::vector, sds::CBActionMap, std::cout;
    using namespace std::chrono_literals;
    sds::Utilities::SendMouseInput smi;
    constexpr auto FirstDelay = 1ns; // mouse move delays
    constexpr auto RepeatDelay = 1ns;
    constexpr int MouseExGroup = 102;
    vector mapBuffer
    {
        CBActionMap{
            .Vk = VK_PAD_A,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = 111,
            .OnDown = []() { std::cout << std::format("[PAD_A]=[DOWN] @{}\n",GetEpochTimestamp()); },
            .OnUp = []() { std::cout << std::format("[PAD_A]=[UP] @{}\n", GetEpochTimestamp()); },
            .OnRepeat = []() { std::cout << std::format("[PAD_A]=[REPEAT] @{}\n", GetEpochTimestamp()); },
            //.OnReset = []() { std::cout << std::format("[PAD_A]=[RESET] @{}\n", GetEpochTimestamp()); }
            //.CustomRepeatDelay = std::chrono::seconds{1},
            .DelayBeforeFirstRepeat = 500ms
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
            .DelayBeforeFirstRepeat = 2s
        },
    	CBActionMap{
            .Vk = VK_PAD_X,
            .UsesInfiniteRepeat = false,
            .SendsFirstRepeatOnly = true,
            .ExclusivityGrouping = 111,
            .OnDown = []() { std::cout << std::format("[PAD_X]=[DOWN] @{}\n", GetEpochTimestamp()); },
            .OnUp = []() { std::cout << std::format("[PAD_X]=[UP] @{}\n", GetEpochTimestamp()); },
            .OnRepeat = []() { std::cout << std::format("[PAD_X]=[REPEAT] @{}\n", GetEpochTimestamp()); },
            .OnReset = []() { std::cout << std::format("[PAD_X]=[RESET] @{}\n", GetEpochTimestamp()); },
            .DelayBeforeFirstRepeat = 2s
        },
        CBActionMap{
            .Vk = VK_PAD_Y,
            .UsesInfiniteRepeat = false,
            .SendsFirstRepeatOnly = true,
            .ExclusivityGrouping = 111,
            .OnDown = []() { std::cout << std::format("[PAD_Y]=[DOWN] @{}\n", GetEpochTimestamp()); },
            .OnUp = []() { std::cout << std::format("[PAD_Y]=[UP] @{}\n", GetEpochTimestamp()); },
            .OnRepeat = []() { std::cout << std::format("[PAD_Y]=[REPEAT] @{}\n", GetEpochTimestamp()); },
            .OnReset = []() { std::cout << std::format("[PAD_Y]=[RESET] @{}\n", GetEpochTimestamp()); },
            .DelayBeforeFirstRepeat = 2s
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
        },
        CBActionMap{
            .Vk = VK_PAD_LTRIGGER,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = 101,
            .OnDown = []() { std::cout << std::format("[LTRIGGER]=[DOWN] @{}\n", GetEpochTimestamp()); },
            .OnUp = []() { std::cout << std::format("[LTRIGGER]=[UP] @{}\n", GetEpochTimestamp()); },
        },
    	CBActionMap{
            .Vk = VK_PAD_RTRIGGER,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = 101,
            .OnDown = []() { std::cout << std::format("[RTRIGGER]=[DOWN] @{}\n", GetEpochTimestamp()); },
            .OnUp = []() { std::cout << std::format("[RTRIGGER]=[UP] @{}\n", GetEpochTimestamp()); },
        }
    };
    return mapBuffer;
}

auto GetDriverMouseMappings()
{
    using std::vector, sds::CBActionMap, std::cout;
    using namespace std::chrono_literals;
    sds::Utilities::SendMouseInput smi;
    constexpr auto FirstDelay = 1ns; // mouse move delays
    constexpr auto RepeatDelay = 1ns;
    constexpr int MouseExGroup = 102;
    vector mapBuffer
    {
        // Mouse move stuff
        CBActionMap{
            .Vk = VK_PAD_RTHUMB_UP,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = MouseExGroup,
            .OnDown = [smi]() mutable
            {
                smi.SendMouseMove(0, 1);
            },
            .OnRepeat = [smi]() mutable
            {
                smi.SendMouseMove(0, 1);
            },
            .DelayBeforeFirstRepeat = FirstDelay,
            .DelayForRepeats = RepeatDelay
        },
        CBActionMap{
            .Vk = VK_PAD_RTHUMB_DOWN,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = MouseExGroup,
            .OnDown = [smi]() mutable
            {
                smi.SendMouseMove(0, -1);
            },
            .OnRepeat = [smi]() mutable
            {
                smi.SendMouseMove(0, -1);
            },
            .DelayBeforeFirstRepeat = FirstDelay,
            .DelayForRepeats = RepeatDelay
        },
        CBActionMap{
            .Vk = VK_PAD_RTHUMB_LEFT,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = MouseExGroup,
            .OnDown = [smi]() mutable
            {
                smi.SendMouseMove(-1, 0);
            },
            .OnRepeat = [smi]() mutable
            {
                smi.SendMouseMove(-1, 0);
            },
            .DelayBeforeFirstRepeat = FirstDelay,
            .DelayForRepeats = RepeatDelay
        },
        CBActionMap{
            .Vk = VK_PAD_RTHUMB_RIGHT,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = MouseExGroup,
            .OnDown = [smi]() mutable
            {
                smi.SendMouseMove(1, 0);
            },
            .OnRepeat = [smi]() mutable
            {
                smi.SendMouseMove(1, 0);
            },
            .DelayBeforeFirstRepeat = FirstDelay,
            .DelayForRepeats = RepeatDelay
        },
        CBActionMap{
            .Vk = VK_PAD_RTHUMB_UPRIGHT,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = MouseExGroup,
            .OnDown = [smi]() mutable
            {
                smi.SendMouseMove(1, 1);
            },
            .OnRepeat = [smi]() mutable
            {
                smi.SendMouseMove(1, 1);
            },
            .DelayBeforeFirstRepeat = FirstDelay,
            .DelayForRepeats = RepeatDelay
        },
        CBActionMap{
            .Vk = VK_PAD_RTHUMB_UPLEFT,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = MouseExGroup,
            .OnDown = [smi]() mutable
            {
                smi.SendMouseMove(-1, 1);
            },
            .OnRepeat = [smi]() mutable
            {
                smi.SendMouseMove(-1, 1);
            },
            .DelayBeforeFirstRepeat = FirstDelay,
            .DelayForRepeats = RepeatDelay
        },
        CBActionMap{
            .Vk = VK_PAD_RTHUMB_DOWNLEFT,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = MouseExGroup,
            .OnDown = [smi]() mutable
            {
                smi.SendMouseMove(-1, -1);
            },
            .OnRepeat = [smi]() mutable
            {
                smi.SendMouseMove(-1, -1);
            },
            .DelayBeforeFirstRepeat = FirstDelay,
            .DelayForRepeats = RepeatDelay
        },
        CBActionMap{
            .Vk = VK_PAD_RTHUMB_DOWNRIGHT,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = MouseExGroup,
            .OnDown = [smi]() mutable
            {
                smi.SendMouseMove(1, -1);
            },
            .OnRepeat = [smi]() mutable
            {
                smi.SendMouseMove(1, -1);
            },
            .DelayBeforeFirstRepeat = FirstDelay,
            .DelayForRepeats = RepeatDelay
        },
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

    auto mapBuffer = GetDriverButtonMappings();
    auto mouseMapBuffer = GetDriverMouseMappings();
    sds::KeyboardActionTranslator translator(std::move(mapBuffer));
    sds::KeyboardActionTranslator mouseTranslator(std::move(mouseMapBuffer));
    sds::KeyboardPlayerInfo playerInfo{};
    sds::KeyboardPollerController controllerPoller(playerInfo.player_id);

    GetterExitCallable gec;
    const auto exitFuture = std::async(std::launch::async, [&]() { gec.GetExitSignal(); });
    while(!gec.IsDone)
    {
        const auto pollResult = controllerPoller();
        const auto translation = translator(pollResult);
        const auto mouseTranslation = mouseTranslator(pollResult);
        translation();
        mouseTranslation();
        nanotime_sleep(1'000'000);
    }
    std::cout << "Performing cleanup actions...\n";
    const auto cleanupTranslation = translator.GetCleanupActions();
    for (auto& cleanupAction : cleanupTranslation)
        cleanupAction();
    const auto mouseCleanupTranslation = mouseTranslator.GetCleanupActions();
    for (auto& cleanupAction : mouseCleanupTranslation)
        cleanupAction();

    exitFuture.wait();
}
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
    constexpr int PadButtonsGroup = 111; // Buttons exclusivity grouping.
    constexpr int LeftThumbGroup = 101; // Left thumbstick exclusivity grouping.
    const auto PrintMessageAndTime = [](std::string_view msg)
    {
        cout << msg << " @" << GetEpochTimestamp() << '\n';
    };
    const auto GetDownLambdaForKeyNamed = [=](std::string keyName)
	{
        return [=]() { PrintMessageAndTime(keyName + "=[DOWN]"); };
    };
    const auto GetUpLambdaForKeyNamed = [=](std::string keyName)
    {
        return [=]() { PrintMessageAndTime(keyName + "=[UP]"); };
    };
    const auto GetRepeatLambdaForKeyNamed = [=](std::string keyName)
    {
        return [=]() { PrintMessageAndTime(keyName + "=[REPEAT]"); };
    };
    const auto GetResetLambdaForKeyNamed = [=](std::string keyName)
    {
        return [=]() { PrintMessageAndTime(keyName + "=[RESET]"); };
    };
    vector mapBuffer
    {
        CBActionMap{
            .Vk = VK_PAD_A,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = PadButtonsGroup,
            .OnDown = GetDownLambdaForKeyNamed("[PAD_A]"),
            .OnUp = GetUpLambdaForKeyNamed("[PAD_A]"),
            .OnRepeat = GetRepeatLambdaForKeyNamed("[PAD_A]"),
            .DelayBeforeFirstRepeat = 500ms
        },
        CBActionMap{
            .Vk = VK_PAD_B,
            .UsesInfiniteRepeat = false,
            .SendsFirstRepeatOnly = true,
            .ExclusivityGrouping = PadButtonsGroup,
            .OnDown = GetDownLambdaForKeyNamed("[PAD_B]"),
            .OnUp = GetUpLambdaForKeyNamed("[PAD_B]"),
            .OnRepeat = GetRepeatLambdaForKeyNamed("[PAD_B]"),
            .OnReset = GetResetLambdaForKeyNamed("[PAD_B]"),
            .DelayBeforeFirstRepeat = 2s
        },
    	CBActionMap{
            .Vk = VK_PAD_X,
            .UsesInfiniteRepeat = false,
            .SendsFirstRepeatOnly = true,
            .ExclusivityGrouping = PadButtonsGroup,
            .OnDown = GetDownLambdaForKeyNamed("[PAD_X]"),
            .OnUp = GetUpLambdaForKeyNamed("[PAD_X]"),
            .OnRepeat = GetRepeatLambdaForKeyNamed("[PAD_X]"),
            .OnReset = GetResetLambdaForKeyNamed("[PAD_X]"),
            .DelayBeforeFirstRepeat = 2s
        },
        CBActionMap{
            .Vk = VK_PAD_Y,
            .UsesInfiniteRepeat = false,
            .SendsFirstRepeatOnly = true,
            .ExclusivityGrouping = PadButtonsGroup,
            .OnDown = GetDownLambdaForKeyNamed("[PAD_Y]"),
            .OnUp = GetUpLambdaForKeyNamed("[PAD_Y]"),
            .OnRepeat = GetRepeatLambdaForKeyNamed("[PAD_Y]"),
            .OnReset = GetResetLambdaForKeyNamed("[PAD_Y]"),
            .DelayBeforeFirstRepeat = 2s
        },
        // Left thumbstick directional stuff
        CBActionMap{
            .Vk = VK_PAD_LTHUMB_UP,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = LeftThumbGroup,
            .OnDown = GetDownLambdaForKeyNamed("[LTHUMB_UP]"),
            .OnUp = GetUpLambdaForKeyNamed("[LTHUMB_UP]"),
            .OnRepeat = GetRepeatLambdaForKeyNamed("[LTHUMB_UP]"),
            .OnReset = GetResetLambdaForKeyNamed("[LTHUMB_UP]"),
        },
        CBActionMap{
            .Vk = VK_PAD_LTHUMB_DOWN,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = LeftThumbGroup,
            .OnDown = GetDownLambdaForKeyNamed("[LTHUMB_DOWN]"),
            .OnUp = GetUpLambdaForKeyNamed("[LTHUMB_DOWN]"),
            .OnRepeat = GetRepeatLambdaForKeyNamed("[LTHUMB_DOWN]"),
            .OnReset = GetResetLambdaForKeyNamed("[LTHUMB_DOWN]"),
        },
        CBActionMap{
            .Vk = VK_PAD_LTHUMB_RIGHT,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = LeftThumbGroup,
            .OnDown = GetDownLambdaForKeyNamed("[LTHUMB_RIGHT]"),
            .OnUp = GetUpLambdaForKeyNamed("[LTHUMB_RIGHT]"),
            .OnRepeat = GetRepeatLambdaForKeyNamed("[LTHUMB_RIGHT]"),
            .OnReset = GetResetLambdaForKeyNamed("[LTHUMB_RIGHT]"),
        },
        CBActionMap{
            .Vk = VK_PAD_LTHUMB_LEFT,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = LeftThumbGroup,
            .OnDown = GetDownLambdaForKeyNamed("[LTHUMB_LEFT]"),
            .OnUp = GetUpLambdaForKeyNamed("[LTHUMB_LEFT]"),
            .OnRepeat = GetRepeatLambdaForKeyNamed("[LTHUMB_LEFT]"),
            .OnReset = GetResetLambdaForKeyNamed("[LTHUMB_LEFT]"),
        },
        CBActionMap{
            .Vk = VK_PAD_LTRIGGER,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = LeftThumbGroup,
            .OnDown = GetDownLambdaForKeyNamed("[LTRIGGER]"),
            .OnUp = GetUpLambdaForKeyNamed("[LTRIGGER]"),
        },
    	CBActionMap{
            .Vk = VK_PAD_RTRIGGER,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = LeftThumbGroup,
            .OnDown = GetDownLambdaForKeyNamed("[RTRIGGER]"),
            .OnUp = GetUpLambdaForKeyNamed("[RTRIGGER]"),
        },
        CBActionMap{
            .Vk = VK_PAD_RSHOULDER,
            .UsesInfiniteRepeat = false,
            .OnDown = []() { system("cls"); std::cout << "Cleared.\n"; }
        },
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
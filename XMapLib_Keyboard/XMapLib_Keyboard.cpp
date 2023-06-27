// XMapLib_Keyboard.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "LibIncludes.h"

#include "KeyboardActionTranslator.h"
#include "ControllerButtonToActionMap.h"
#include "KeyboardPollerController.h"
#include "../XMapLib_Utils/nanotime.h"
#include "../XMapLib_Utils/SendMouseInput.h"

#include <iostream>
#include <fstream>


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

auto GetEpochTimestamp()
{
    const auto currentTime = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(currentTime.time_since_epoch());
}

auto GetDriverButtonMappings()
{
    using std::vector, sds::CBActionMap, std::cout;
    using namespace std::chrono_literals;
    using namespace sds;

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
    // These are a good idea in case someone wants this to work for XINPUT_KEYSTROKE instead, just change these to the VK_ ones.
    constexpr detail::VirtualKey_t ButtonA{XINPUT_GAMEPAD_A};
    constexpr detail::VirtualKey_t ButtonB{XINPUT_GAMEPAD_B};
    constexpr detail::VirtualKey_t ButtonX{XINPUT_GAMEPAD_X};
    constexpr detail::VirtualKey_t ButtonY{XINPUT_GAMEPAD_Y};

    constexpr detail::VirtualKey_t ButtonStart{XINPUT_GAMEPAD_START};
    constexpr detail::VirtualKey_t ButtonBack{XINPUT_GAMEPAD_BACK};
    constexpr detail::VirtualKey_t ButtonShoulderLeft{XINPUT_GAMEPAD_LEFT_SHOULDER};
    constexpr detail::VirtualKey_t ButtonShoulderRight{XINPUT_GAMEPAD_RIGHT_SHOULDER};

	constexpr detail::VirtualKey_t DpadUp{XINPUT_GAMEPAD_DPAD_UP};
    constexpr detail::VirtualKey_t DpadDown{XINPUT_GAMEPAD_DPAD_DOWN};
    constexpr detail::VirtualKey_t DpadLeft{XINPUT_GAMEPAD_DPAD_LEFT};
    constexpr detail::VirtualKey_t DpadRight{XINPUT_GAMEPAD_DPAD_RIGHT};

    constexpr detail::VirtualKey_t ThumbLeftClick{XINPUT_GAMEPAD_LEFT_THUMB};
    constexpr detail::VirtualKey_t ThumbRightClick{XINPUT_GAMEPAD_RIGHT_THUMB};

    static constexpr detail::VirtualKey_t LeftThumbstickLeft{VK_GAMEPAD_LEFT_THUMBSTICK_LEFT};
    static constexpr detail::VirtualKey_t LeftThumbstickRight{VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT};
    static constexpr detail::VirtualKey_t LeftThumbstickUp{VK_GAMEPAD_LEFT_THUMBSTICK_UP};
    static constexpr detail::VirtualKey_t LeftThumbstickDown{VK_GAMEPAD_LEFT_THUMBSTICK_DOWN};

    static constexpr detail::VirtualKey_t RightThumbstickLeft{VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT};
    static constexpr detail::VirtualKey_t RightThumbstickRight{VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT};
    static constexpr detail::VirtualKey_t RightThumbstickUp{VK_GAMEPAD_RIGHT_THUMBSTICK_UP};
    static constexpr detail::VirtualKey_t RightThumbstickDown{VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN};

    static constexpr detail::VirtualKey_t TriggerLeft{VK_GAMEPAD_LEFT_TRIGGER};
    static constexpr detail::VirtualKey_t TriggerRight{VK_GAMEPAD_RIGHT_TRIGGER};

    vector mapBuffer
    {
        CBActionMap{
            .ButtonVirtualKeycode = ButtonA,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = PadButtonsGroup,
            .OnDown = GetDownLambdaForKeyNamed("[PAD_A]"),
            .OnUp = GetUpLambdaForKeyNamed("[PAD_A]"),
            .OnRepeat = GetRepeatLambdaForKeyNamed("[PAD_A]"),
            .DelayBeforeFirstRepeat = 500ms
        },
        CBActionMap{
            .ButtonVirtualKeycode = ButtonB,
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
            .ButtonVirtualKeycode = ButtonX,
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
            .ButtonVirtualKeycode = ButtonY,
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
            .ButtonVirtualKeycode = LeftThumbstickUp,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = LeftThumbGroup,
            .OnDown = GetDownLambdaForKeyNamed("[LTHUMB_UP]"),
            .OnUp = GetUpLambdaForKeyNamed("[LTHUMB_UP]"),
            .OnRepeat = GetRepeatLambdaForKeyNamed("[LTHUMB_UP]"),
            .OnReset = GetResetLambdaForKeyNamed("[LTHUMB_UP]"),
        },
        CBActionMap{
            .ButtonVirtualKeycode = LeftThumbstickDown,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = LeftThumbGroup,
            .OnDown = GetDownLambdaForKeyNamed("[LTHUMB_DOWN]"),
            .OnUp = GetUpLambdaForKeyNamed("[LTHUMB_DOWN]"),
            .OnRepeat = GetRepeatLambdaForKeyNamed("[LTHUMB_DOWN]"),
            .OnReset = GetResetLambdaForKeyNamed("[LTHUMB_DOWN]"),
        },
        CBActionMap{
            .ButtonVirtualKeycode = LeftThumbstickRight,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = LeftThumbGroup,
            .OnDown = GetDownLambdaForKeyNamed("[LTHUMB_RIGHT]"),
            .OnUp = GetUpLambdaForKeyNamed("[LTHUMB_RIGHT]"),
            .OnRepeat = GetRepeatLambdaForKeyNamed("[LTHUMB_RIGHT]"),
            .OnReset = GetResetLambdaForKeyNamed("[LTHUMB_RIGHT]"),
        },
        CBActionMap{
            .ButtonVirtualKeycode = LeftThumbstickLeft,
            .UsesInfiniteRepeat = true,
            .ExclusivityGrouping = LeftThumbGroup,
            .OnDown = GetDownLambdaForKeyNamed("[LTHUMB_LEFT]"),
            .OnUp = GetUpLambdaForKeyNamed("[LTHUMB_LEFT]"),
            .OnRepeat = GetRepeatLambdaForKeyNamed("[LTHUMB_LEFT]"),
            .OnReset = GetResetLambdaForKeyNamed("[LTHUMB_LEFT]"),
        },
        CBActionMap{
            .ButtonVirtualKeycode = TriggerLeft,
            .UsesInfiniteRepeat = false,
            .ExclusivityGrouping = LeftThumbGroup,
            .OnDown = GetDownLambdaForKeyNamed("[LTRIGGER]"),
            .OnUp = GetUpLambdaForKeyNamed("[LTRIGGER]"),
            .DelayBeforeFirstRepeat = 1ns,
            .DelayForRepeats = 1ns
        },
    	CBActionMap{
            .ButtonVirtualKeycode = TriggerRight,
            .UsesInfiniteRepeat = false,
            .ExclusivityGrouping = LeftThumbGroup,
            .OnDown = GetDownLambdaForKeyNamed("[RTRIGGER]"),
            .OnUp = GetUpLambdaForKeyNamed("[RTRIGGER]"),
            .DelayBeforeFirstRepeat = 1ns,
            .DelayForRepeats = 1ns
        },
        CBActionMap{
            .ButtonVirtualKeycode = ButtonShoulderRight,
            .UsesInfiniteRepeat = false,
            .OnDown = []() { system("cls"); std::cout << "Cleared.\n"; }
        },
    	CBActionMap{
            .ButtonVirtualKeycode = ButtonShoulderLeft,
            .UsesInfiniteRepeat = false,
            .OnDown = []()
            {

            }
        },
    };
    return mapBuffer;
}

auto GetDriverMouseMappings()
{
    using std::vector, sds::CBActionMap, std::cout;
    using namespace std::chrono_literals;
    sds::Utilities::SendMouseInput smi;
    constexpr auto FirstDelay = 0ns; // mouse move delays
    constexpr auto RepeatDelay = 0ns;
    constexpr int MouseExGroup = 102;
    vector mapBuffer
    {
        // Mouse move stuff
        CBActionMap{
            .ButtonVirtualKeycode = VK_PAD_RTHUMB_UP,
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
            .ButtonVirtualKeycode = VK_PAD_RTHUMB_DOWN,
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
            .ButtonVirtualKeycode = VK_PAD_RTHUMB_LEFT,
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
            .ButtonVirtualKeycode = VK_PAD_RTHUMB_RIGHT,
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
            .ButtonVirtualKeycode = VK_PAD_RTHUMB_UPRIGHT,
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
            .ButtonVirtualKeycode = VK_PAD_RTHUMB_UPLEFT,
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
            .ButtonVirtualKeycode = VK_PAD_RTHUMB_DOWNLEFT,
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
            .ButtonVirtualKeycode = VK_PAD_RTHUMB_DOWNRIGHT,
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

auto RunTestDriverLoop()
{
    using namespace std::chrono_literals;

    auto mapBuffer = GetDriverButtonMappings();
    // Unit test covers testing both translator constructors.
    sds::KeyboardPlayerInfo playerInfo{};
    sds::KeyboardPollerControllerLegacy poller{std::move(mapBuffer)};

    GetterExitCallable gec;
    const auto exitFuture = std::async(std::launch::async, [&]() { gec.GetExitSignal(); });
    while (!gec.IsDone)
    {
        const auto stateUpdate = sds::GetWrappedLegacyApiStateUpdate(playerInfo.player_id);
        const auto translation = poller(stateUpdate);
        translation();
        nanotime_sleep(sds::KeyboardSettings::PollingLoopDelay.count());
    }
    std::cout << "Performing cleanup actions...\n";
    //const auto cleanupTranslation = translator.GetCleanupActions();
    //for (auto& cleanupAction : cleanupTranslation)
    //    cleanupAction();

    exitFuture.wait();
}

//void WritePollResult(std::fstream& outFile, const sds::ControllerStateUpdateWrapper& polledState)
//{
//    outFile << polledState.VirtualKey << '\n';
//    outFile << polledState.KeyDown << '\n';
//    outFile << polledState.KeyUp << '\n';
//    outFile << polledState.KeyRepeat << '\n';
//}
//
//// The purpose of this is to record every polled event occurring during a manual test driver run.
//// In order to be re-created in a unit test and benchmarked.
//void RunRecordingLoop()
//{
//    //using namespace std::chrono_literals;
//    std::fstream outFile("recording.txt", std::ios::out | std::ios::binary);
//    constexpr sds::KeyboardPlayerInfo playerInfo{};
//    sds::KeyboardPollerController controllerPoller(playerInfo.player_id);
//
//    GetterExitCallable gec;
//    const auto exitFuture = std::async(std::launch::async, [&]() { gec.GetExitSignal(); });
//    while (!gec.IsDone)
//    {
//        const auto pollResult = controllerPoller();
//        WritePollResult(outFile, pollResult);
//        nanotime_sleep(sds::KeyboardSettings::PollingLoopDelay.count());
//    }
//
//    exitFuture.wait();
//}

//void RunTriggerTestLoop()
//{
//    using namespace std::chrono_literals;
//
//    auto mapBuffer = GetDriverButtonMappings();
//    // Unit test covers testing both translator constructors.
//    sds::KeyboardActionTranslator translator(std::move(mapBuffer));
//    sds::KeyboardPlayerInfo playerInfo{};
//    sds::KeyboardPollerController controllerPoller(playerInfo.player_id);
//
//    GetterExitCallable gec;
//    const auto exitFuture = std::async(std::launch::async, [&]() { gec.GetExitSignal(); });
//    while (!gec.IsDone)
//    {
//        const auto pollResult = controllerPoller();
//        std::cout << pollResult << '\n';
//        //const auto translation = translator(pollResult);
//        //translation();
//        constexpr auto timeCount = std::chrono::milliseconds(1000);
//        constexpr auto nanosCount = std::chrono::nanoseconds(timeCount);
//        nanotime_sleep(nanosCount.count());
//    }
//    std::cout << "Performing cleanup actions...\n";
//    const auto cleanupTranslation = translator.GetCleanupActions();
//    for (auto& cleanupAction : cleanupTranslation)
//        cleanupAction();
//
//    exitFuture.wait();
//}

// Test driver program for keyboard mapping
int main()
{
    //RunRecordingLoop();
    RunTestDriverLoop();
    //RunTriggerTestLoop();
}
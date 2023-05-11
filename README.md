# XMapLib_Keyboard
A closer to the metal library for Xbox controller to keyboard and mouse input.
Allows for defining custom actions to be performed at each state update of a controller button
mapping. Functions performed on states key-down, key-up, etc. can be easily assigned via lambda functions to perform 
some input simulation or game task., or be easily assigned to any free functions or callable class (overloading operator()).

# Exclusivity Groupings
Exclusivity groupings are a way to ensure only one key assigned to the grouping is in the key-down state at the same time.
This is performed by "overtaking" meaning the most recently pressed key from the group will force a key-up translation result for the overtaken
key before sending the key-down. This behavior will eventually be a customization point, most likely implemented by providing a class matching
a concept interface to a template param of the translator class. For now it has only the provided behavior described here.

# Language Standard
The language standard used for this project is currently C++latest and is likely to remain as such as I use more of the latest C++ language and library features.

# Design
  <b>The main classes for use presently in the native C++ XMapLib_Keyboard project are :</b>
  
 ![keyboard_lib_diagram4](https://github.com/calebtt/XMapLib/assets/12226096/1c826fa0-8647-4bf4-8935-ef42c3eadfa5)

# Example
The process may look like:
```
//
// ...
//
#include "KeyboardActionTranslator.h"
#include "KeyboardPollerController.h"
#include "ControllerButtonToActionMap.h"
#include "KeyboardSettingsPack.h"

inline
auto GetEpochTimestamp()
{
    const auto currentTime = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(currentTime.time_since_epoch());
}

// Build and return the mappings.
inline
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
    }
    std::cout << "Performing cleanup actions...\n";
    const auto cleanupTranslation = translator.GetCleanupActions();
    for (auto& cleanupAction : cleanupTranslation)
        cleanupAction();

    exitFuture.wait();
}
```

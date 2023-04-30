// XMapLib_Keyboard.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "LibIncludes.h"
#include <iostream>
#include <string>
#include <format>

#include "KeyboardActionTranslator.h"
#include "KeyboardMapper.h"
#include "TestPollProvider.h"
#include "TestMappingProvider.h"

//constexpr unsigned short VirtKey{ VK_PAD_A };
//
//inline
//auto MainLoop()
//{
//    using namespace std::chrono_literals;
//    using std::cout;
//    GetterExit ge{};
//    while (ge())
//    {
//        std::this_thread::sleep_for(2s);
//        cout << "Still running...\n";
//    }
//}
//
//inline
//auto GetThreadUnit()
//{
//    using TUnit_t = imp::ThreadUnitPlusPlus;
//    using SharedTUnit_t = std::shared_ptr<TUnit_t>;
//    SharedTUnit_t threadUnit = std::make_shared<TUnit_t>();
//    return threadUnit;
//}
//
//inline
//auto TestCaller(sds::TranslationResult& tr)
//{
//    CallAndUpdateTranslationResult(tr);
//    return std::array{tr};
//}
//
//inline
//auto RunProgressionTest()
//{
//    using namespace sds;
//    using namespace std::chrono_literals;
//    using std::ranges::for_each, std::cout;
//    TestMappingProvider testMaps{ VirtKey };
//    TestPollProvider testPoll{ VirtKey };
//	KeyboardActionTranslator translator{ testMaps.GetMappings() };
//    // It seems as though if all you are doing is feeding the output of one object to another object, they don't need
//    // to be dependent on one another at all. This design also makes testing the functionality quite simple, just change
//    // the source of input to the translator component to a source for test data.
//    auto jvt = std::views::join(
//        std::array{
//        	TestCaller(translator(testPoll.GetDownState()).at(0)),
//            TestCaller(translator(testPoll.GetRepeatState()).at(0)),
//            TestCaller(translator(testPoll.GetUpState()).at(0)),
//            TestCaller(translator(testPoll.GetNoState()).at(0))
//        });
//    cout << "Dumping intermediate translation result buffers...\n";
//    for_each(jvt, [&](const sds::TranslationResult& e) { cout << e << '\n'; });
//    cout << "Dumping cleanup actions buffer...\n";
//    const auto cleanupActions = translator.GetCleanupActions();
//    for_each(cleanupActions, [&](const sds::TranslationResult& e) { cout << e << '\n'; });
//    cout << '\n';
//}
//
//inline
//auto RunCleanupTest()
//{
//    using namespace sds;
//    using namespace std::chrono_literals;
//    using std::ranges::for_each, std::cout;
//    TestMappingProvider testMaps{ VirtKey };
//    TestPollProvider testPoll{ VirtKey };
//    KeyboardActionTranslator translator{ testMaps.GetMappings() };
//    // It seems as though if all you are doing is feeding the output of one object to another object, they don't need
//    // to be dependent on one another at all. This design also makes testing the functionality quite simple, just change
//    // the source of input to the translator component to a source for test data.
//    auto jvt = std::views::join(
//        std::array{
//            TestCaller(translator(testPoll.GetDownState()).at(0)),
//            TestCaller(translator(testPoll.GetRepeatState()).at(0))
//        });
//    cout << "Dumping intermediate translation result buffers...\n";
//    for_each(jvt, [&](const sds::TranslationResult& e) { cout << e << '\n'; });
//    cout << "Dumping cleanup actions buffer...\n";
//    const auto cleanupActions = translator.GetCleanupActions();
//    for_each(cleanupActions, [&](const sds::TranslationResult& e) { cout << e << '\n'; });
//    cout << '\n';
//}
//
//inline
//auto RunOvertakingTest()
//{
//    using namespace sds;
//    using namespace std::chrono_literals;
//    using std::ranges::for_each, std::cout;
//    TestMappingProvider testMaps{ VirtKey };
//    TestPollProvider testPoll{ VirtKey };
//
//    auto testMappings = testMaps.GetMappings();
//    testMappings.front().ExclusivityGrouping = 100;
//    testMappings.back().ExclusivityGrouping = 100;
//
//    KeyboardActionTranslator translator{ testMaps.GetMappings() };
//    // It seems as though if all you are doing is feeding the output of one object to another object, they don't need
//    // to be dependent on one another at all. This design also makes testing the functionality quite simple, just change
//    // the source of input to the translator component to a source for test data.
//    auto jvt = std::views::join(
//        std::array{
//            TestCaller(translator(testPoll.GetDownState()).at(0)),
//            TestCaller(translator(testPoll.GetRepeatState()).at(0))
//        });
//    cout << "Dumping intermediate translation result buffers...\n";
//    for_each(jvt, [&](const sds::TranslationResult& e) { cout << e << '\n'; });
//    cout << "Dumping cleanup actions buffer...\n";
//    const auto cleanupActions = translator.GetCleanupActions();
//    for_each(cleanupActions, [&](const sds::TranslationResult& e) { cout << e << '\n'; });
//    cout << '\n';
//}

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
    exitFuture.wait();
    //RunProgressionTest();
    //RunCleanupTest();
}
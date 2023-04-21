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

int main()
{
    //RunProgressionTest();
    //RunCleanupTest();
}
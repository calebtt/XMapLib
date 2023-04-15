#include "pch.h"
#include "CppUnitTest.h"
#include "TestMappingProvider.h"
#include "TestPollProvider.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestKeyboard
{
    template<typename Poller_t>
    concept IsInputPoller = requires(Poller_t & t)
    {
        { t.GetUpdatedState(0) };
        { t.GetUpdatedState(0) } -> std::convertible_to<sds::ControllerStateWrapper>;
    };

	TEST_CLASS(TestKeyboard)
	{
        static constexpr unsigned short VirtKey{ VK_PAD_A };
	public:
		TEST_METHOD(TestMethod1)
		{
            RunProgressionTest();
            RunCleanupTest();
		}
        TEST_METHOD(VerifyTestPoller)
		{
            TestPollProvider tpp{ .Vk = VirtKey };
            Assert::IsTrue(tpp.GetDownState().KeyDown);
            Assert::IsTrue(tpp.GetUpState().KeyUp);
            Assert::IsTrue(tpp.GetRepeatState().KeyRepeat);
            const auto noState = tpp.GetNoState();
            const auto nsAllFalse = noState.KeyDown || noState.KeyUp || noState.KeyRepeat;
            Assert::IsFalse(nsAllFalse);
		}
        TEST_METHOD(VerifyTestMapsProvider)
		{
            TestMappingProvider testMaps{ VirtKey };
            const auto mapVec = testMaps.GetMappings();
            for(const auto& e : mapVec)
            {
                Assert::IsTrue(e.OnDown.has_value());
                Assert::IsTrue(e.OnUp.has_value());
                Assert::IsTrue(e.OnRepeat.has_value());
                Assert::IsTrue(e.OnReset.has_value());
            }
		}
    private:
        auto RunProgressionTest() -> void
        {
            using namespace sds;
            using namespace std::chrono_literals;
            using std::ranges::for_each, std::cout;
            TestMappingProvider testMaps{ VirtKey };
            TestPollProvider testPoll{ VirtKey };
            KeyboardActionTranslator translator{ testMaps.GetMappings() };
            // It seems as though if all you are doing is feeding the output of one object to another object, they don't need
            // to be dependent on one another at all. This design also makes testing the functionality quite simple, just change
            // the source of input to the translator component to a source for test data.
            auto jvt = std::views::join(
                std::array{
                    TestCaller(translator(testPoll.GetDownState()).at(0)),
                    TestCaller(translator(testPoll.GetRepeatState()).at(0)),
                    TestCaller(translator(testPoll.GetUpState()).at(0)),
                    TestCaller(translator(testPoll.GetNoState()).at(0))
                });
            cout << "Dumping intermediate translation result buffers...\n";
            for_each(jvt, [&](const sds::TranslationResult& e) { cout << e << '\n'; });
            cout << "Dumping cleanup actions buffer...\n";
            const auto cleanupActions = translator.GetCleanupActions();
            for_each(cleanupActions, [&](const sds::TranslationResult& e) { cout << e << '\n'; });
            cout << '\n';
        }
        auto RunCleanupTest() -> void
        {
            using namespace sds;
            using namespace std::chrono_literals;
            using std::ranges::for_each, std::cout;
            TestMappingProvider testMaps{ VirtKey };
            TestPollProvider testPoll{ VirtKey };
            KeyboardActionTranslator translator{ testMaps.GetMappings() };
            // It seems as though if all you are doing is feeding the output of one object to another object, they don't need
            // to be dependent on one another at all. This design also makes testing the functionality quite simple, just change
            // the source of input to the translator component to a source for test data.
            auto jvt = std::views::join(
                std::array{
                    TestCaller(translator(testPoll.GetDownState()).at(0)),
                    TestCaller(translator(testPoll.GetRepeatState()).at(0))
                });
            cout << "Dumping intermediate translation result buffers...\n";
            for_each(jvt, [&](const sds::TranslationResult& e) { cout << e << '\n'; });
            cout << "Dumping cleanup actions buffer...\n";
            const auto cleanupActions = translator.GetCleanupActions();
            for_each(cleanupActions, [&](const sds::TranslationResult& e) { cout << e << '\n'; });
            cout << '\n';
        }
    private:
        /**
         * \brief Function used to "perform" the action suggested by the TranslationResult and then update the
         * pointer-to mapping object's "LastState" member.
         * \remarks Calls the OnEvent function matching the TranslationResult state bool(s), if present, and then updates the
         * state machine.
         * \param tr The object upon which to perform the action.
         */
        auto CallAndUpdateTranslationResult(sds::TranslationResult& tr) -> void
        {
            auto DoIf = [&](const bool theCond, auto& theFnOpt, std::function<void()> updateFn) {
                if (theCond)
                    updateFn();
                if (theCond && theFnOpt) {
                    (*theFnOpt)();
                }
            };
            DoIf(tr.DoState.IsDown(), tr.ButtonMapping->OnDown, [&]() { tr.ButtonMapping->LastAction.SetDown(); });
            DoIf(tr.DoState.IsRepeating(), tr.ButtonMapping->OnRepeat, [&]() { tr.ButtonMapping->LastAction.SetRepeat(); });
            DoIf(tr.DoState.IsUp(), tr.ButtonMapping->OnUp, [&]() { tr.ButtonMapping->LastAction.SetUp(); });
            DoIf(tr.DoState.IsInitialState(), tr.ButtonMapping->OnReset, [&]() { tr.ButtonMapping->LastAction.SetInitial(); });
        }
		auto GetThreadUnit() const
        {
            using TUnit_t = imp::ThreadUnitPlusPlus;
            using SharedTUnit_t = std::shared_ptr<TUnit_t>;
            SharedTUnit_t threadUnit = std::make_shared<TUnit_t>();
            return threadUnit;
        }
		auto TestCaller(sds::TranslationResult& tr) -> std::array<sds::TranslationResult, 1>
        {
            CallAndUpdateTranslationResult(tr);
            return std::array{ tr };
        }
		auto RunOvertakingTest()
        {
            using namespace sds;
            using namespace std::chrono_literals;
            using std::ranges::for_each, std::cout;
            TestMappingProvider testMaps{ VirtKey };
            TestPollProvider testPoll{ VirtKey };

            auto testMappings = testMaps.GetMappings();
            testMappings.front().ExclusivityGrouping = 100;
            testMappings.back().ExclusivityGrouping = 100;

            KeyboardActionTranslator translator{ testMaps.GetMappings() };
            // It seems as though if all you are doing is feeding the output of one object to another object, they don't need
            // to be dependent on one another at all. This design also makes testing the functionality quite simple, just change
            // the source of input to the translator component to a source for test data.
            auto jvt = std::views::join(
                std::array{
                    TestCaller(translator(testPoll.GetDownState()).at(0)),
                    TestCaller(translator(testPoll.GetRepeatState()).at(0))
                });
            cout << "Dumping intermediate translation result buffers...\n";
            for_each(jvt, [&](const sds::TranslationResult& e) { cout << e << '\n'; });
            cout << "Dumping cleanup actions buffer...\n";
            const auto cleanupActions = translator.GetCleanupActions();
            for_each(cleanupActions, [&](const sds::TranslationResult& e) { cout << e << '\n'; });
            cout << '\n';
        }
	};
}

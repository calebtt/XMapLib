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

    inline
    auto PrintResultsWithMessage(std::vector<sds::TranslationResult>& trv, std::string_view initMessage) -> void
    {
        using std::stringstream, std::ranges::for_each;
        std::stringstream ss;
        ss << initMessage << '\n';
        for_each(trv, [&](const sds::TranslationResult& e) { ss << e << '\n'; });
        Logger::WriteMessage(ss.str().c_str());
    }

	TEST_CLASS(TestKeyboard)
	{
        static constexpr unsigned short VirtKey{ VK_PAD_A };
	public:
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
                Assert::IsTrue(e.Vk == VirtKey);
            }
		}
        TEST_METHOD(TestProgression)
		{
            using namespace sds;
            using namespace std::chrono_literals;
            // Test data provider objs
            TestMappingProvider testMaps{ VirtKey };
            TestPollProvider testPoll{ VirtKey };
            // Construct a translator object, which encapsulates some test mappings
            KeyboardActionTranslator translator{ testMaps.GetMappings() };

			// Pass a polled state for each phase into the translator, store the result
            const auto downPack = translator(testPoll.GetDownState());
            Assert::IsTrue(downPack.NextStateRequests.size() == 1);
            // Grab only the first elements (should only be 1 result per call)
            auto downResult = downPack.NextStateRequests.front();
            // Advance the pointed-to mapping in the translationresult to the next state
            CallAndUpdateTranslationResult(downResult);
            std::this_thread::sleep_for(1s);

            const auto repeatPack = translator(testPoll.GetRepeatState());
            Assert::IsTrue(repeatPack.RepeatRequests.size() == 1);
            auto repeatResult = repeatPack.RepeatRequests.front();
            CallAndUpdateTranslationResult(repeatResult);
            std::this_thread::sleep_for(1s);
            const auto upPack = translator(testPoll.GetUpState());
            Assert::IsTrue(upPack.NextStateRequests.size() == 1);
            auto upResult = upPack.NextStateRequests.front();
            CallAndUpdateTranslationResult(upResult);

            const auto noResultPack = translator(testPoll.GetNoState());
            //Assert::IsTrue(noResultPack.OvertakingRequests.size() == 1);
            //auto noResult = noResultPack.front();
            //CallAndUpdateTranslationResult(noResult);

            Assert::IsTrue(downResult.DoState.IsDown(), L"Translation for polled key-down wasn't down.");
            Assert::IsTrue(repeatResult.DoState.IsRepeating(), L"Translation for polled key-repeat wasn't repeat.");
            Assert::IsTrue(upResult.DoState.IsUp(), L"Translation for polled key-up wasn't up.");
            //Assert::IsTrue(noResult.DoState.IsInitialState(), L"Translation for polled none wasn't none.");
		}
        TEST_METHOD(TestCleanup)
		{
            using namespace sds;
            using namespace std::chrono_literals;
            using std::ranges::for_each, std::cout;
            TestMappingProvider testMaps{ VirtKey };
            TestPollProvider testPoll{ VirtKey };
            KeyboardActionTranslator translator{ testMaps.GetMappings() };

            // Pass a polled state for each phase into the translator, store the result
            const auto downVec = translator(testPoll.GetDownState());
            Assert::IsTrue(downVec.NextStateRequests.size() == 1);
            // Grab only the first elements (should only be 1 result per call)
            auto downResult = downVec.NextStateRequests.front();
            // Advance the pointed-to mapping in the translationresult to the next state
            CallAndUpdateTranslationResult(downResult);
            std::this_thread::sleep_for(1s);

            const auto repeatVec = translator(testPoll.GetRepeatState());
            Assert::IsTrue(repeatVec.RepeatRequests.size() == 1);
            auto repeatResult = repeatVec.RepeatRequests.front();
            CallAndUpdateTranslationResult(repeatResult);
            std::this_thread::sleep_for(1s);

            //Logger::WriteMessage("Dumping intermediate translation result buffers...\n");
            Logger::WriteMessage("Dumping cleanup actions buffer...\n");
            const auto cleanupActions = translator.GetCleanupActions();
            std::stringstream ss;
            for_each(cleanupActions, [&](const sds::TranslationResult& e)
            {
	            ss << e << '\n';
                Assert::IsTrue(e.DoState.IsUp(), L"Cleanup action is not 'up'.");
            });
            Logger::WriteMessage(ss.str().c_str());
		}
        TEST_METHOD(TestOvertaking)
        {
            using namespace sds;
            using namespace std::chrono_literals;
            using std::ranges::for_each, std::stringstream;
            // Test data provider objs
            TestMappingProvider testMaps{ VirtKey };
            TestPollProvider testPoll{ VirtKey };

            // Constructing two mappings with different vk, same ex. group
            auto mappings = testMaps.GetMapping(VirtKey, 101);
            mappings.append_range(testMaps.GetMapping(VirtKey + 1, 101));

        	// Translator obj and subject of test
            KeyboardActionTranslator translator{ mappings };

        	auto firstDownVec = translator(testPoll.GetDownState(VirtKey));
            Assert::IsTrue(firstDownVec.NextStateRequests.size() == 1);
            CallAndUpdateTranslationResult(firstDownVec.NextStateRequests.front());
            std::this_thread::sleep_for(1s);
            auto secondDownVec = translator(testPoll.GetDownState(VirtKey + 1));
            CallAndUpdateTranslationResult(secondDownVec.OvertakenRequests.front());
            CallAndUpdateTranslationResult(secondDownVec.NextStateRequests.front());
            Assert::IsTrue(secondDownVec.OvertakenRequests.size() == 1, L"No overtaken request in buf.");
            Assert::IsTrue(secondDownVec.NextStateRequests.size() == 1, L"Second call to translate should hold the newest key-down");

            //PrintResultsWithMessage(firstDownVec, "Dumping first down key buffer...");
            //PrintResultsWithMessage(secondDownVec, "Dumping overtaking actions buffer...");
        }
        TEST_METHOD(TestNotOvertaking)
        {
            using namespace sds;
            using namespace std::chrono_literals;
            using std::ranges::for_each, std::stringstream;
            // Test data provider objs
            TestMappingProvider testMaps{ VirtKey };
            TestPollProvider testPoll{ VirtKey };

            // Constructing two mappings with different vk, same ex. group
            auto mappings = testMaps.GetMapping(VirtKey, 101);
            mappings.append_range(testMaps.GetMapping(VirtKey + 1, 100));

            // Translator obj and subject of test
            KeyboardActionTranslator translator{ mappings };

            auto firstDownVec = translator(testPoll.GetDownState(VirtKey));
            Assert::IsTrue(firstDownVec.NextStateRequests.size() == 1);
            CallAndUpdateTranslationResult(firstDownVec.NextStateRequests.front());
            std::this_thread::sleep_for(1s);
            auto secondDownVec = translator(testPoll.GetDownState(VirtKey + 1));
            CallAndUpdateTranslationResult(secondDownVec.NextStateRequests.front());
            Assert::IsTrue(secondDownVec.OvertakenRequests.empty(), L"Overtaken request in buf, should not be.");
            Assert::IsTrue(secondDownVec.NextStateRequests.size() == 1, L"Second call to translate should hold the newest key-down");

            //PrintResultsWithMessage(firstDownVec, "Dumping first down key buffer...");
            //PrintResultsWithMessage(secondDownVec, "Dumping overtaking actions buffer...");
        }

        TEST_METHOD(TestManyMappingsProgression)
        {
	        
        }
    private:
        /**
         * \brief Function used to "perform" the action suggested by the TranslationResult and then update the
         * pointer-to mapping object's "LastState" member.
         * \remarks Calls the OnEvent function matching the TranslationResult state bool(s), if present, and then updates the
         * state machine.
         * \param tr The object upon which to perform the action.
         */
        static
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
    private:
		auto GetThreadUnit() const
        {
            using TUnit_t = imp::ThreadUnitPlusPlus;
            using SharedTUnit_t = std::shared_ptr<TUnit_t>;
            SharedTUnit_t threadUnit = std::make_shared<TUnit_t>();
            return threadUnit;
        }
	};
}

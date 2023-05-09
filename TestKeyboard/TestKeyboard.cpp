#include "pch.h"
#include "CppUnitTest.h"
#include "TestMappingProvider.h"
#include "TestPollProvider.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestKeyboard
{
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
            auto mapVec = testMaps.GetMappings();
            for(auto& e : mapVec)
            {
                Assert::IsTrue(static_cast<bool>(e.OnDown));
                Assert::IsTrue(static_cast<bool>(e.OnUp));
                Assert::IsTrue(static_cast<bool>(e.OnRepeat));
                Assert::IsTrue(static_cast<bool>(e.OnReset));
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
            auto downPack = translator(testPoll.GetDownState());
            Assert::IsTrue(downPack.NextStateRequests.size() == 1);
            // Grab only the first elements (should only be 1 result per call)
            auto& downResult = downPack.NextStateRequests.front();
            // Advance the pointed-to mapping in the translationresult to the next state
            CallAndUpdateTranslationResult(downResult);
            std::this_thread::sleep_for(1s);

            auto repeatPack = translator(testPoll.GetRepeatState());
            Assert::IsTrue(repeatPack.RepeatRequests.size() == 1);
            auto& repeatResult = repeatPack.RepeatRequests.front();
            CallAndUpdateTranslationResult(repeatResult);
            std::this_thread::sleep_for(1s);
            auto upPack = translator(testPoll.GetUpState());
            Assert::IsTrue(upPack.NextStateRequests.size() == 1);
            auto& upResult = upPack.NextStateRequests.front();
            CallAndUpdateTranslationResult(upResult);

            const auto noResultPack = translator(testPoll.GetNoState());

            Assert::IsTrue(downResult.DoState.IsDown(), L"Translation for polled key-down wasn't down.");
            Assert::IsTrue(repeatResult.DoState.IsRepeating(), L"Translation for polled key-repeat wasn't repeat.");
            Assert::IsTrue(upResult.DoState.IsUp(), L"Translation for polled key-up wasn't up.");
            //Assert::IsTrue(noResult.DoState.IsInitialState(), L"Translation for polled none wasn't none.");
		}
        TEST_METHOD(TestCleanup)
		{
            //TODO add another progression that moves the state to repeat before testing the cleanup
            using namespace sds;
            using namespace std::chrono_literals;
            using std::ranges::for_each, std::cout;
            TestMappingProvider testMaps{ VirtKey };
            TestPollProvider testPoll{ VirtKey };
            KeyboardActionTranslator translator{ testMaps.GetMappings() };

            // Pass a polled state for each phase into the translator, store the result
            auto downVec = translator(testPoll.GetDownState());
            Assert::IsTrue(downVec.NextStateRequests.size() == 1);
            // Grab only the first elements (should only be 1 result per call)
            auto& downResult = downVec.NextStateRequests.front();
            // Advance the pointed-to mapping in the translationresult to the next state
            CallAndUpdateTranslationResult(downResult);
            std::this_thread::sleep_for(1s);

            auto repeatVec = translator(testPoll.GetRepeatState());
            Assert::IsTrue(repeatVec.RepeatRequests.size() == 1);
            auto& repeatResult = repeatVec.RepeatRequests.front();
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
            //TODO add another progression that moves the state to repeat before testing the overtaking
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
            using namespace sds;
            using namespace std::chrono_literals;
            constexpr auto SleepDelay = 500ms;
            // Test data provider objs
            TestMappingProvider testMaps{ VirtKey };
            TestPollProvider testPoll{ VirtKey };

            std::vector<CBActionMap> mappings;
            mappings.append_range(testMaps.GetMapping(VirtKey));
            mappings.append_range(testMaps.GetMapping(VirtKey + 1));
            mappings.append_range(testMaps.GetMapping(VirtKey + 2));
            mappings.append_range(testMaps.GetMapping(VirtKey + 3));
            mappings.append_range(testMaps.GetMapping(VirtKey + 4, 101));
            mappings.append_range(testMaps.GetMapping(VirtKey + 5, 101));
            mappings.append_range(testMaps.GetMapping(VirtKey + 6, 101));

            // Construct a translator object, which encapsulates some test mappings
            KeyboardActionTranslator translator{ std::move(mappings) };

            // Perform a slightly more complex test of the progression, now with more mappings.
            auto vkDown = translator(testPoll.GetDownState(VirtKey));
            AssertTranslationPackSizes(vkDown, 0, 0, 0, 1);
            auto& vkDownResult = vkDown.NextStateRequests.front();
            Assert::IsTrue(vkDownResult.DoState.IsDown());
            CallAndUpdateTranslationResult(vkDownResult);

            std::this_thread::sleep_for(SleepDelay);

            auto vk1Down = translator(testPoll.GetDownState(VirtKey + 1));
            AssertTranslationPackSizes(vk1Down, 0, 1, 0, 1);
            auto& vk1DownResult = vk1Down.NextStateRequests.front();
            Assert::IsTrue(vk1DownResult.DoState.IsDown());
            CallAndUpdateTranslationResult(vk1DownResult);
            
        	std::this_thread::sleep_for(SleepDelay);

            // Make sure there are 2 repeat requests for both keys down.
            auto repeatTestPack = translator(testPoll.GetNoState(VirtKey));
            AssertTranslationPackSizes(repeatTestPack, 0, 2, 0, 0);

        	std::this_thread::sleep_for(SleepDelay);

            // Send vk+1 up
        	auto vk1Up = translator(testPoll.GetUpState(VirtKey + 1));
            // There is also a repeat request because VirtKey is still "down" and we waited an entire second
            // but only 1 repeat request, as this key-up request should remove the repeat for this down key.
            AssertTranslationPackSizes(vk1Up, 0, 1, 0, 1);
            auto& vk1UpResult = vk1Up.NextStateRequests.front();
            Assert::IsTrue(vk1UpResult.DoState.IsUp());
            CallAndUpdateTranslationResult(vk1UpResult);

            std::this_thread::sleep_for(SleepDelay);

            // Send vk+4 down (ex. grp. 101)
            // should still be vk+0 down/repeating, a reset for the previous vk+1 up, and a new vk+4 down
            auto vk4Down = translator(testPoll.GetDownState(VirtKey + 4));
            AssertTranslationPackSizes(vk4Down, 1, 1, 0, 1);
            auto& vk4DownResult = vk4Down.NextStateRequests.front();
            Assert::IsTrue(vk4DownResult.DoState.IsDown());
            CallAndUpdateTranslationResult(vk4DownResult);

            std::this_thread::sleep_for(SleepDelay);

            auto vk5Down = translator(testPoll.GetDownState(VirtKey + 5));
            AssertTranslationPackSizes(vk5Down, 1, 2, 1, 1);
            auto& vk5DownResult = vk5Down.NextStateRequests.front();
            auto& vk5OvertakenResult = vk5Down.OvertakenRequests.front();
            Assert::IsTrue(vk5DownResult.DoState.IsDown());
            Assert::IsTrue(vk5OvertakenResult.DoState.IsUp());
            CallAndUpdateTranslationResult(vk5Down.OvertakenRequests.front());
            CallAndUpdateTranslationResult(vk5DownResult);

            // Cleanup actions
            Logger::WriteMessage("Performing cleanup actions...\n");
            auto cleanupActions = translator.GetCleanupActions();
            for(auto& e : cleanupActions)
            {
                CallAndUpdateTranslationResult(e);
            }
        }
        TEST_METHOD(TestBenchmarkTranslator)
        {
            using namespace sds;
            using namespace std::chrono_literals;
            namespace chron = std::chrono;
            static constexpr std::size_t TestIterations{ 1'000 };

            Logger::WriteMessage("Beginning timed test of many translation iterations.\n");
            // Test data provider objs
            TestMappingProvider testMaps{ VirtKey };
            KeyboardPollerController keyPoller{ 0 };

            // Note that these mappings write to the test logger, might add some latency.
            std::vector<CBActionMap> mappings;
            mappings.append_range(testMaps.GetMapping(VK_PAD_A));
            mappings.append_range(testMaps.GetMapping(VK_PAD_B));
            mappings.append_range(testMaps.GetMapping(VK_PAD_X));
            mappings.append_range(testMaps.GetMapping(VK_PAD_Y));
            mappings.append_range(testMaps.GetMapping(VK_GAMEPAD_LEFT_THUMBSTICK_UP, 101));
            mappings.append_range(testMaps.GetMapping(VK_GAMEPAD_LEFT_THUMBSTICK_DOWN, 101));
            mappings.append_range(testMaps.GetMapping(VK_GAMEPAD_LEFT_THUMBSTICK_LEFT, 101));
            mappings.append_range(testMaps.GetMapping(VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT, 101));

            // Construct a translator object, which encapsulates some test mappings
            KeyboardActionTranslator translator{ std::move(mappings) };

            const auto beginTime = chron::steady_clock::now();
            for(std::size_t i{}; i < TestIterations; ++i)
            {
                translator(keyPoller())();
            }
            const auto endTime = chron::steady_clock::now() - beginTime;
            const auto indivTime = chron::nanoseconds( endTime.count() / TestIterations);
            const auto indivTimeUs = chron::duration_cast<chron::microseconds>(indivTime);
            const auto indivTimeMs = chron::duration_cast<chron::milliseconds>(indivTime);

            const auto totalMsg = std::format("Total time for {} test iterations is: {}\n", TestIterations, endTime);
            const auto indivMsg = std::format("Avg. time for each test iteration is: {} -or- {} -or- {}\n", indivTime, indivTimeUs, indivTimeMs);
            const auto timeMsg = totalMsg + indivMsg;
            Logger::WriteMessage(timeMsg.c_str());
        }
    private:
        static
        void AssertTranslationPackSizes(
            const sds::TranslationPack& p,
            const std::size_t updateCount, 
            const std::size_t repeatCount,
            const std::size_t overtakenCount,
            const std::size_t nextCount)
        {
            Assert::IsTrue(p.UpdateRequests.size() == updateCount, L"Update req size mismatch.");
            Assert::IsTrue(p.RepeatRequests.size() == repeatCount, L"Repeat req size mismatch.");
            Assert::IsTrue(p.OvertakenRequests.size() == overtakenCount, L"Overtaken req size mismatch.");
            Assert::IsTrue(p.NextStateRequests.size() == nextCount, L"Next state req size mismatch.");
        }
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
            // Don't need to test for containing a function, they will--just might not do anything.
        	tr.OperationToPerform();
            tr.AdvanceStateFn();
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

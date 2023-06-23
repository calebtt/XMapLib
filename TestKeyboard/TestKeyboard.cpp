#include "pch.h"
#include "CppUnitTest.h"
#include "TestMappingProvider.h"
#include "TestPollProvider.h"
#include <filesystem>
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
                Assert::IsTrue(e.ButtonVirtualKeycode == VirtKey);
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

            Assert::IsTrue(downResult.DoState == ActionState::KEYDOWN, L"Translation for polled key-down wasn't down.");
            Assert::IsTrue(repeatResult.DoState == ActionState::KEYREPEAT, L"Translation for polled key-repeat wasn't repeat.");
            Assert::IsTrue(upResult.DoState == ActionState::KEYUP, L"Translation for polled key-up wasn't up.");
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
                Assert::IsTrue(e.DoState == ActionState::KEYUP, L"Cleanup action is not 'up'.");
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
        TEST_METHOD(TestOvertakingRepeated)
        {
            // This test is intended to test the case wherein, for example,
            // ltrigger down, rtrigger down overtakes and puts ltrigger up, then user releases and presses ltrigger again
            // This is a test for a case found manually by using the test driver.
            using namespace sds;
            using namespace std::chrono_literals;
            using std::ranges::for_each, std::stringstream;
            
            TestMappingProvider testMaps{ VirtKey };
            TestPollProvider testPoll{ VirtKey };

            // Constructing two mappings with different vk, same ex. group
            auto mappings = testMaps.GetMapping(VirtKey, 101);
            mappings.append_range(testMaps.GetMapping(VirtKey + 1, 101));

            // Translator obj and subject of test
            KeyboardActionTranslator translator{ mappings };
            std::this_thread::sleep_for(500ms);

            const auto setVk1Down = [&]()
            {
                // Set vk1 down
                const auto firstDownVec = translator(testPoll.GetDownState(VirtKey));
                Assert::IsTrue(firstDownVec.NextStateRequests.size() == 1);
                firstDownVec();
            };
            const auto setVk1Up = [&]()
            {
                // Set vk1 up
                const auto firstUpVec = translator(testPoll.GetUpState(VirtKey));
                Assert::IsTrue(firstUpVec.NextStateRequests.size() == 1);
                firstUpVec();
            };
            const auto setVk2Down = [&]()
            {
                // Set vk2 down
                const auto secondDownVec = translator(testPoll.GetDownState(VirtKey + 1));
                Assert::IsTrue(secondDownVec.NextStateRequests.size() == 1, L"call to translate should hold the key-down");
                secondDownVec();
            };
            const auto setVk2Up = [&]()
            {
                // Set vk2 up
                const auto secondUpVec = translator(testPoll.GetUpState(VirtKey + 1));
                Assert::IsTrue(secondUpVec.NextStateRequests.size() == 1, L"call to translate should hold the key-down");
                secondUpVec();
            };
            const auto updateLoop = [&]()
            {
                // Update
                const auto updateVec = translator(testPoll.GetNoState());
                updateVec();
            };
            const auto doCleanup = [&]()
            {
                const auto cleanupActions1 = translator.GetCleanupActions();
                for (const auto& e : cleanupActions1)
                    e();
                const auto repeatTest = translator(testPoll.GetNoState());
                Assert::IsTrue(repeatTest.RepeatRequests.empty());
            };
            Logger::WriteMessage("Beginning vk1down while vk2 down/up in a loop.\n");
            // Start by testing vk1 down while vk2 down/up in a loop
            setVk1Down();
            std::this_thread::sleep_for(500ms);

            for (int i = 0; i < 3; i++)
            {
                std::this_thread::sleep_for(500ms);
                setVk2Down();
                std::this_thread::sleep_for(500ms);
                setVk2Up();
                std::this_thread::sleep_for(500ms);
                updateLoop();
            }
            Logger::WriteMessage("Beginning cleanup.\n");
            // Cleanup
            doCleanup();
            Logger::WriteMessage("Beginning vk2down while vk1 down/up in a loop.\n");
            // Test vk2 down while vk1 down/up in a loop
            setVk2Down();
            std::this_thread::sleep_for(500ms);
            updateLoop(); // TODO an exclusivity group match will not reset! Might be a problem with the real poller.
            
            for (int i = 0; i < 3; i++)
            {
                std::this_thread::sleep_for(500ms);
                setVk1Down();
                std::this_thread::sleep_for(500ms);
                setVk1Up();
                std::this_thread::sleep_for(500ms);
                updateLoop();
            }
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
            Assert::IsTrue(vkDownResult.DoState == ActionState::KEYDOWN);
            CallAndUpdateTranslationResult(vkDownResult);

            std::this_thread::sleep_for(SleepDelay);

            auto vk1Down = translator(testPoll.GetDownState(VirtKey + 1));
            AssertTranslationPackSizes(vk1Down, 0, 1, 0, 1);
            auto& vk1DownResult = vk1Down.NextStateRequests.front();
            Assert::IsTrue(vk1DownResult.DoState == ActionState::KEYDOWN);
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
            Assert::IsTrue(vk1UpResult.DoState == ActionState::KEYUP);
            CallAndUpdateTranslationResult(vk1UpResult);

            std::this_thread::sleep_for(SleepDelay);

            // Send vk+4 down (ex. grp. 101)
            // should still be vk+0 down/repeating, a reset for the previous vk+1 up, and a new vk+4 down
            auto vk4Down = translator(testPoll.GetDownState(VirtKey + 4));
            AssertTranslationPackSizes(vk4Down, 1, 1, 0, 1);
            auto& vk4DownResult = vk4Down.NextStateRequests.front();
            Assert::IsTrue(vk4DownResult.DoState == ActionState::KEYDOWN);
            CallAndUpdateTranslationResult(vk4DownResult);

            std::this_thread::sleep_for(SleepDelay);

            auto vk5Down = translator(testPoll.GetDownState(VirtKey + 5));
            AssertTranslationPackSizes(vk5Down, 1, 2, 1, 1);
            auto& vk5DownResult = vk5Down.NextStateRequests.front();
            auto& vk5OvertakenResult = vk5Down.OvertakenRequests.front();
            Assert::IsTrue(vk5DownResult.DoState == ActionState::KEYDOWN);
            Assert::IsTrue(vk5OvertakenResult.DoState == ActionState::KEYUP);
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
                // TODO measure the time it takes to call Xinput's get state differently.
                // Perhaps in the test driver.
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
        TEST_METHOD(TestExclusivityGroupsAlgorithm)
        {
	        // Tests the function attempting to detect when multiple exclusivity groupings are mapped
            // to the same key/vk/id.
            TestMappingProvider tmp;
            auto mapList = tmp.GetMapping(1, 101);
            mapList.append_range(tmp.GetMapping(1, 102));
            const auto testResult = sds::AreExclusivityGroupsUnique(mapList);
            Assert::IsFalse(testResult, L"Failed to detect multiple exclusivity groupings mapped to same vk.");
        }
        TEST_METHOD(TestTranslatorConstructors)
        {
            /* Here we are going to test that translator objects constructed by either Ctor
             * both produce the same result.
             */
            using namespace sds;
            using namespace std::chrono_literals;
            // Test data provider objs
            TestMappingProvider testMaps{ VirtKey };
            TestPollProvider testPoll{ VirtKey };
            const auto copyMappings = testMaps.GetMappings();
            auto moveMappings = testMaps.GetMappings();

            // Construct both translator objects, either copying or moving test mappings.
            KeyboardActionTranslator copyTranslator{ copyMappings };
            KeyboardActionTranslator moveTranslator{ std::move(moveMappings) };

            // Pass a polled state for each phase into the translator, store the result
            auto copyDownPack = copyTranslator(testPoll.GetDownState());
            auto moveDownPack = moveTranslator(testPoll.GetDownState());

        	Assert::IsTrue(copyDownPack.NextStateRequests.size() == 1);
            Assert::IsTrue(moveDownPack.NextStateRequests.size() == copyDownPack.NextStateRequests.size());

            // Grab only the first elements (should only be 1 result per call)
            auto& copyDownResult = copyDownPack.NextStateRequests.front();
            auto& moveDownResult = moveDownPack.NextStateRequests.front();

        	// Advance the pointed-to mapping in the translationresult to the next state
            CallAndUpdateTranslationResult(copyDownResult);
            CallAndUpdateTranslationResult(moveDownResult);

            std::this_thread::sleep_for(1s);

            auto copyRepeatPack = copyTranslator(testPoll.GetRepeatState());
            auto moveRepeatPack = moveTranslator(testPoll.GetRepeatState());

            Assert::IsTrue(copyRepeatPack.RepeatRequests.size() == 1);
            Assert::IsTrue(moveRepeatPack.RepeatRequests.size() == copyRepeatPack.RepeatRequests.size());

            auto& copyRepeatResult = copyRepeatPack.RepeatRequests.front();
            auto& moveRepeatResult = moveRepeatPack.RepeatRequests.front();
            CallAndUpdateTranslationResult(copyRepeatResult);
            CallAndUpdateTranslationResult(moveRepeatResult);

            std::this_thread::sleep_for(1s);

            auto copyUpPack = copyTranslator(testPoll.GetUpState());
            auto moveUpPack = moveTranslator(testPoll.GetUpState());

            Assert::IsTrue(copyUpPack.NextStateRequests.size() == 1);
            Assert::IsTrue(moveUpPack.NextStateRequests.size() == copyUpPack.NextStateRequests.size());

            auto& copyUpResult = copyUpPack.NextStateRequests.front();
            auto& moveUpResult = moveUpPack.NextStateRequests.front();
            CallAndUpdateTranslationResult(copyUpResult);
            CallAndUpdateTranslationResult(moveUpResult);

            const auto copyNoResultPack = copyTranslator(testPoll.GetNoState());
            const auto moveNoResultPack = moveTranslator(testPoll.GetNoState());

            Assert::IsTrue(copyDownResult.DoState == ActionState::KEYDOWN, L"Translation for polled key-down wasn't down.");
            Assert::IsTrue(copyRepeatResult.DoState == ActionState::KEYREPEAT, L"Translation for polled key-repeat wasn't repeat.");
            Assert::IsTrue(copyUpResult.DoState == ActionState::KEYUP, L"Translation for polled key-up wasn't up.");
            Assert::IsTrue(moveDownResult.DoState == ActionState::KEYDOWN, L"Translation for polled key-down wasn't down.");
            Assert::IsTrue(moveRepeatResult.DoState == ActionState::KEYREPEAT, L"Translation for polled key-repeat wasn't repeat.");
            Assert::IsTrue(moveUpResult.DoState == ActionState::KEYUP, L"Translation for polled key-up wasn't up.");
        }
        TEST_METHOD(TestBenchmarkFromRecording)
        {
            using namespace sds;
            using namespace std::chrono_literals;
            namespace chron = std::chrono;

            Logger::WriteMessage("Beginning timed test of recorded input polled data.\n");
            TestMappingProvider testMaps{ VirtKey };
            const auto pollBuffer = LoadPollRecording();
            Assert::IsFalse(pollBuffer.empty());
            Logger::WriteMessage(std::format("\nLoaded {} recorded poll inputs.\n", pollBuffer.size()).c_str());

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
            for(std::size_t i{}; i < pollBuffer.size(); ++i)
            {
                const auto translationResult = translator(pollBuffer[i]);
                translationResult();
            }
            const auto endTime = chron::steady_clock::now() - beginTime;
            const auto indivTime = chron::nanoseconds(endTime.count() / pollBuffer.size());
            const auto indivTimeUs = chron::duration_cast<chron::microseconds>(indivTime);
            const auto indivTimeMs = chron::duration_cast<chron::milliseconds>(indivTime);

            const auto totalMsg = std::format("\nTotal time for {} test iterations is: {}\n", pollBuffer.size(), endTime);
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
        auto LoadPollRecording() const -> std::vector<sds::ControllerStateWrapper>
		{
            const auto currentPath = std::filesystem::current_path();
            Logger::WriteMessage(currentPath.c_str());
            std::vector<sds::ControllerStateWrapper> pollBuffer;
            std::ifstream inFile("../../TestKeyboard/TestData/recording.txt", std::ios::binary);
            while(inFile)
            {
                sds::ControllerStateWrapper current{};
                std::string tempBuffer;
                std::stringstream ss;

            	std::getline(inFile, tempBuffer);
                ss << tempBuffer << " ";
                std::getline(inFile, tempBuffer);
                ss << tempBuffer << " ";
                std::getline(inFile, tempBuffer);
                ss << tempBuffer << " ";
                std::getline(inFile, tempBuffer);
                ss << tempBuffer << " ";

                ss >> current.VirtualKey >> current.KeyDown >> current.KeyUp >> current.KeyRepeat;
                //ss >> current.KeyRepeat >> current.KeyUp >> current.KeyDown >> current.VirtualKey;
                pollBuffer.emplace_back(current);
            }
            return pollBuffer;
		}
	};
}

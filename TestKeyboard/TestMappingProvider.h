#pragma once
#include "pch.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestKeyboard
{
    struct TestMappingProvider
    {
        unsigned short Vk{ VK_PAD_A };

        auto GetMappings() const
        {
            using namespace sds;
            std::vector<CBActionMap> mappings;
            CBActionMap tm{
                .Vk = Vk,
                .UsesRepeat = true,
                .ExclusivityGrouping = {},
                .OnDown = []() { Logger::WriteMessage("Action:[Down]\n"); },
                .OnUp = []() { Logger::WriteMessage("Action:[Up]\n"); },
                .OnRepeat = []() { Logger::WriteMessage("Action:[Repeat]\n"); },
                .OnReset = []() { Logger::WriteMessage("Action:[Reset]\n"); },
                .CustomRepeatDelay = {},
                .LastAction = {}
            };
            mappings.emplace_back(tm);
            return mappings;
        }
        auto GetMapping(const unsigned short newVk, std::optional<int> exGroup = {}) const
        {
            using namespace sds;
            std::vector<CBActionMap> mappings;
            CBActionMap tm{
                .Vk = newVk,
                .UsesRepeat = true,
                .ExclusivityGrouping = exGroup,
                .OnDown = [newVk](){ Logger::WriteMessage("Action:[Down]\n"); },
                .OnUp = []() { Logger::WriteMessage("Action:[Up]\n"); },
                .OnRepeat = []() { Logger::WriteMessage("Action:[Repeat]\n"); },
                .OnReset = []() { Logger::WriteMessage("Action:[Reset]\n"); },
                .CustomRepeatDelay = {},
                .LastAction = {}
            };
            mappings.emplace_back(tm);
            return mappings;
        }
        //auto GetOvertakingMappings() const
        //{
        //    using namespace sds;
        //    std::vector<CBActionMap> mappings;
        //    CBActionMap tm{
        //        .Vk = Vk,
        //        .UsesRepeat = false,
        //        .ExclusivityGrouping = 101,
        //        .OnDown = []() { Logger::WriteMessage("Action:[Map1]:[Down]\n"); },
        //        .OnUp = []() { Logger::WriteMessage("Action:[Map1]:[Up]\n"); },
        //        .OnRepeat = []() { Logger::WriteMessage("Action:[Map1]:[Repeat]\n"); },
        //        .OnReset = []() { Logger::WriteMessage("Action:[Map1]:[Reset]\n"); },
        //        .CustomRepeatDelay = {},
        //        .LastAction = {}
        //    };
	       // CBActionMap tm2{
        //        .Vk = Vk+1, //Different vk for this mapping
        //        .UsesRepeat = false,
        //        .ExclusivityGrouping = 101,
        //        .OnDown = []() { Logger::WriteMessage("Action:[Map2]:[Down]\n"); },
        //        .OnUp = []() { Logger::WriteMessage("Action:[Map2]:[Up]\n"); },
        //        .OnRepeat = []() { Logger::WriteMessage("Action:[Map2]:[Repeat]\n"); },
        //        .OnReset = []() { Logger::WriteMessage("Action:[Map2]:[Reset]\n"); },
        //        .CustomRepeatDelay = {},
        //        .LastAction = {}
        //    };
        //    mappings.emplace_back(tm);
        //    mappings.emplace_back(tm2);
        //    return mappings;
        //}
    };
}
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
            const std::string vkString = "Vk:[" + std::to_string(Vk) + "]\n";
            const std::string downMessage = "Action:[Down] " + vkString;
            const std::string upMessage = "Action:[Up] " + vkString;
            const std::string repeatMessage = "Action:[Repeat] " + vkString;
            const std::string resetMessage = "Action:[Reset] " + vkString;
            std::vector<CBActionMap> mappings;
            CBActionMap tm{
                .Vk = Vk,
                .UsesRepeatBehavior = true,
                .ExclusivityGrouping = {},
                .OnDown = [=]() { Logger::WriteMessage(downMessage.c_str()); },
                .OnUp = [=]() { Logger::WriteMessage(upMessage.c_str()); },
                .OnRepeat = [=]() { Logger::WriteMessage(repeatMessage.c_str()); },
                .OnReset = [=]() { Logger::WriteMessage(resetMessage.c_str()); },
                .LastAction = {}
            };
            mappings.emplace_back(tm);
            return mappings;
        }
        auto GetMapping(const unsigned short newVk, std::optional<int> exGroup = {}) const
        {
            using namespace sds;
            const std::string vkString = "Vk:[" + std::to_string(newVk) + "]\n";
            const std::string downMessage = "Action:[Down] " + vkString;
            const std::string upMessage = "Action:[Up] " + vkString;
            const std::string repeatMessage = "Action:[Repeat] " + vkString;
            const std::string resetMessage = "Action:[Reset] " + vkString;

            std::vector<CBActionMap> mappings;
            CBActionMap tm{
                .Vk = newVk,
                .UsesRepeatBehavior = true,
                .ExclusivityGrouping = exGroup,
                .OnDown = [=](){ Logger::WriteMessage(downMessage.c_str()); },
                .OnUp = [=]() { Logger::WriteMessage(upMessage.c_str()); },
                .OnRepeat = [=]() { Logger::WriteMessage(repeatMessage.c_str()); },
                .OnReset = [=]() { Logger::WriteMessage(resetMessage.c_str()); },
                .LastAction = {}
            };
            mappings.emplace_back(tm);
            return mappings;
        }
    };
}
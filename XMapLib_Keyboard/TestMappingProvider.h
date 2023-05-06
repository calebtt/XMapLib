#pragma once
#include "ControllerButtonToActionMap.h"

struct TestMappingProvider
{
    unsigned short Vk{ VK_PAD_A };

    auto GetMappings() const
    {
        using namespace sds;
        std::vector<CBActionMap> mappings;
        CBActionMap tm{
            .Vk = Vk,
            .UsesInfiniteRepeat = false,
            .ExclusivityGrouping = {},
            .OnDown = []() { std::cout << "Action:[Down]\n"; },
            .OnUp = []() { std::cout << "Action:[Up]\n"; },
            .OnRepeat = []() { std::cout << "Action:[Repeat]\n"; },
            .OnReset = []() { std::cout << "Action:[Reset]\n"; },
            .DelayForRepeats = {},
            .LastAction = {}
        };
        mappings.emplace_back(tm);
        return mappings;
    }
};
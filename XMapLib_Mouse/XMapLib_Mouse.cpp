// XMapLib_Mouse.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "LibIncludes.h"
#include <iostream>
#include "MouseMapper.h"

inline
auto MainLoop()
{
    using namespace std::chrono_literals;
    using std::cout;
    GetterExit ge{};
    while (ge())
    {
        std::this_thread::sleep_for(2s);
        cout << "Still running...\n";
    }
}

inline
auto GetThreadUnit()
{
    using TUnit_t = imp::ThreadUnitFP;
    using SharedTUnit_t = std::shared_ptr<TUnit_t>;
    SharedTUnit_t threadUnit = std::make_shared<TUnit_t>();
    return threadUnit;
}

inline
auto GetMouseType()
{
    return sds::MouseMapper<>{GetThreadUnit()};
}

int main()
{
    using namespace std::chrono_literals;
    using std::cout;

    auto mp = GetMouseType();
    mp.Start();
    const auto sensMsg = mp.SetSensitivity(100);
    if(!sensMsg.empty())
    {
        std::cerr << std::format("Error setting sensitivty level: {}", sensMsg);
    }
    MainLoop();

    mp.Stop();
}

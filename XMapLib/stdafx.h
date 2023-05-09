// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <Xinput.h>
#include <tchar.h>

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>
#include <utility>
#include <limits>
#include <thread>
#include <memory>
#include <mutex>
#include <atomic>
#include <functional>
#include <tuple>
#include <locale>
#include <format>
#include <chrono>
#include <variant>
#include <array>
#include <numbers>
#include <fstream>

#include <ranges>

#include <cassert>


// Include some commonly used global stuff.

#include "../XMapLib_Keyboard/KeyboardPollerController.h"

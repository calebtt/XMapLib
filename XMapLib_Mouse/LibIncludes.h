#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif
#include <Windows.h>
#include <Xinput.h>
#include <tchar.h>

#include <iostream>
#include <syncstream>
#include <sstream>
#include <fstream>

#include <string>
#include <vector>
#include <map>
#include <array>
#include <tuple>
#include <variant>
#include <optional>

#include <algorithm>
#include <utility>

#include <thread>
#include <memory>
#include <mutex>
#include <atomic>
#include <functional>

#include <locale>
#include <format>

#include <chrono>

#include <numbers>
#include <limits>

#include <ranges>

#include <cassert>

// Begin custom includes
#include "../DelayManagerProj/DelayManager/DelayManager.hpp"

#include "../PolarCode/PolarQuadrantCalc/PolarCalc.h"
#include "../PolarCode/PolarQuadrantCalc/PolarCalcFaster.h"
#include "../PolarCode/PolarQuadrantCalc/PolarTransform.h"
#include "../PolarCode/PolarQuadrantCalc/PolarTransformMag.h"
#include "../PolarCode/PolarQuadrantCalc/PolarTransformOptional.h"

#include "../XMapLib_Utils/Arithmetic.h"
#include "../XMapLib_Utils/ControllerStatus.h"
#include "../XMapLib_Utils/CallbackRange.h"
#include "../XMapLib_Utils/SendKeyInput.h"
#include "../XMapLib_Utils/SendMouseInput.h"
#include "../XMapLib_Utils/TPrior.h"
#include "../XMapLib_Utils/VirtualMap.h"
#include "../XMapLib_Utils/Smarts.h"
#include "../XMapLib_Utils/GetterExit.h"

#include "../impcool_sol/immutable_thread_pool/ThreadUnitPlusPlus.h"
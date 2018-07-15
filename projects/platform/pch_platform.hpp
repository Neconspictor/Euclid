// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <algorithm>
#include <assert.h>
#include <atomic>
#include <cassert>
#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <locale>
#include <memory>
#include <mutex>
#include <queue>
#include <ratio>
#include <sstream>
#include <stdio.h>
#include <string>
#include <thread>
#include <time.h>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/any.hpp>
#include <boost/filesystem.hpp>
#include <boost/predef.h>
#include <boost/thread/sync_queue.hpp>
#include <boost/thread/concurrent_queues/detail/sync_deque_base.hpp>

//#include <utf8.h>

// platform specific includes

#ifdef _WIN32
#include <vld.h>
#include <Windows.h>
#endif
// TODO: reference additional headers your program requires here

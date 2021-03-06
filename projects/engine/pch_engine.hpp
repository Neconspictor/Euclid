#pragma once


#include <algorithm>
#include <assert.h>
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <locale>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <ratio>
#include <set>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <tchar.h>
#include <thread>
#include <time.h>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <boost/any.hpp>
#include <boost/filesystem.hpp>
#include <boost/function/function1.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <boost/predef.h>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/thread/sync_queue.hpp>
#include <boost/thread/concurrent_queues/detail/sync_deque_base.hpp>

#include <nex/common/File.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
//#include <glm/gtc/matrix_transform.inl>
#include <glm/gtc/epsilon.hpp>
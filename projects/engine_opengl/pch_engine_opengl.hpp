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
#include <set>
#include <sstream>
#include <stdio.h>
#include <string>
#include <thread>
#include <time.h>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
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

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
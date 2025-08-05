#include "TrickSaber/Utils/VectorPool.hpp"

namespace TrickSaber::Utils {

// Static member definitions
std::queue<UnityEngine::Vector3> VectorPool::vector3Pool;
std::queue<UnityEngine::Quaternion> VectorPool::quaternionPool;
std::mutex VectorPool::vector3Mutex;
std::mutex VectorPool::quaternionMutex;

} // namespace TrickSaber::Utils
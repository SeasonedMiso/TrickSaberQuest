#include "MockVectorPool.hpp"

namespace TrickSaber::Utils {

// Static member definitions
std::queue<MockVector3> MockVectorPool::vector3Pool;
std::queue<MockQuaternion> MockVectorPool::quaternionPool;
std::mutex MockVectorPool::vector3Mutex;
std::mutex MockVectorPool::quaternionMutex;

} // namespace TrickSaber::Utils
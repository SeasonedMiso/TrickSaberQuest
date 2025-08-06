#pragma once

#ifdef HOST_TESTS

// Host-native mock types for testing without Unity dependencies

namespace UnityEngine {
    struct Vector3 {
        float x, y, z;
        Vector3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
        static Vector3 get_zero() { return Vector3(0, 0, 0); }
    };

    struct Quaternion {
        float x, y, z, w;
        Quaternion(float x = 0, float y = 0, float z = 0, float w = 1) : x(x), y(y), z(z), w(w) {}
        static Quaternion get_identity() { return Quaternion(0, 0, 0, 1); }
    };
}

// Simple 2D vector for thumbstick input
struct Vector2 {
    float x, y;
    Vector2(float x = 0, float y = 0) : x(x), y(y) {}
};

#endif // HOST_TESTS
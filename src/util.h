#pragma once
#define TESTING
#include <iostream>
#include <cstdint>

struct RGBColor {
    unsigned char red = 0u;
    unsigned char green = 0u;
    unsigned char blue = 0u;
};

struct Vector3f {
    float x;
    float y;
    float z;
};

inline Vector3f make_vec3f(float x, float y, float z) {
    return { x, y, z };
}

inline Vector3f operator+(Vector3f a, const Vector3f& b) noexcept {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

inline Vector3f operator-(Vector3f a, const Vector3f& b) noexcept {
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

inline Vector3f& operator+=(Vector3f& a, const Vector3f& b) noexcept {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

inline Vector3f& operator-=(Vector3f& a, const Vector3f& b) noexcept {
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

inline Vector3f operator*(Vector3f v, float s) noexcept {
    v.x *= s;
    v.y *= s;
    v.z *= s;
    return v;
}

inline Vector3f operator*(float s, Vector3f v) noexcept {
    v.x *= s;
    v.y *= s;
    v.z *= s;
    return v;
}

inline Vector3f& operator*=(Vector3f& v, float s) noexcept {
    v.x *= s;
    v.y *= s;
    v.z *= s;
    return v;
}

inline Vector3f operator/(Vector3f v, float s) noexcept {
    float inv = 1.0f / s;
    v.x *= inv;
    v.y *= inv;
    v.z *= inv;
    return v;
}

inline Vector3f& operator/=(Vector3f& v, float s) noexcept {
    float inv = 1.0f / s;
    v.x *= inv;
    v.y *= inv;
    v.z *= inv;
    return v;
}

inline float operator*(Vector3f a, Vector3f b) noexcept {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline void swap(float& a, float& b) {
    float temp = a;
    a = b;
    b = temp;
}

inline float maxFinite(float a, float b, float c) {
    float result = -INFINITY;

    if (!std::isinf(a)) {
        result = fmax(result, a);
    }
    if (!std::isinf(b)) {
        result = fmax(result, b);
    }
    if (!std::isinf(c)) {
        result = fmax(result, c);
    }

    return result;
}

inline RGBColor operator*(RGBColor v, float s) noexcept {
    v.red *= s;
    v.green *= s;
    v.blue *= s;
    return v;
}
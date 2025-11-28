#pragma once
#include <iostream>

static const float PI_F = 3.14159265f;
static const float HALF_PI = 3.14159265f / 2.0;

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

inline void swap(float& a, float& b) {
    float temp = a;
    a = b;
    b = temp;
}
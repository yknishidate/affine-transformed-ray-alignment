#pragma once
#include <limits>

struct Hit
{
    Hit() = default;

    Hit(float t, unsigned int primID = 0xFFFFFFFF, unsigned int geomID = 0xFFFFFFFF, unsigned int instID = 0xFFFFFFFF, float u = 0.0f, float v = 0.0f)
        : t(t), primID(primID), geomID(geomID), instID(instID), u(u), v(v)
    {
    }

    friend bool operator < (const Hit& a, const Hit& b)
    {
        if (a.t == b.t) {
            if (a.instID == b.instID) {
                if (a.geomID == b.geomID) return a.primID < b.primID;
                else                      return a.geomID < b.geomID;
            }
            return a.instID < b.instID;
        }
        return a.t < b.t;
    }

    friend bool operator == (const Hit& a, const Hit& b)
    {
        return a.t == b.t && a.primID == b.primID && a.geomID == b.geomID && a.instID == b.instID;
    }

    friend bool operator <= (const Hit& a, const Hit& b)
    {
        if (a == b) return true;
        return a < b;
    }

    friend bool operator != (const Hit& a, const Hit& b)
    {
        return !(a == b);
    }

    float t = std::numeric_limits<float>::infinity();
    unsigned int primID = 0xFFFFFFFF;
    unsigned int geomID = 0xFFFFFFFF;
    unsigned int instID = 0xFFFFFFFF;
    float u;
    float v;
};

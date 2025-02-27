#pragma once

#include "Buffer.hpp"
#include "Vector.hpp"
#include "Color.hpp"

class Sampler
{
    public:
        template<typename T>
        static Color sample(const T* buffer, const Vector4f& t0, const Vector4f& t1, 
                               const Vector4f& t2, float w0, float w1, float w2)
        {
            float u = t0.x * w0 + t1.x * w1 + t2.x * w2;
            float v = t0.y * w0 + t1.y * w1 + t2.y * w2;
            u = std::min(u, 1.f);
            v = std::min(v, 1.f);
            int width = u * buffer->width - 1;
            int height = v * buffer->height - 1;
            uint32_t result = buffer->get(width, height);
            return Color((uint8_t)(result >> 24), (uint8_t)(result >> 16), (uint8_t)(result >> 8));
        }

    private:
};
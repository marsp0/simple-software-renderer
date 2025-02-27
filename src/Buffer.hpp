#pragma once

#include <stdint.h>
#include <vector>
#include <assert.h>

#include <iostream>

template<typename T>
class Buffer {
    
    public:

        Buffer(int width, int height, T initialValue = (T)0): 
               width(width), height(height), origin(width * height - width),
               initialValue(initialValue)
        {
            this->buffer.resize(this->width * this->height);
            this->clear();
        }
        ~Buffer() 
        {

        }

        void set(int x, int y, T value) 
        {
            this->buffer[origin - y * this->width + x] = value;
        }

        T get(int x, int y) const
        {
            assert(x < this->width && y < this->height);
            return this->buffer[origin - y * this->width + x];
        }

        int size() const
        {
            return this->width * this->height;
        }

        T* data() 
        {
            return this->buffer.data();
        }

        void clear()
        {
            for (int i = 0; i < this->width * this->height; i++)
            {
                this->buffer[i] = this->initialValue;
            }
        }

        // Data
        int width;
        int height;
        int origin;
        T   initialValue;
        std::vector<T> buffer;
    
    private:

};

typedef Buffer<float>    DepthBuffer;
typedef Buffer<uint32_t> FrameBuffer;
typedef Buffer<uint32_t> TextureBuffer;
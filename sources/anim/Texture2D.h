#pragma once

#include <memory>
#include <stdint.h>
#include <string>

typedef std::shared_ptr<struct Texture2D> Texture2DPtr;

struct Texture2D
{
    Texture2D(uint32_t id, size_t width, size_t height) :
        _id(id), _width(width), _height(height) {
    }

    uint32_t _id = 0;
    size_t _width = 0;
    size_t _height = 0;
};

Texture2DPtr createTexture(int32_t width, int32_t height, void* data);
Texture2DPtr createTexture(const std::string& path, bool vflip = true);
void bindTexture(const Texture2DPtr& texture, uint8_t slot);

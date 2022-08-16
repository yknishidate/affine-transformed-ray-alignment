#pragma once
#include <vector>
#include "tinyshow.hpp"

struct Image
{
    struct Color
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    struct Position
    {
        int32_t x;
        int32_t y;

        Position operator+ (const Position& other) const
        {
            return Position{ x + other.x, y + other.y };
        }
    };

    Image(int width, int height) : width{ width }, height{ height }
    {
        image.resize(width * height);
        clear();
    }

    Color& operator [](Position pos)
    {
        return image[pos.y * width + pos.x];
    }

    void clear(Color color = { 0, 0, 0 })
    {
        std::ranges::fill(image, color);
    }

    void show() const
    {
        tinyshow::Show<uint8_t>(image.data(), width, height, 3);
        tinyshow::WaitKey();
    }

    std::vector<Color> image;
    int width;
    int height;
};

#pragma once
#include "scene.hpp"
#include "image.hpp"

inline std::vector<Image::Position> getOffset(int32_t width, int32_t NumDivs)
{
    std::vector<Image::Position> positions;
    positions.reserve(NumDivs);

    for (int32_t group = 0; group < NumDivs; group++) {
        positions.push_back(Image::Position{ width * group / NumDivs, 0 });
    }
    return positions;
}

namespace Rendering
{
    template<int NumDivs>
    void render(const Scene& scene, Image& img)
    {
        assert((img.width % NumDivs) == 0);

        std::vector offset = getOffset(img.width, NumDivs);

        for (int32_t y = 0; y < img.height; y++) {
            for (int32_t x = 0; x < img.width / NumDivs; x++) {
                glm::vec3 origin{ 0.0, 0.0, scene.cameraDist };
                glm::vec3 direction{ (x / float(img.width) - 0.5), -(y / float(img.height) - 0.5), -1.0 };
                std::array<Hit, NumDivs>&& hits = scene.intersect<NumDivs>(origin, direction);
                for (int groupID = 0; groupID < NumDivs; groupID++) {
                    if (hits[groupID].geomID == RTC_INVALID_GEOMETRY_ID) continue;

                    int32_t renderX = x + offset[groupID].x;
                    int32_t renderY = y + offset[groupID].y;
                    img[{renderX, renderY}].r = hits[groupID].u * 255;
                    img[{renderX, renderY}].g = 0;
                    img[{renderX, renderY}].b = hits[groupID].geomID * 32;
                }
            }
        }
    }
};

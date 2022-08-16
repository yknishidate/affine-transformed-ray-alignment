#include <filesystem>
#include "time.hpp"
#include "image.hpp"
#include "scene.hpp"
#include "rendering.hpp"

constexpr int NUM_ITERATION = 5;

template <int NumDivs>
void execute(Scene scene, Image& img)
{
    double buildTime = Time::measure([&]()
    {
        for (int i = 0; i < NUM_ITERATION; i++) 
            scene.build<NumDivs>();
    });

    double renderTime = Time::measure([&]()
    {
        for (int i = 0; i < NUM_ITERATION; i++) 
            Rendering::render<NumDivs>(scene, img);
    });

    std::cout << buildTime / NUM_ITERATION << ","
        << renderTime / NUM_ITERATION << std::endl;
}

struct SceneData
{
    std::string name;
    std::string filepath;
    glm::vec3 position{ 0.0f };
    glm::vec3 scale{ 1.0f };
    glm::vec3 rotation{ 0.0f };
};

int main()
{
    std::vector<SceneData> datas{
        {"Mitsuba", "../asset/mitsuba/mitsuba.obj", { 0, -0.9, 0 }, { .5, .5, .5 }, { 0.0, 0.0, 0.0 }},
    };

    for (const auto& data : datas) {
        Image img{ 1024, 1024 };

        Scene scene{};
        scene.init(data.filepath, data.position, data.scale, data.rotation);

        std::cout << data.name << std::endl;
        std::cout << "NumDivs,BuildTime,RenderTime" << std::endl;
        std::cout << "1,"; execute<1>(scene, img);
        std::cout << "2,"; execute<2>(scene, img);
        std::cout << "4,"; execute<4>(scene, img);
        std::cout << "8,"; execute<8>(scene, img);
        std::cout << "16,"; execute<16>(scene, img);
        std::cout << "32,"; execute<32>(scene, img);
        std::cout << "64,"; execute<64>(scene, img);
        std::cout << "128,"; execute<128>(scene, img);
        std::cout << "256,"; execute<256>(scene, img);

        img.show();
        img.clear();
    }
}

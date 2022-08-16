#pragma once
#include <list>
#include <array>
#include <string>
#include <vector>
#include <iostream>

#include <embree3/rtcore.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "hit.hpp"

int gNumGroups = 0;
int gNumMeshs = 0;

struct Primitive
{
    uint32_t groupID;
};

struct Triangle
{
    glm::vec3 v0, v1, v2;
};

template <int Size>
struct IntersectContext
{
    IntersectContext(std::array<Hit, Size>& hits)
        : hits{ hits }
    {
    }

    RTCIntersectContext context;
    std::array<Hit, Size>& hits;
};

inline glm::vec3 projection(float cameraDist, const glm::vec3& v)
{
    float x = v.x / (cameraDist - v.z);
    float y = v.y / (cameraDist - v.z);
    float z = cameraDist - v.z;
    return { x, y, z };
}

template <int NumDivs>
bool isInner(float cameraDist, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
{
    const glm::vec3 proj0 = projection(cameraDist, v0);
    const glm::vec3 proj1 = projection(cameraDist, v1);
    const glm::vec3 proj2 = projection(cameraDist, v2);
    const float minX = std::min({ proj0.x, proj1.x, proj2.x });
    const float maxX = std::max({ proj0.x, proj1.x, proj2.x });
    const float minY = std::min({ proj0.y, proj1.y, proj2.y });
    const float maxY = std::max({ proj0.y, proj1.y, proj2.y });
    const float maxZ = std::max({ proj0.z, proj1.z, proj2.z });

    const float frustMinX = -0.5;
    const float frustMaxX = -0.5 + 1.0 / NumDivs;
    const float frustMinY = 0.5 - 1.0;
    const float frustMaxY = 0.5;

    if (maxZ < 0.0f) {
        return false;
    }
    if (minX > frustMaxX || frustMinX > maxX) {
        return false;
    }
    if (minY > frustMaxY || frustMinY > maxY) {
        return false;
    }
    return true;
}

template<int NumDivs>
glm::mat4 getAffineTransform(float cameraDist, int w)
{
    glm::mat4 mat{ 1 };
    mat[2][0] = 1.0f / NumDivs * w;
    mat[3][0] = -cameraDist * (1.0f / NumDivs * w);
    mat[2][1] = 0.0;
    mat[3][1] = 0.0;
    return mat;
}

template <int size>
void storeClosestHitPerGroup(const RTCFilterFunctionNArguments* args)
{
    assert(*args->valid == -1);
    auto context = reinterpret_cast<IntersectContext<size>*>(args->context);
    const auto ray = reinterpret_cast<RTCRay*>(args->ray);
    const auto hit = reinterpret_cast<RTCHit*>(args->hit);
    assert(args->N == 1);
    args->valid[0] = 0;

    const uint32_t groupID = hit->instID[0] / gNumMeshs;
    if (context->hits[groupID].t > ray->tfar) {
        context->hits[groupID].t = ray->tfar;
        context->hits[groupID].primID = hit->primID;
        context->hits[groupID].geomID = hit->geomID;
        context->hits[groupID].instID = hit->instID[0];
        context->hits[groupID].u = hit->u;
        context->hits[groupID].v = hit->v;
    }
}

struct Scene
{
    Scene() = default;

    ~Scene()
    {
        rtcReleaseScene(scene);
        rtcReleaseDevice(device);
    }

    void init(const std::string& filepath, glm::vec3 position = { 0, 0, 0 }, glm::vec3 scale = { 1, 1, 1 }, glm::vec3 rotation = { 0, 0, 0 })
    {
        rtcReleaseDevice(device);
        device = rtcNewDevice(nullptr);

        loadModel(filepath);
        this->position = position;
        this->scale = scale;
        this->rotation = rotation;
    }

    void loadModel(const std::string& filepath)
    {
        tinyobj::ObjReaderConfig reader_config;
        reader_config.triangulate = true;

        tinyobj::ObjReader reader;
        if (!reader.ParseFromFile(filepath, reader_config)) {
            if (!reader.Error().empty()) {
                std::cout << "Error: " << reader.Error() << std::endl;
            }
            return;
        }

        const auto& attrib = reader.GetAttrib();
        const auto& shapes = reader.GetShapes();

        // loop over shapes
        meshes.resize(shapes.size());

        for (size_t s = 0; s < shapes.size(); ++s) {
            rtcReleaseScene(meshes[s]);
            meshes[s] = rtcNewScene(device);

            verticesList.emplace_back();
            indicesList.emplace_back();
            auto& vertices = verticesList.back();
            auto& indices = indicesList.back();

            size_t index_offset = 0;
            // loop over faces
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
                const size_t fv = shapes[s].mesh.num_face_vertices[f];

                // loop over vertices
                for (size_t v = 0; v < fv; ++v) {
                    const tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    const tinyobj::real_t vx = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 0];
                    const tinyobj::real_t vy = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 1];
                    const tinyobj::real_t vz = attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 2];
                    vertices.push_back(glm::vec3(vx, vy, vz));
                }

                for (int i = 0; i < 3; ++i) {
                    indices.push_back(indices.size());
                }

                index_offset += fv;
            }

            RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
            rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0,
                                       RTC_FORMAT_FLOAT3, vertices.data(), 0, sizeof(glm::vec3), vertices.size());
            rtcSetSharedGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
                                       indices.data(), 0, 3 * sizeof(uint32_t), indices.size() / 3);
            rtcCommitGeometry(geom);
            rtcAttachGeometry(meshes[s], geom);
            rtcReleaseGeometry(geom);

            rtcCommitScene(meshes[s]);
        }
    }

    template <int NumDivs>
    void build()
    {
        gNumGroups = NumDivs;
        gNumMeshs = meshes.size();

        rtcReleaseScene(scene);
        scene = rtcNewScene(device);

        // Instance
        const glm::mat4 transMat = glm::translate(glm::mat4{ 1 }, position);
        const glm::mat4 scaleMat = glm::scale(glm::mat4{ 1 }, scale);
        const glm::mat4 rotateXMat = glm::rotate(glm::mat4{ 1 }, glm::radians(rotation.x), glm::vec3(1, 0, 0));
        const glm::mat4 rotateYMat = glm::rotate(glm::mat4{ 1 }, glm::radians(rotation.y), glm::vec3(0, 1, 0));
        const glm::mat4 rotateZMat = glm::rotate(glm::mat4{ 1 }, glm::radians(rotation.z), glm::vec3(0, 0, 1));
        const glm::mat4 modelMatrix = transMat * scaleMat * rotateXMat * rotateYMat * rotateZMat;

        for (int x = 0; x < NumDivs; x++)
        {
            const glm::mat4 alignMatrix = getAffineTransform<NumDivs>(cameraDist, x);
            const glm::mat4 matrix = alignMatrix * modelMatrix;

            for (auto mesh: meshes)
            {
                RTCGeometry instance = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_INSTANCE);
                rtcSetGeometryInstancedScene(instance, mesh);
                rtcSetGeometryTransform(instance, 0, RTC_FORMAT_FLOAT4X4_COLUMN_MAJOR, &matrix);
                rtcAttachGeometry(scene, instance);
                rtcReleaseGeometry(instance);
                rtcCommitGeometry(instance);
            }
        }
        rtcCommitScene(scene);
    }

    Hit intersect(const glm::vec3& origin, const glm::vec3& direction) const
    {
        RTCRayHit rayhit;
        rayhit.ray.org_x = origin.x;
        rayhit.ray.org_y = origin.y;
        rayhit.ray.org_z = origin.z;
        rayhit.ray.dir_x = direction.x;
        rayhit.ray.dir_y = direction.y;
        rayhit.ray.dir_z = direction.z;
        rayhit.ray.tnear = 0.0f;
        rayhit.ray.tfar = INFINITY;
        rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;

        RTCIntersectContext context;
        rtcInitIntersectContext(&context);

        rtcIntersect1(scene, &context, &rayhit);

        return Hit{ rayhit.ray.tfar, rayhit.hit.primID, rayhit.hit.geomID, rayhit.hit.instID[0],
                    rayhit.hit.u, rayhit.hit.v };
    }

    template <int Size>
    std::array<Hit, Size> intersect(const glm::vec3& origin, const glm::vec3& direction) const
    {
        RTCRayHit rayhit;
        rayhit.ray.org_x = origin.x;
        rayhit.ray.org_y = origin.y;
        rayhit.ray.org_z = origin.z;
        rayhit.ray.dir_x = direction.x;
        rayhit.ray.dir_y = direction.y;
        rayhit.ray.dir_z = direction.z;
        rayhit.ray.tnear = 0.0f;
        rayhit.ray.tfar = INFINITY;
        rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;

        std::array<Hit, Size> hits{};

        IntersectContext<Size> context{ hits };
        rtcInitIntersectContext(&context.context);
        context.context.filter = storeClosestHitPerGroup<Size>;

        rtcIntersect1(scene, &context.context, &rayhit);
        return hits;
    }

    std::list<std::vector<glm::vec3>> verticesList;
    std::list<std::vector<uint32_t>> indicesList;

    RTCDevice device;
    std::vector<RTCScene> meshes;
    RTCScene scene;

    float cameraDist = 5.0;
    glm::vec3 position = { 0, 0, 0 };
    glm::vec3 scale = { 1, 1, 1 };
    glm::vec3 rotation = { 0, 0, 0 };
};

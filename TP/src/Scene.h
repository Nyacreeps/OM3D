#ifndef SCENE_H
#define SCENE_H

#include <SceneObject.h>
#include <PointLight.h>
#include <Camera.h>

#include <glad/glad.h>
#include <vector>
#include <memory>
#include <TypedBuffer.h>
#include "Vertex.h"

namespace OM3D {

Result<std::shared_ptr<StaticMesh>> meshFromGltf(const std::string& file_name);

class Scene : NonMovable {

    public:
        Scene();

        static Result<std::unique_ptr<Scene>> from_gltf(const std::string& file_name);

        void renderShading(const Camera &camera, std::shared_ptr<Program> programp) const;
        void renderShadingSpheres(const Camera &camera, std::shared_ptr<Program> programp) const;
        void renderShadingDirectional(const Camera &camera, std::shared_ptr<Program> programp) const;
        void render(const Camera& camera) const;
        void renderOcclusion(const Camera& camera, bool debug);

        void add_object(SceneObject obj);
        void add_object(PointLight obj);
        void sortObjects(const Camera &camera);
        
        std::vector<SceneObject> _objects;
        void deleteQueries() {
            for (auto& obj : _objects) {
                glDeleteQueries(1, &obj._queryId);
            }
        }

    private:
        std::vector<PointLight> _point_lights;
        TypedBuffer<Instance> _instanceBuffer;
        glm::vec3 _sun_direction = glm::vec3(0.2f, 1.0f, 0.1f);
};

}

#endif // SCENE_H

#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#include <Scene.h>
#include <Camera.h>

namespace OM3D {

class SceneView {
    public:
        SceneView(Scene* scene = nullptr);

        Camera& camera();
        const Camera& camera() const;
        
        void renderShading(std::shared_ptr<Program> programp) const;
        void renderShadingSpheres(std::shared_ptr<Program> programp) const;
        void renderShadingDirectional(std::shared_ptr<Program> programp) const;
        void renderTAA(std::shared_ptr<Program> programp) const;
        void render() const;
        void renderOcclusion(bool debug);

    private:
        Scene* _scene = nullptr;
        Camera _camera;

};

}

#endif // SCENEVIEW_H

#include "SceneView.h"

namespace OM3D {

SceneView::SceneView(const Scene* scene) : _scene(scene) {
}

Camera& SceneView::camera() {
    return _camera;
}

const Camera& SceneView::camera() const {
    return _camera;
}

void SceneView::renderShading(std::shared_ptr<Program> programp) const {
    if(_scene) {
        _scene->renderShading(_camera, programp);
    }
}

void SceneView::renderShadingSpheres(std::shared_ptr<Program> programp) const {
    if(_scene) {
        _scene->renderShadingSpheres(_camera, programp);
    }
}

void SceneView::render() const {
    if(_scene) {
        _scene->render(_camera);
    }
}

}

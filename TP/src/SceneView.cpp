#include "SceneView.h"

namespace OM3D {

SceneView::SceneView(Scene* scene) : _scene(scene) {
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

void SceneView::renderShadingDirectional(std::shared_ptr<Program> programp) const {
    if(_scene) {
        _scene->renderShadingDirectional(_camera, programp);
    }
}

void SceneView::renderTAA(std::shared_ptr<Program> programp) const {
    if(_scene) {
        _scene->renderTAA(_camera, programp);
    }
}

void SceneView::render() const {
    if(_scene) {
        _scene->render(_camera);
    }
}

void SceneView::renderOcclusion(bool debug) {
    if(_scene) {
        _scene->renderOcclusion(_camera, debug);
    }
}

}

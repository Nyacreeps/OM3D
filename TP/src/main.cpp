#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

#include <graphics.h>
#include <SceneView.h>
#include <Texture.h>
#include <Framebuffer.h>
#include <ImGuiRenderer.h>

#include <imgui/imgui.h>

using namespace OM3D;

static float delta_time = 0.0f;
const glm::uvec2 window_size(800, 500);

void glfw_check(bool cond) {
    if (!cond) {
        const char* err = nullptr;
        glfwGetError(&err);
        std::cerr << "GLFW error: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void update_delta_time() {
    static double time = 0.0;
    const double new_time = program_time();
    delta_time = float(new_time - time);
    time = new_time;
}

void process_inputs(GLFWwindow* window, Camera& camera) {
    static glm::dvec2 mouse_pos;

    glm::dvec2 new_mouse_pos;
    glfwGetCursorPos(window, &new_mouse_pos.x, &new_mouse_pos.y);

    {
        glm::vec3 movement = {};
        if (glfwGetKey(window, 'W') == GLFW_PRESS) {
            movement += camera.forward();
        }
        if (glfwGetKey(window, 'S') == GLFW_PRESS) {
            movement -= camera.forward();
        }
        if (glfwGetKey(window, 'D') == GLFW_PRESS) {
            movement += camera.right();
        }
        if (glfwGetKey(window, 'A') == GLFW_PRESS) {
            movement -= camera.right();
        }
        if (glfwGetKey(window, ' ') == GLFW_PRESS) {
            movement += camera.up();
        }
        if (glfwGetKey(window, 'C') == GLFW_PRESS) {
            movement -= camera.up();
        }

        float speed = 10.0f;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            speed *= 10.0f;
        }

        if (movement.length() > 0.0f) {
            const glm::vec3 new_pos = camera.position() + movement * delta_time * speed;
            camera.set_view(glm::lookAt(new_pos, new_pos + camera.forward(), camera.up()));
        }
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        const glm::vec2 delta = glm::vec2(mouse_pos - new_mouse_pos) * 0.01f;
        if (delta.length() > 0.0f) {
            glm::mat4 rot = glm::rotate(glm::mat4(1.0f), delta.x, glm::vec3(0.0f, 1.0f, 0.0f));
            rot = glm::rotate(rot, delta.y, camera.right());
            camera.set_view(glm::lookAt(camera.position(),
                                        camera.position() + (glm::mat3(rot) * camera.forward()),
                                        (glm::mat3(rot) * camera.up())));
        }
    }

    mouse_pos = new_mouse_pos;
}

std::unique_ptr<Scene> create_default_scene() {
    auto scene = std::make_unique<Scene>();

    // Load default cube model
    auto result = Scene::from_gltf(std::string(data_path) + "cube.glb");
    ALWAYS_ASSERT(result.is_ok, "Unable to load default scene");
    scene = std::move(result.value);
    auto& object = scene->_objects[0];

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            for (int k = -1; k <= 1; k++) {
                if (i == 0 && j == 0 && k == 0) continue;
                auto obj1 = SceneObject(std::make_shared<StaticMesh>(StaticMesh::CubeMesh()),
                                        Material::empty_material());
                obj1.set_transform(
                    glm::translate(object.transform(), {i * 4.0f, j * 4.0f, k * 4.0f}));
                scene->add_object(std::move(obj1));
            }
        }
    }

    auto obj1 = SceneObject(std::make_shared<StaticMesh>(StaticMesh::CubeMesh()),
                            Material::empty_material());
    obj1.set_transform(glm::translate(glm::scale(object.transform(), glm::vec3(4.0f, 4.0f, 4.0f)),
                                      {7.0f, 0.0f, 0.0f}));
    scene->add_object(std::move(obj1));

    // Add lights
    {
        PointLight light;
        light.set_position(glm::vec3(1.0f, 100.0f, 4.0f));
        light.set_color(glm::vec3(1.0f, 1.0f, 1.0f));
        light.set_radius(1000.0f);
        scene->add_object(std::move(light));
    }
    {
        PointLight light;
        light.set_position(glm::vec3(1.0f, 2.0f, -4.0f));
        light.set_color(glm::vec3(1.0f, 1.0f, 1.0f));
        light.set_radius(50.0f);
        scene->add_object(std::move(light));
    }

    return scene;
}

int main(int, char**) {
    DEBUG_ASSERT([] {
        std::cout << "Debug asserts enabled" << std::endl;
        return true;
    }());

    glfw_check(glfwInit());
    DEFER(glfwTerminate());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window =
        glfwCreateWindow(window_size.x, window_size.y, "TP window", nullptr, nullptr);
    glfw_check(window);
    DEFER(glfwDestroyWindow(window));

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    init_graphics();

    ImGuiRenderer imgui(window);

    std::unique_ptr<Scene> scene = create_default_scene();
    SceneView scene_view(scene.get());

    auto tonemap_program = Program::from_file("tonemap.comp");

    Texture depth(window_size, ImageFormat::Depth32_FLOAT);
    Texture albedo(window_size, ImageFormat::RGBA8_sRGB);
    Texture normals(window_size, ImageFormat::RGBA8_UNORM);
    Texture lit(window_size, ImageFormat::RGBA16_FLOAT);
    Texture color(window_size, ImageFormat::RGBA8_UNORM);
    Framebuffer gBuffer(&depth, std::array{&albedo, &normals});
    Framebuffer mainFrameBuffer(&depth, std::array{&lit});
    Framebuffer tonemap_framebuffer(nullptr, std::array{&color});
    auto gdebug_program1 = Program::from_files("gdebug1.frag", "screen.vert");
    auto gdebug_program2 = Program::from_files("gdebug2.frag", "screen.vert");
    auto shading_program = Program::from_files("shading.frag", "screen.vert");
    auto shadingspheres_program =
        Program::from_files("shading_spheres.frag", "shading_spheres.vert");
    auto shadingdirectional_program =
        Program::from_files("shading_directional.frag", "screen.vert");
    auto occlusionrend_program = Program::from_files("prepass.frag", "basic.vert");

    int gDebugMode = 0;
    int occDebugMode = 0;
    int gBufferRenderMode = 0;
    bool renderSpheres = false;

    for (;;) {
        glfwPollEvents();
        if (glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            break;
        }

        update_delta_time();

        if (const auto& io = ImGui::GetIO(); !io.WantCaptureMouse && !io.WantCaptureKeyboard) {
            process_inputs(window, scene_view.camera());
        }

        // Render the scene to the gbuffer
        if (gBufferRenderMode == 0) {
            gBuffer.bind();
            scene_view.render();
        } else {
            gBuffer.bind();
            scene->sortObjects(scene_view.camera());
            scene_view.renderOcclusion(occDebugMode);
        }

        if (gDebugMode == 1) {
            gdebug_program1->bind();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            albedo.bind(0);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        } else if (gDebugMode == 2) {
            gdebug_program1->bind();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            normals.bind(0);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        } else if (gDebugMode == 3) {
            gdebug_program2->bind();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            depth.bind(0);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        } else {
            mainFrameBuffer.bind(true, false);
            albedo.bind(0);
            normals.bind(1);
            depth.bind(2);
            if (renderSpheres) {
                scene_view.renderShadingDirectional(shadingdirectional_program);
                scene_view.renderShadingSpheres(shadingspheres_program);
            } else {
                scene_view.renderShading(shading_program);
            }
            // Apply a tonemap in compute shader
            {
                tonemap_program->bind();
                lit.bind(0);
                color.bind_as_image(1, AccessType::WriteOnly);
                glDispatchCompute(align_up_to(window_size.x, 8), align_up_to(window_size.y, 8), 1);
            }
            // Blit tonemap result to screen
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            tonemap_framebuffer.blit();
        }

        glDisable(GL_CULL_FACE); // ensure GUI does not cull
        // GUI
        imgui.start();
        {
            char buffer[1024] = {};
            if (ImGui::InputText("Load scene", buffer, sizeof(buffer),
                                 ImGuiInputTextFlags_EnterReturnsTrue)) {
                auto result = Scene::from_gltf(buffer);
                if (!result.is_ok) {
                    std::cerr << "Unable to load scene (" << buffer << ")" << std::endl;
                } else {
                    scene = std::move(result.value);
                    scene_view = SceneView(scene.get());
                }
            }
            ImGui::Text("Prepass");
            ImGui::RadioButton("Classic prepass", &gBufferRenderMode, 0);
            ImGui::RadioButton("Occlusion culling prepass", &gBufferRenderMode, 1);
            ImGui::Text("Display mode");
            ImGui::RadioButton("Normal display", &gDebugMode, 0);
            ImGui::RadioButton("Display Gbuffer albedo", &gDebugMode, 1);
            ImGui::RadioButton("Display Gbuffer normals", &gDebugMode, 2);
            ImGui::RadioButton("Display Gbuffer depth", &gDebugMode, 3);
            ImGui::Checkbox("Switch to volume based deferred shading", &renderSpheres);
            ImGui::Text("Occlusion");
            ImGui::RadioButton("Normal occlusion", &occDebugMode, 0);
            ImGui::RadioButton("Display occludees in red", &occDebugMode, 1);
        }
        imgui.finish();

        glfwSwapBuffers(window);
    }

    scene = nullptr; // destroy scene and child OpenGL objects
}

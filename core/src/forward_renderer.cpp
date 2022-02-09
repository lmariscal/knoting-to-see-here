#include <bgfx/bgfx.h>
#include <knoting/forward_renderer.h>
#include <knoting/mesh.h>
#include <knoting/texture.h>

#include <knoting/components.h>
#include <knoting/engine.h>
#include <knoting/scene.h>
#include <stb_image.h>
#include <fstream>
#include <string_view>

namespace knot {

ForwardRenderer::~ForwardRenderer() {}

ForwardRenderer::ForwardRenderer(Engine& engine) : m_engine(engine) {}

void ForwardRenderer::render_pbr() {
    using namespace components;
    clear_framebuffer();
    bgfx::touch(0);

    auto sceneOpt = Scene::get_active_scene();
    if (!sceneOpt) {
        return;
    }
    Scene& scene = sceneOpt.value();
    entt::registry& registry = scene.get_registry();

    //=CAMERA===========================
    auto cameras = registry.view<Transform, EditorCamera, Name>();

    for (auto& cam : cameras) {
        auto goOpt = scene.get_game_object_from_handle(cam);
        if (!goOpt) {
            continue;
        }

        GameObject go = goOpt.value();
        Transform& transform = go.get_component<Transform>();
        EditorCamera& editorCamera = go.get_component<EditorCamera>();
        Name& name = go.get_component<Name>();

        const glm::vec3 pos = transform.get_position();
        const glm::vec3 lookTarget = editorCamera.get_look_target();
        const glm::vec3 up = editorCamera.get_up();

        const float fovY = editorCamera.get_fov();
        const float aspectRatio = float((float)get_window_width() / (float)get_window_height());
        const float zNear = editorCamera.get_z_near();
        const float zFar = editorCamera.get_z_far();

        // Set view and projection matrix for view 0.
        {
            glm::mat4 view;
            view = glm::lookAt(pos, lookTarget, up);
            glm::mat4 proj = glm::perspective(fovY, aspectRatio, zNear, zFar);

            bgfx::setViewTransform(0, &view[0][0], &proj[0][0]);
        }
    }

    //=PBR PIPELINE===========================

    auto entities = registry.view<Transform, Mesh, Material, Name>();
    for (auto& e : entities) {
        auto goOpt = scene.get_game_object_from_handle(e);
        if (!goOpt) {
            continue;
        }

        GameObject go = goOpt.value();
        Transform& transform = go.get_component<Transform>();
        Mesh& mesh = go.get_component<Mesh>();
        Material& material = go.get_component<Material>();
        Name& name = go.get_component<Name>();

        bgfx::setTransform(value_ptr(transform.get_model_matrix()));

        // Set vertex and index buffer.
        bgfx::setVertexBuffer(0, mesh.get_vertex_buffer());
        bgfx::setIndexBuffer(mesh.get_index_buffer());

        // Bind Uniforms & textures.
        material.set_uniforms();

        // TODO enable MSAA in bgfx
        bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS);

        bgfx::submit(0, material.get_program());
    }

    bgfx::frame();
}

void ForwardRenderer::on_render() {
    // clear_framebuffer();

    // bgfx::touch(0);

    //
    //    const glm::vec3 at = {0.0f, 0.0f, 0.0f};
    //    const glm::vec3 eye = {0.0f, 0.0f, -7.0f};
    //    const glm::vec3 up = {0.0f, 1.0f, 0.0f};
    //
    //    const float fovY = glm::radians(60.0f);
    //    const float aspectRatio = float((float)get_window_width() / (float)get_window_height());
    //    const float zNear = 0.1f;
    //    const float zFar = 100.0f;
    //
    //    // Set view and projection matrix for view 0.
    //    {
    //        glm::mat4 view;
    //        view = glm::lookAt(eye, at, up);
    //        glm::mat4 proj = glm::perspective(fovY, aspectRatio, zNear, zFar);
    //
    //        bgfx::setViewTransform(0, &view[0][0], &proj[0][0]);
    //    }
    //
    //    float lightPosRadius[4][4];
    //    for (uint32_t ii = 0; ii < m_numLights; ++ii) {
    //        lightPosRadius[ii][0] =
    //            glm::sin((m_timePassed * (0.1f + ii * 0.17f) + ii * glm::half_pi<float>() * 1.37f)) * 15.0f;
    //        lightPosRadius[ii][1] =
    //            glm::cos((m_timePassed * (0.2f + ii * 0.29f) + ii * glm::half_pi<float>() * 1.49f)) * 15.0f;
    //        lightPosRadius[ii][2] = 15.5f;
    //        lightPosRadius[ii][3] = 17.0f;
    //    }
    //
    //    bgfx::setUniform(u_lightPosRadius, lightPosRadius, m_numLights);
    //
    //    float lightRgbInnerR[4][4] = {
    //        {1.0f, 0.7f, 0.2f, 0.8f},
    //        {0.7f, 0.2f, 1.0f, 0.8f},
    //        {0.2f, 1.0f, 0.7f, 0.8f},
    //        {1.0f, 0.4f, 0.2f, 0.8f},
    //    };
    //
    //    bgfx::setUniform(u_lightRgbInnerR, lightRgbInnerR, m_numLights);
    //
    //    for (uint32_t yy = 0; yy < 11; ++yy) {
    //        for (uint32_t xx = 0; xx < 11; ++xx) {
    //            glm::mat4 mtx = glm::identity<glm::mat4>();
    //            mtx = glm::translate(mtx, glm::vec3(15.0f - float(xx) * 3.0f, -15.0f + float(yy) * 3.0f, 30.0f));
    //            mtx *= glm::yawPitchRoll(m_timePassed + xx * 0.21f, m_timePassed + yy * 0.37f, 0.0f);
    //            bgfx::setTransform(&mtx[0][0]);
    //
    //            // Set vertex and index buffer.
    //            bgfx::setVertexBuffer(0, m_cube.get_vertex_buffer());
    //            bgfx::setIndexBuffer(m_cube.get_index_buffer());
    //
    //            // Bind textures.
    //            bgfx::setTexture(0, s_texColor, m_textureColor);
    //            bgfx::setTexture(1, s_texNormal, m_textureNormal);
    //
    //            bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z |
    //                           BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_MSAA);
    //
    //            bgfx::submit(0, m_shaderProgram.get_program());
    //        }
    //    }
    //
    //    bgfx::frame();
}

void ForwardRenderer::on_post_render() {}

void ForwardRenderer::on_awake() {}

void ForwardRenderer::on_update(double m_delta_time) {
    m_timePassed += (float)m_delta_time;
}

void ForwardRenderer::on_late_update() {}

void ForwardRenderer::on_destroy() {}

void ForwardRenderer::recreate_framebuffer(uint16_t width, uint16_t height, uint16_t id) {
    bgfx::reset((uint32_t)width, (uint32_t)height, BGFX_RESET_VSYNC);
    bgfx::setViewClear(id, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, (uint32_t)m_clearColor);
    bgfx::setViewRect(id, 0, 0, width, height);
}

void ForwardRenderer::clear_framebuffer(uint16_t id) {
    bgfx::setViewClear(id, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, (uint32_t)m_clearColor);
}

int ForwardRenderer::get_window_width() {
    return m_engine.get_window_module().lock()->get_window_width();
}

int ForwardRenderer::get_window_height() {
    return m_engine.get_window_module().lock()->get_window_height();
}

}  // namespace knot

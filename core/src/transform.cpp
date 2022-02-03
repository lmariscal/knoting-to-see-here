#include <knoting/log.h>
#include <knoting/transform.h>

namespace knot {
namespace components {

Transform::Transform(const vec3& position, const vec3& scale, const quat& rotation)
    : m_position(position), m_scale(scale), m_rotation(rotation) {}

vec3 Transform::get_position() const {
    return m_position;
}

vec3 Transform::get_scale() const {
    return m_scale;
}

quat Transform::get_rotation() const {
    return m_rotation;
}

vec3 Transform::get_rotation_euler() const {
    return degrees(eulerAngles(m_rotation));
}

void Transform::set_position(const vec3& position) {
    m_position = position;
}

void Transform::set_scale(const vec3& scale) {
    m_scale = scale;
}

void Transform::set_rotation(const quat& rotation) {
    m_rotation = rotation;
}

void Transform::set_rotation_euler(const vec3& euler) {
    m_rotation = quat(radians(euler));
}

mat4 Transform::get_model_matrix() const {
    log::debug("TODO Transform::get_model_matrix");
    return mat4(1.0f);
}

}  // namespace components
}  // namespace knot

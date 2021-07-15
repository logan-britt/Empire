#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform uniform_buffer_object {
  mat2 transform;
} ubo;

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec3 frag_color;

void main() {
  vec2 position = ubo.transform*in_position;
  position = position - vec2(1.0, 1.0);

  gl_Position = vec4(position, 0.0, 1.0);
  frag_color = in_color;
}
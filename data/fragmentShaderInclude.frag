#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "getcolor.glsl"

layout(location = 0) in vec3 f_Position;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor   = vec4( getColor(), 1);
}

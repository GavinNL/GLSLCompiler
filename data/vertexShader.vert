#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3  in_Position;

layout(location = 0) out vec3 f_Position;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position  = vec4( in_Position, 1.0);
    f_Position   = in_Position;
}

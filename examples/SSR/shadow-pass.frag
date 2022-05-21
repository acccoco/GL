#version 330 core

out vec4 out_depth;

void main()
{
    /// 这里是正交投影，gl_FragCoord.z 就是线性深度，且范围是 [0, 1]
    out_depth = vec4(gl_FragCoord.z);
}
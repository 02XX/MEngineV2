#version 450

void main()
{
    //全屏三角形
  vec2 pos = vec2(
        float(gl_VertexIndex & 1) * 4.0 - 1.0,
        float((gl_VertexIndex >> 1) & 1) * 4.0 - 1.0
    );
    gl_Position = vec4(pos, 0.0, 1.0);
}
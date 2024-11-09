#version 450

layout (triangles) in;
layout (line_strip, max_vertices = 18) out;

layout (location = 0) in vec3 normals[];
layout (location = 1) in vec3 tangents[];
layout (location = 2) in vec3 bitangents[];

layout (location = 0) out vec3 outColor;

const float MAGNITUDE = 0.04;

void GenerateLine(int index, vec3 offsets[3], vec3 color)
{
	gl_Position = gl_in[index].gl_Position;
	outColor = color;
	EmitVertex();
	gl_Position = gl_in[index].gl_Position + vec4(offsets[index], 0.0) * MAGNITUDE;
	outColor = color;
	EmitVertex();
	EndPrimitive();
}

void main()
{
	for (int i = 0; i < 3; ++i)
	{
		GenerateLine(i, normals, vec3(1.0, 1.0, 0.0));
		GenerateLine(i, tangents, vec3(0.0, 0.0, 1.0));
		GenerateLine(i, bitangents, vec3(1.0, 0.0, 0.0));
	}
}

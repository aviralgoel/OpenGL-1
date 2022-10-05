#include "SandboxLayer.h"
#include <stb_image/stb_image.h>

using namespace GLCore;
using namespace GLCore::Utils;

struct Vec2
{
	float x, y;
};
struct Vec3
{
	float x, y, z;
};
struct Vec4 {
	float x, y, z, w;
};

struct Vertex {
	Vec3 Position;
	Vec4 Color;
	Vec2 TexCoords;
	float TexID;
};

SandboxLayer::SandboxLayer()
	: m_CameraController(16.0f / 9.0f)
{
}

SandboxLayer::~SandboxLayer()
{
}

static Vertex* CreateQuad(Vertex* target, float x, float y, float TextureID)
{
	float size = 1;
	
	target->Position = { x, y, 0.0f };
	target->Color = { 1.18f, 0.6f, 0.96f, 1.0f };
	target->TexCoords = { 0.0f, 0.0f, };
	target->TexID = TextureID;
	target++;
	
	target->Position = { x+ size,y, 0.0f };
	target->Color = { 1.18f, 0.6f, 0.96f, 1.0f };
	target->TexCoords = { 1.0f, 0.0f, };
	target->TexID = TextureID;
	target++;

	target->Position = { x+size, y+size, 0.0f };
	target->Color = { 1.18f, 0.6f, 0.96f, 1.0f };
	target->TexCoords = { 1.0f, 1.0f, };
	target->TexID = TextureID;
	target++;

	target->Position = { x,  y+size, 0.0f };
	target->Color = { 1.18f, 0.6f, 0.96f, 1.0f };
	target->TexCoords = { 0.0f, 1.0f, };
	target->TexID = TextureID;
	target++;

	return target;
}
static GLuint LoadTexture(const std::string& path)
{
	int w, h, bits;
	// OpenGL expects the 0.0 coordinate on the y-axis to be on the bottom side of the image, but images usually have 0.0 at the top of the y-axis.
	// flip the y-axis during image loading
	stbi_set_flip_vertically_on_load(1);

	auto* pixels = stbi_load(path.c_str(), &w, &h, &bits, STBI_rgb);
	GLuint textureID;
	glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	stbi_image_free(pixels);
	return textureID;
}
void SandboxLayer::OnAttach()
{
	// load two textures, each with different ID
	m_Texture1 = LoadTexture("assets/textures/usc1.png");
	m_Texture2 = LoadTexture("assets/textures/usc2.png");

	EnableGLDebugging();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_Shader = Shader::FromGLSLTextFiles(
		"assets/shaders/test.vert.glsl",
		"assets/shaders/test.frag.glsl"
	);

	glUseProgram(m_Shader->GetRendererID());
	auto loc = glGetUniformLocation(m_Shader->GetRendererID(), "u_Textures");
	int samplers[2] = { 0,1 };
	glUniform1iv(loc, 2, samplers);

	const size_t MaxQuadCount = 1000;
	const size_t MaxVertexCount = MaxQuadCount * 4;
	const size_t MaxIndexCount = MaxQuadCount * 6;

	glCreateVertexArrays(1, &m_QuadVA);
	glBindVertexArray(m_QuadVA);

	glCreateBuffers(1, &m_QuadVB);
	glBindBuffer(GL_ARRAY_BUFFER, m_QuadVB);
	// for allocation
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MaxQuadCount, nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Color));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexCoords));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexID));

	//uint32_t indices[] = { 
	//						0, 1, 2, 2, 3, 0, 
	//						4, 5, 6, 6, 7, 4
	//					};

	uint32_t indices[MaxIndexCount];
	uint32_t offset = 0;
	for (int i = 0; i < MaxIndexCount; i+=6)
	{
		indices[i + 0] = 0 + offset;
		indices[i + 1] = 1 + offset;
		indices[i + 2] = 2 + offset;

		indices[i + 3] = 2 + offset;
		indices[i + 4] = 3 + offset;
		indices[i + 5] = 0 + offset;
		offset += 4;

	}
	glCreateBuffers(1, &m_QuadIB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadIB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void SandboxLayer::OnDetach()
{
	glDeleteVertexArrays(1, &m_QuadVA);
	glDeleteBuffers(1, &m_QuadVB);
	glDeleteBuffers(1, &m_QuadIB);
}

void SandboxLayer::OnEvent(Event& event)
{
	m_CameraController.OnEvent(event);

	EventDispatcher dispatcher(event);
}

void SandboxLayer::OnUpdate(Timestep ts)
{
	uint32_t indexCounter = 0;
	std::array<Vertex, 1000> vertices;
	Vertex* buffer = vertices.data();
	for (int i = 0; i < 15; i+=1)
	{
		for (int j = 0; j < 15; j +=1)
		{
			buffer = CreateQuad(buffer, i+m_QuadPosition[0], j+m_QuadPosition[1], (i + j) % 2);
			indexCounter+=6;
		}
	}
	
	m_CameraController.OnUpdate(ts);
	// Set dynamic vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_QuadVB);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_Shader->GetRendererID());
	glBindTextureUnit(0, m_Texture1);
	glBindTextureUnit(1, m_Texture2);
	int location = glGetUniformLocation(m_Shader->GetRendererID(), "u_ViewProjection");
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m_CameraController.GetCamera().GetViewProjectionMatrix()));

	//location = glGetUniformLocation(m_Shader->GetRendererID(), "u_Color");
	//glUniform4fv(location, 1, glm::value_ptr(m_SquareColor));

	glBindVertexArray(m_QuadVA);
	glDrawElements(GL_TRIANGLES, indexCounter, GL_UNSIGNED_INT, nullptr);
}

void SandboxLayer::OnImGuiRender()
{
	ImGui::Begin("Controls");
	ImGui::DragFloat2("Quad Position", m_QuadPosition, 0.1f);
	//ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}
#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

class SandboxLayer : public GLCore::Layer
{
public:
	SandboxLayer();
	virtual ~SandboxLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(GLCore::Event& event) override;
	virtual void OnUpdate(GLCore::Timestep ts) override;
	virtual void OnImGuiRender() override;
private:
	GLCore::Utils::Shader* m_Shader;
	GLCore::Utils::OrthographicCameraController m_CameraController;
	
	GLuint m_QuadVA, m_QuadVB, m_QuadIB;

	glm::vec4 m_SquareColor = { 0.8f, 0.2f, 0.3f, 1.0f };
	GLuint m_Texture1, m_Texture2;

	float m_QuadPosition[2] = { -1.5, -0.5f };
};
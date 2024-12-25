#include "Common/stdafx.h"

#include "UI/UI.h"
#include "VK/Application/Application.h"
#include <ImGui/imgui.h>

#include "VK/Base/FrameworkWindows.h"

void Application::BuildUI()
{
	// if we haven't initialized GLTF loader yet, dont draw the UI.
	if (m_pGltfLoader == nullptr)
	{
		LoadScene(m_activeScene);
		return;
	}

	ImGuiIO io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameBorderSize = 1.0f;

	const uint32_t W = this->GetWidth();
	const uint32_t H = this->GetHeight();

	const uint32_t PROFILER_WINDOW_PADDING_X = 10;
	const uint32_t PROFILER_WINDOW_PADDING_Y = 10;
	const uint32_t PROFILER_WINDOW_SIZE_X = 330;
	const uint32_t PROFILER_WINDOW_SIZE_Y = 450;
	const uint32_t PROFILER_WINDOW_POS_X = W - PROFILER_WINDOW_PADDING_X - PROFILER_WINDOW_SIZE_X;
	const uint32_t PROFILER_WINDOW_POS_Y = W - PROFILER_WINDOW_PADDING_Y;

	const uint32_t CONTROLS_WINDOW_POS_X = 10;
	const uint32_t CONTROLS_WINDOW_POS_Y = 10;
	const uint32_t CONTROLW_WINDOW_SIZE_X = 350;
	const uint32_t CONTROLW_WINDOW_SIZE_Y = 780;	// assuming > 720p

	// Render CONTROLS Window.
	//
	ImGui::SetNextWindowPos(ImVec2(CONTROLS_WINDOW_POS_X, CONTROLS_WINDOW_POS_Y), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(CONTROLW_WINDOW_SIZE_X, CONTROLW_WINDOW_SIZE_Y), ImGuiCond_FirstUseEver);

	if (m_UIState.bShowControlsWindow)
	{
		ImGui::Begin("CONTROLS (F1)", &m_UIState.bShowProfilerWindow);
		if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Checkbox("Play", &m_bPlay);
			ImGui::SliderFloat("TIme", &m_time, 0, 30);
		}

		ImGui::Spacing();
		ImGui::Spacing();
	}
}

void UIState::Initialize()
{
	//
	// 
	// Init GUI state.
	this->SelectedTonemapperIndex = 0;
	this->bUseTAA = false;
	this->bUseMagnifier = false;
	this->bLockMagnifierPosition = this->bLockMagnifierPositionHistory = false;
	this->SelectedSkydomeTypeIndex;
	this->Exposure = 1.0f;
	this->IBLFactor = 2.0f;
	this->EmissiveFactor = 1.0f;
	this->bDrawLightFrustum = false;
	this->bDrawBoundingBox = false;
	this->WireframeMode = WireframeMode::WIREFRAME_MODE_OFF;
	this->WireframeColor[0] = 0.0f;
	this->WireframeColor[1] = 1.0f;
	this->WireframeColor[2] = 0.0f;
	this->bShowControlsWindow = true;
	this->bShowProfilerWindow = true;
}
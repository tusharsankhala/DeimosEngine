#include "Common/stdafx.h"

#include "UI/UI.h"
#include "VK/Application/Application.h"
#include <ImGui/imgui.h>

#include "VK/Base/FrameworkWindows.h"

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
	this->EmissiveColor = 1.0f;
	this->bDrawLightFrustum = false;
	this->bDrawBoundingBox = false;
	this->WireframeMode = WireframeMode::WIREFRAME_MODE_OFF;
	this->WireframeColor[0] = 0.0f;
	this->WireframeColor[1] = 1.0f;
	this->WireframeColor[2] = 0.0f;
	this->bShowControlsWindow = true;
	this->bShowProfilerWindow = true;
}
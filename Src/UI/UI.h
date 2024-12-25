#pragma once

#include <string>

struct UIState
{
	//
	// Window Management
	//
	bool bShowControlsWindow;
	bool bShowProfilerWindow;

	//
	// Post Process Controls
	//
	//-------------------------------------
	int SelectedTonemapperIndex;
	float Exposure;

	bool bUseTAA;

	bool bUseMagnifier;
	bool bLockMagnifierPosition;
	bool bLockMagnifierPositionHistory;
	int	 LockedMagnifiedScreenPositionX; 
	int	 LockedMagnifiedScreenPositionY;

	//
	// APP/Scene CONTROLS
	//
	//-------------------------------------
	float IBLFactor;
	float EmissiveFactor;
	int	  SelectedSkydomeTypeIndex;

	bool  bDrawLightFrustum;
	bool  bDrawBoundingBox;

	enum class WireframeMode : int
	{
		WIREFRAME_MODE_OFF = 0,
		WIREFRAME_MODE_SHARED = 1,
		WIREFRAME_MODE_SOLID_COLOR = 2,
	};

	WireframeMode WireframeMode;
	float		  WireframeColor[3];	
	//
	// PROFILER CONTROLS
	//
	bool  bShowMilliseconds;

	void Initialize();

};

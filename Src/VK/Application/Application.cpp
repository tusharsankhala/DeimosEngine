#include "VK/Common/stdafx.h"
#include <intrin.h>
#include "Application.h"

Application::Application(LPCSTR name) : FrameworkWindows(name)
{
	m_time = 0;
	m_bPlay = true;

	m_pGltfLoader = NULL;

}

//--------------------------------------------------------------------------------------
//
// OnParseCommandLine.
//
//--------------------------------------------------------------------------------------
void Application::OnParseCommandLine(LPSTR lpCmdLine, uint32_t* pWidth, uint32_t* pHeight)
{
	*pWidth				= 1920;
	*pHeight			= 1080;
	m_activeScene		= 0;			// Load the first one by default.
	m_bIsBenchmarking	= false;
	m_vsyncEnabled		= false;
	m_fontSize			= 13.f;
	m_activeCamera		= 0;
}

void Application::OnCreate()
{
	// Init the shader compiler.
	InitDirectXCompiler();
	CreateShaderCache();

	// Create a instane of the renderer and initialize it, we need to do that for each GPU.
	m_pRenderer = new Renderer();
	m_pRenderer->OnCreate( &m_device, &m_swapChain, m_fontSize );

	// Init GUI( non GFX stuff )
	ImGUI_Init((void*) m_windowHwnd );
	m_UIState.Initialize();

	OnResize( true );
	OnUpdateDisplay();

	// Init camera, looking at the origin.
	m_camera.LookAt(math::Vector4( 0, 0, 5, 0 ), math::Vector4( 0, 0, 0, 0 ));
}

void Application::OnDestroy()
{
}

//--------------------------------------------------------------------------------------
//
// OnRender, updates the state from the UI, animates, transforms and renders the scene
//
//--------------------------------------------------------------------------------------

void Application::OnRender()
{
	// Do any start of frame necessities.
	BeginFrame();
}

//--------------------------------------------------------------------------------------
//
// OnEvent, win32 sends us events and we forward them to ImGUI.
//
//--------------------------------------------------------------------------------------

bool Application::OnEvent( MSG msg )
{
	if( ImGUI_WndProcHandler( msg.hwnd, msg.message, msg.wParam, msg.lParam ) )
	{
		return true;
	}

	// Handle function keys ( F1, F2... ) here, rest of the input is handled.
	// by ImGUI later in HandleInput() function.
	const WPARAM& keyPressed = msg.wParam;
	switch( msg.message )
	{
		case WM_KEYUP:
		case WM_SYSKEYUP:
			/* WINDOW TOGGLES */
			if( keyPressed == VK_F1 )	m_UIState.bShowControlsWindow ^= 1;
			if (keyPressed == VK_F2)	m_UIState.bShowProfilerWindow ^= 1;
			break;
	}

	return true;
}

//--------------------------------------------------------------------------------------
//
// OnResize.
//
//--------------------------------------------------------------------------------------

void Application::OnResize( bool resizeRender )
{
	// Destory resources ( if we are not minimised )
	if( resizeRender && m_width && m_height && m_pRenderer )
	{
		m_pRenderer->OnDestoryWindowSizeDependentResources();
		m_pRenderer->OnCreateWindowSizeDependentResources( &m_swapChain, m_width, m_height );
	}

	m_camera.SetFov( PI_OVER_4, m_width, m_height, 0.1f, 1000.0f );
}

//--------------------------------------------------------------------------------------
//
// OnUpdateDisplay.
//
//--------------------------------------------------------------------------------------

void Application::OnUpdateDisplay()
{
	// Destroy rersources ( If we are not minimised )
	if( m_pRenderer )
	{
		m_pRenderer->OnUpdateDisplayDependentResources( &m_swapChain, m_UIState.bUseMagnifier );
	}
}

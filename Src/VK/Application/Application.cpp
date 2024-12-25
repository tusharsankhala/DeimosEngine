#include "VK/Common/stdafx.h"
#include <intrin.h>
#include "Application.h"
#include <Common/GLTF/GltfHelpers.h>

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

	// read globals.
	auto process = [&](json jData)
	{
		*pWidth = jData.value("width", *pWidth);
		*pHeight = jData.value("height", *pHeight);
		m_fullscreenMode = jData.value("presentationMode", m_fullscreenMode);
		m_activeScene = jData.value("activeScene", m_activeScene);
		m_activeCamera = jData.value("activeCamera", m_activeCamera);
		m_isCpuValidationLayerEnabled = jData.value("CpuValidationLayerEnabled", m_isCpuValidationLayerEnabled);
		m_isGpuValidationLayerEnabled = jData.value("GpuValidationLayerEnabled", m_isGpuValidationLayerEnabled);
		m_vsyncEnabled = jData.value("vsync", m_vsyncEnabled);
		m_freesyncHDROptionEnabled = jData.value("FreesyncHDROptionEnabled", m_freesyncHDROptionEnabled);
		m_bIsBenchmarking = jData.value("benchmark", m_bIsBenchmarking);
		m_fontSize = jData.value("fontSize", m_fontSize);
	};

	// Read config file ( and override values form commandline if so )
	// 
	{
		std::ifstream f("GLTFSample.json");
		if (!f)
		{
			MessageBox(NULL, "Config file not found!\n", "Cauldron Panic!", MB_ICONERROR);
			exit(0);
		}

		try
		{
			f >> m_jsonConfigFile;
		}
		catch (json::parse_error)
		{
			MessageBox(NULL, "Error parsing GLTFSample.json!\n", "Cauldron Panic!", MB_ICONERROR);
			exit(0);
		}
	}

	json globals = m_jsonConfigFile["globals"];
	process(globals);

	// get the list of scenes
	for (const auto& scene : m_jsonConfigFile["scenes"])
		m_sceneNames.push_back(scene["name"]);
	
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
	ImGUI_Shutdown();

	m_device.GPUFlush();

	//m_pRenderer->UnloadScene();
	m_pRenderer->OnDestroyWindowSizeDependentResources();
	m_pRenderer->OnDestroy();

	delete m_pRenderer;

	// shutdown the shader compiler.
	DestroyShaderCache(&m_device);

	if (m_pGltfLoader)
	{
		delete m_pGltfLoader;
		m_pGltfLoader = NULL;
	}
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

	ImGUI_UpdateIO();
	ImGui::NewFrame();

	if (m_loadingScene)
	{
		// the scene loads in chuncks, that way me may show a progress bar.
		static int loadingStage = 0;
		loadingStage = m_pRenderer->LoadScene(m_pGltfLoader, loadingStage);
		if (loadingStage == 0)
		{
			m_time = 0;
			m_loadingScene = false;
		}
	}
	else if (m_pGltfLoader && m_bIsBenchmarking)
	{
		// Benchmarking takes control of the time, and exits the app whne the animation is done.
		std::vector<TimeStamp> timeStamps = m_pRenderer->GetTimingValues();
		std::string Filename;
		m_time = BenchmarkLoop(timeStamps, &m_camera, Filename);
	}
	else
	{
		BuildUI();	// UI logic, Note that the rendering of the UI happens later.
		OnUpdate();	// Update camera, handle keyboard/moouse input.
	}

	// Do render frame using AFR.
	m_pRenderer->OnRender(&m_UIState, m_camera, &m_swapChain);

	// Framework will handle Present and some other end of frame magic.
	EndFrame();
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
		m_pRenderer->OnDestroyWindowSizeDependentResources();
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

//--------------------------------------------------------------------------------------
//
// LoadScene.
//
//--------------------------------------------------------------------------------------

void Application::LoadScene(int sceneIndex)
{
	json scene = m_jsonConfigFile["scenes"][sceneIndex];

	// Release everything and load the GLTF, just the light json data, the rest ( textures and geometry) will be done in the main loop.
	if (m_pGltfLoader != NULL)
	{
		m_pRenderer->UnloadScene();
		m_pRenderer->OnDestroyWindowSizeDependentResources();
		m_pRenderer->OnDestroy();
		m_pGltfLoader->Unload();
		m_pRenderer->OnCreate(&m_device, &m_swapChain, m_fontSize);
		m_pRenderer->OnCreateWindowSizeDependentResources(&m_swapChain, m_width, m_height);
	}

	delete(m_pGltfLoader);
	m_pGltfLoader = new GLTFCommon();
	if (m_pGltfLoader->Load(scene["directory"], scene["filename"]) == false)
	{
		MessageBox(NULL, "The selected model could not be found", "Renderer Panic!", MB_ICONERROR);
		exit(0);
	}

	// Load the UI settings, and also some defaults  cameras and lights, in case the GLTF has none.
	{
#define LOAD(j, key, val) val = j.value(key, val)

		// global settings.
		LOAD(scene, "TAA", m_UIState.bUseTAA);
		LOAD(scene, "toneMapper", m_UIState.SelectedTonemapperIndex);
		LOAD(scene, "skyDomeType", m_UIState.SelectedSkydomeTypeIndex);
		LOAD(scene, "exposure", m_UIState.Exposure);
		LOAD(scene, "iblFactor", m_UIState.IBLFactor);
		LOAD(scene, "emmisiveFactor", m_UIState.EmissiveFactor);
		LOAD(scene, "skyDomeType", m_UIState.SelectedSkydomeTypeIndex);

		// Add a default light in case there are none.
		if (m_pGltfLoader->m_lights.size() == 0)
		{
			tfNode n;
			n.m_transform.LookAt(PolarToVector(PI_OVER_2, 0.58f) * 3.5f, math::Vector4(0, 0, 0, 0));

			tfLight l;
			l.m_type = tfLight::LIGHT_SPOTLIGHT;
			l.m_intensity = scene.value("intensity", 1.0f);
			l.m_color = math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
			l.m_range = 15;
			l.m_outerConeAngle = PI_OVER_4;
			l.m_innerConeAngle = PI_OVER_4 * 0.9f;
			l.m_shadowResolution = 1024;

			m_pGltfLoader->AddLight(n, l);
		}

		// Allocate shadow information ( if any)
		m_pRenderer->AllocateShadowMaps(m_pGltfLoader);

		// Set default camera.
		json camera = scene["camera"];
		m_activeCamera = scene.value("äctiveCamera", m_activeCamera);
		math::Vector4 from = GetVector(GetElementJsonArray(camera, "defaultFrom", { 0.0, 0.0, 10.0 }));
		math::Vector4 to = GetVector(GetElementJsonArray(camera, "defaultTo", { 0.0, 0.0, 0.0 }));
		m_camera.LookAt(from, to);

		// set benchmarking state if enabled
		if (m_bIsBenchmarking)
		{
			std::string deviceName;
			std::string driverVersion;
			m_device.GetDeviceInfo(&deviceName, &driverVersion);
			BenchmarkConfig(scene["BenchmarkingSettings"], m_activeCamera, m_pGltfLoader, deviceName, driverVersion);
		}

		// indicate thge mainloop we started loading the GLTF and it needs to load the rest (textures and geometry)
		m_loadingScene = true;
	}

}

//--------------------------------------------------------------------------------------
//
// OnUpdate.
//
//--------------------------------------------------------------------------------------
void Application::OnUpdate()
{
	ImGuiIO& io = ImGui::GetIO();

	// If the mouse is not used by the GUI then it is being used by the camera.
	//
	if (io.WantCaptureMouse)
	{
		io.MouseDelta.x = 0;
		io.MouseDelta.y = 0;
		io.MouseWheel = 0;
	}

	// Update camera.
	UpdateCamera(m_camera, io);
	if (m_UIState.bUseTAA)
	{
		static uint32_t Seed;
		m_camera.SetProjectionJitter(m_width, m_height, Seed);
	}
	else
	{
		m_camera.SetProjectionJitter(0.f, 0.f);
	}

	// Keyboard and Mouse.
	HandleInput(io);

	// Animation Update.
	if (m_bPlay)
	{
		m_time += (float)m_deltaTime / 1000.0f;		// Animation time in seconds.
	}

	if (m_pGltfLoader)
	{
		m_pGltfLoader->SetAnimationTime(0, m_time);
		m_pGltfLoader->TransformScene(0, math::Matrix4::identity());
	}
}

//--------------------------------------------------------------------------------------
//
// HandleInput.
//
//--------------------------------------------------------------------------------------

void Application::HandleInput(const ImGuiIO& io)
{
	auto fnIsKeyTriggered = [&io](char key) { return io.KeysDown[key] && io.KeysDownDuration[key] == 0.0f; };

	// Handle keyboard/Mouse input here.

	/* MAGNIFIER CONTROLS */
	if (fnIsKeyTriggered('L'))
	{}

	if (fnIsKeyTriggered('M') || io.MouseClicked[2]) // middle mouse / M key toggles magnifier
	{
	}

	if( io.MouseClicked[1] && m_UIState.bUseMagnifier)	// right mouse click
	{ }
}

//--------------------------------------------------------------------------------------
//
// UpdateCamera.
//
//--------------------------------------------------------------------------------------
void Application::UpdateCamera(Camera & cam, const ImGuiIO & io)
{
	float yaw = cam.GetYaw();
	float pitch = cam.GetPitch();
	float distance = cam.GetDistance();

	cam.UpdatePreviousMatrices();	// set previous view matrix.

	// Sets camera based on UI selection ( WASD, Orbit or any of the GLTF cameras )
	if ((io.KeyCtrl == false) && (io.MouseDown[0] == true))
	{
		yaw -= io.MouseDelta.x / 100.f;
		pitch += io.MouseDelta.y / 100.f;
	}

	// Choose camera movement depending on settings.
	if (m_activeCamera == 0)
	{
		// If nothing has changed, don't calculate an update ( we are getting micro chnages in view causing bugs)
		if (!io.MouseWheel && (!io.MouseDown[0] || (!io.MouseDelta.x && !io.MouseDelta.y)))
			return;

		// Orbiting.
		distance -= (float)io.MouseWheel / 3.0f;
		distance = std::max<float>(distance, 0.1f);

		bool panning = (io.KeyCtrl == true) && (io.MouseDown[0] == true);

		cam.UpdateCameraPolar(yaw, pitch,
							  panning ? -io.MouseDelta.x / 100.0f : 0.0f,
							  panning ? -io.MouseDelta.y / 100.0f : 0.0f,
							  distance);
	}
	else if ( m_activeCamera == 1)
	{
		// WASD
		cam.UpdateCameraWASD(yaw, pitch, io.KeysDown, io.DeltaTime);
	}
	else if (m_activeCamera > 1)
	{
		// Use a camera from the GLTF
		m_pGltfLoader->GetCamera(m_activeCamera - 2, &cam);
	}
}
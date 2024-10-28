#include "Common/stdafx.h"
#include "FrameworkWindows.h"
#include "Common/Misc/Misc.h"

#include <array>

LRESULT CALLBACK WindowProc( HWND hWnd,
                             UINT message,
                             WPARAM wParam,
                             LPARAM lParam );

static const char* const WINDOW_CLASS_NAME  = "DeimosEngine";
static FrameworkWindows* pFrameworkInstance	= nullptr;
static bool bIsMinimised                    = false;
static RECT m_windowRect;
static LONG lBorderedStyle                  = 0;
static LONG lBorderlessStyle                = 0;
static UINT lwindowStyle                    = 0;

// Deafult values for validation layers - applications can override these values in their constructors.
#if _DEBUG
static constexpr bool ENABLE_CPU_VALIDATION_DEFAULT  = true;
static constexpr bool ENABLE_GPU_VALIDATION_DEFAULT = true;
#else // Release
static constexpr bool ENABLE_CPU_VALIDATION_DEFAULT = false;
static constexpr bool ENABLE_GPU_VALIDATION_DEFAULT = false;
#endif

int RunFramework( HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow, FrameworkWindows* pFramework )
{
	// Init Logging.
    // Init logging
    int result = Log::InitLogSystem();
    assert(!result);

    // Init window class.
    HWND hWnd;
    WNDCLASSEX windowClass;

    ZeroMemory(&windowClass, sizeof(WNDCLASSEX));
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = WINDOW_CLASS_NAME;
    windowClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(101)); // '#define ICON_IDI 101' is assumed to be included with resource.h
    if (windowClass.hIcon == NULL)
    {
        DWORD dw = GetLastError();
        if (dw == ERROR_RESOURCE_TYPE_NOT_FOUND)
            Log::Trace("Warning: Icon file or .rc file not found, using default Windows app icon.");
        else
            Log::Trace("Warning: error loading icon, using default Windows app icon.");
    }
    RegisterClassEx(&windowClass);

    // If this is null, nothing to do, bail
    assert(pFramework);
    if (!pFramework) return -1;
    pFrameworkInstance = pFramework;

    // Get command line and config file parameters for app run
    uint32_t Width = 1920;
    uint32_t Height = 1080;
    pFramework->OnParseCommandLine(lpCmdLine, &Width, &Height);

    // Window setup based on config params
    lwindowStyle = WS_OVERLAPPEDWINDOW;
    RECT windowRect = { 0, 0, (LONG)Width, (LONG)Height };
    AdjustWindowRect(&windowRect, lwindowStyle, FALSE);    // adjust the size

    // This makes sure that in a multi-monitor setup with different resolutions, get monitor info returns correct dimensions
    SetProcessDpiAwarenessContext( DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 );

    // Create the window
    hWnd = CreateWindowEx(NULL,
        WINDOW_CLASS_NAME,    // name of the window class
        pFramework->GetName(),
        lwindowStyle,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        NULL,    // we have no parent window, NULL
        NULL,    // we aren't using menus, NULL
        hInstance,    // application handle
        NULL);    // used with multiple windows, NULL

    // Framework owns device and swapchain, so initialize them
    pFramework->DeviceInit(hWnd);

    // Sample create callback
    pFramework->OnCreate();

    // show the window
    ShowWindow(hWnd, nCmdShow);
    lBorderedStyle = GetWindowLong(hWnd, GWL_STYLE);
    lBorderlessStyle = lBorderedStyle & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);

    // main loop
    MSG msg = { 0 };
    while (msg.message != WM_QUIT)
    {
        // check to see if any messages are waiting in the queue
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg); // translate keystroke messages into the right format            
            DispatchMessage(&msg); // send the message to the WindowProc function
        }
        else if (!bIsMinimised)
            pFramework->OnRender();
    }

    // Destroy app-side framework 
    pFramework->OnDestroy();

    // Shutdown all the device stuff before quitting
    pFramework->DeviceShutdown();

    // Delete the framework created by the sample
    pFrameworkInstance = nullptr;
    delete pFramework;

    // Shutdown logging before quitting the application
    Log::TerminateLogSystem();

    // return this part of the WM_QUIT message to Sample
    return static_cast<char>(msg.wParam);// Init window class
}

void SetFullscreen( HWND hWnd, bool fullScreen )
{

}

// This si the main message handler for the program.
LRESULT CALLBACK WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    // sort through and find what code to run for the message given
    switch (message)
    {
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }

    // When close button is clicked on window
    case WM_CLOSE:
    {
        PostQuitMessage(0);
        return 0;
    }

    case WM_KEYDOWN:
    {
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }

        break;
    }

    case WM_SYSKEYDOWN:
    {
        const bool bAltKeyDown = (lParam & (1 << 29));
        if ((wParam == VK_RETURN) && bAltKeyDown) // For simplicity, alt+enter only toggles in/out windowed and borderless fullscreen
            pFrameworkInstance->ToggleFullScreen();
        break;
    }

    case WM_SIZE:
    {
        if (pFrameworkInstance)
        {
            RECT clientRect = {};
            GetClientRect(hWnd, &clientRect);
            pFrameworkInstance->OnResize(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
            bIsMinimised = (IsIconic(hWnd) == TRUE);
            return 0;
        }
        break;
    }

    // When window goes outof focus, use this event to fall back on SDR.
    // If we don't gracefully fallback to SDR, the renderer will output HDR colours which will look extremely bright and washed out.
    // However if you want to use breakpoints in HDR mode to inspect/debug values, you will have to comment this function call.
    case WM_ACTIVATE:
    {
        if (pFrameworkInstance)
        {
            pFrameworkInstance->OnActivate(wParam != WA_INACTIVE);
        }

        break;
    }

    case WM_MOVE:
    {
        if (pFrameworkInstance)
        {
            pFrameworkInstance->OnWindowMove();

            return 0;
        }
        break;
    }

    // Turn off MessageBeep sound on Alt+Enter
    case WM_MENUCHAR: return MNC_CLOSE << 16;
    }

    if (pFrameworkInstance)
    {
        MSG msg;
        msg.hwnd = hWnd;
        msg.message = message;
        msg.wParam = wParam;
        msg.lParam = lParam;
        pFrameworkInstance->OnEvent(msg);
    }

    // Handle any messages the switch statement didn't
    return DefWindowProc(hWnd, message, wParam, lParam);
}

static std::string GetCPUNameString()
{
    int nIDs = 0;
    int nExIDs = 0;

    char strCPUName[0x40] = { };

    std::array<int, 4> cpuInfo;
    std::vector<std::array<int, 4>> extData;

    __cpuid(cpuInfo.data(), 0);

    // Calling __cpuid with 0x80000000 as the function_id argument
    // gets the number of the highest valid extended ID.
    __cpuid(cpuInfo.data(), 0x80000000);

    nExIDs = cpuInfo[0];
    for (int i = 0x80000000; i <= nExIDs; ++i)
    {
        __cpuidex(cpuInfo.data(), i, 0);
        extData.push_back(cpuInfo);
    }

    // Interpret CPU strCPUName string if reported
    if (nExIDs >= 0x80000004)
    {
        memcpy(strCPUName, extData[2].data(), sizeof(cpuInfo));
        memcpy(strCPUName + 16, extData[3].data(), sizeof(cpuInfo));
        memcpy(strCPUName + 32, extData[4].data(), sizeof(cpuInfo));
    }

    return strlen(strCPUName) != 0 ? strCPUName : "UNAVAILABLE";
}

namespace Engine_VK
{
	FrameworkWindows::FrameworkWindows( LPCSTR name )
        : m_name(name)
        , m_width(0)
        , m_height(0)

        // Simulation management
        , m_lastFrameTime(MillisecondsNow())
        , m_deltaTime(0.0)

        // Device management
        , m_windowHwnd(NULL)
        , m_device()
        , m_stablePowerState(false)
        , m_isCpuValidationLayerEnabled( ENABLE_CPU_VALIDATION_DEFAULT )
        , m_isGpuValidationLayerEnabled( ENABLE_GPU_VALIDATION_DEFAULT )

        // Swapchain management
        , m_swapChain()
        , m_vsyncEnabled(false)
        , m_fullscreenMode(PRESENTATIONMODE_WINDOWED)
        , m_previousFullscreenMode(PRESENTATIONMODE_WINDOWED)

        // Display management
        , m_monitor()
        , m_freesyncHDROptionEnabled(false)
        , m_previousDisplayModeNamesIndex(DISPLAYMODE_SDR)
        , m_currentDisplayModeNamesIndex(DISPLAYMODE_SDR)
        , m_displayModesAvailable()
        , m_displayModesNamesAvailable()
        , m_enableLocalDimming(true)

        // System info
        , m_systemInfo() // initialized after device
	{}

    void FrameworkWindows::DeviceInit( HWND windowsHandle )
    {
        // Store the window handle ( other things need it later )
        m_windowHwnd = windowsHandle;

        // Create Device.
        m_device.OnCreate( m_name, "DeimosEngine v0.1", m_isCpuValidationLayerEnabled, m_isGpuValidationLayerEnabled, m_windowHwnd);
        m_device.CreatePipelineCache();

        // Get the monitor.
        m_monitor = MonitorFromWindow( m_windowHwnd, MONITOR_DEFAULTTONEAREST );

        // Create Swapchain.
        uint32_t dwNumberOfBackBuffers = 2;
        m_swapChain.OnCreate( &m_device, dwNumberOfBackBuffers, m_windowHwnd );

        m_swapChain.EnumerateDisplayModes( &m_displayModesAvailable, &m_displayModesNamesAvailable );

        if( m_previousFullscreenMode != m_fullscreenMode )
        {
            HandleFullScreen();
            m_previousFullscreenMode = m_fullscreenMode;
        }

        // Get the system info.
        std::string dummyStr;
        m_device.GetDeviceInfo( &m_systemInfo.mGPUName, &dummyStr );    // 2nd parameter is unused.
        m_systemInfo.mCPUName = GetCPUNameString();
        m_systemInfo.mGfxAPI = "Vulkan";
    }

    void FrameworkWindows::DeviceShutdown()
    {
        // Fullscreen state should always be false before exiting the app.
        if ( m_fullscreenMode = PRESENTATIONMODE_EXCLUSIVE_FULLSCREEN )
            m_swapChain.SetFullScreen( false );

        m_swapChain.OnDestroyWindowSizeDependentResources();
        m_swapChain.OnDestroy();

        m_device.DestroyPipelineCache();
        m_device.OnDestroy();
    }

    // BeginFrame will handle time updates and other start of frame logic needed
    void FrameworkWindows::BeginFrame()
    {
        // Get Timings.
        double timeNow = MillisecondsNow();
        m_deltaTime = (float)( timeNow - m_lastFrameTime );
        m_lastFrameTime = timeNow;
    }

    // EndFrame will handle Present and other end of frame logic needed
    void FrameworkWindows::EndFrame()
    {
        VkResult res = m_swapChain.Present();

        // *********************************************************************************
        // Edge case for handling Fullscreen Exclusive (FSE) mode
        // Usually OnActivate() detects application changing focus and that's where this transition is handled.
        // However, SwapChain::GetFullScreen() returns true when we handle the event in the OnActivate() block,
        // which is not expected. Hence we handle FSE -> FSB transition here at the end of the frame.
        if ( res == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT )
        {
            // For sanity check: it should only be possible when we are in FULLSCREENMODE_EXCLUSIVE_FULLSCREEN mode.
            assert( m_fullscreenMode == PRESENTATIONMODE_EXCLUSIVE_FULLSCREEN );

            m_fullscreenMode = PRESENTATIONMODE_BORDERLESS_FULLSCREEN;
            HandleFullScreen();
        }
        // *********************************************************************************
    }


    void FrameworkWindows::ToggleFullScreen()
    {
        if( m_fullscreenMode == PRESENTATIONMODE_WINDOWED )
        {
            m_fullscreenMode = PRESENTATIONMODE_BORDERLESS_FULLSCREEN;
        }
        else
        {
            m_fullscreenMode = PRESENTATIONMODE_WINDOWED;
        }

        HandleFullScreen();
        m_previousFullscreenMode = m_fullscreenMode;
    }

    void FrameworkWindows::HandleFullScreen()
    {
        // Flush the gpu to make sure we don't change anything stil active
        m_device.GPUFlush();

        //-------------------- For HDR only.
        // if FS2 modes, always fallback to SDR.
        if (m_fullscreenMode == PRESENTATIONMODE_WINDOWED &&
            (m_displayModesAvailable[m_currentDisplayModeNamesIndex] == DISPLAYMODE_FSHDR_Gamma22 ||
                m_displayModesAvailable[m_currentDisplayModeNamesIndex] == DISPLAYMODE_FSHDR_SCRGB))
        {
            m_currentDisplayModeNamesIndex = DISPLAYMODE_SDR;
        }

        // When hdr10 modes, fall back to SDR unless windowMode hdr is enabled.
        if (m_fullscreenMode == PRESENTATIONMODE_WINDOWED && !CheckIfWindowModeHdrOn() &&
            (m_displayModesAvailable[m_currentDisplayModeNamesIndex] == DISPLAYMODE_SDR))
        {
            m_currentDisplayModeNamesIndex = DISPLAYMODE_SDR;
        }
        // for every other case go back to previous state.
        else
        {
            m_currentDisplayModeNamesIndex = m_previousDisplayModeNamesIndex;
        }

        // ------------------- For HDR only.
        bool resizeResources = false;

        switch( m_fullscreenMode )
        {
            case PRESENTATIONMODE_WINDOWED:
            {
                if ( m_previousFullscreenMode == PRESENTATIONMODE_EXCLUSIVE_FULLSCREEN )
                {
                    m_swapChain.SetFullScreen( false );
                }

                SetFullscreen( m_windowHwnd, false );

                break;
            }

            case PRESENTATIONMODE_BORDERLESS_FULLSCREEN:
            {
                if ( m_previousFullscreenMode == PRESENTATIONMODE_WINDOWED )
                {
                    SetFullscreen(m_windowHwnd, false);                   
                }
                else if (m_previousFullscreenMode == PRESENTATIONMODE_EXCLUSIVE_FULLSCREEN )
                {
                    m_swapChain.SetFullScreen( false );
                    resizeResources = true;
                }
   
                break;
            }

            case PRESENTATIONMODE_EXCLUSIVE_FULLSCREEN:
            {
                if ( m_previousFullscreenMode == PRESENTATIONMODE_WINDOWED )
                {
                    SetFullscreen( m_windowHwnd, false );

                    // This should automatically call onResize thanks to WM_SIZE with Correct presentation mode.
                }
                else if ( m_previousFullscreenMode == PRESENTATIONMODE_BORDERLESS_FULLSCREEN )
                {
                    resizeResources = true;
                }

                m_swapChain.SetFullScreen(false);

                break;
            }
        }

        RECT clientRect = {};
        GetClientRect(m_windowHwnd, &clientRect);
        uint32_t nw = clientRect.right - clientRect.left;
        uint32_t nh = clientRect.bottom - clientRect.top;
        resizeResources = (resizeResources && nw == m_width && nh == m_height);
        OnResize(nw, nh);
        if ( resizeResources )
        {
            UpdateDisplay();
            OnResize(true);
        }
    }

    void FrameworkWindows::OnActivate( bool windowActive )
    {
        // *********************************************************************************
        // Edge case for handling Fullscreen Exclusive (FSE) mode 
        // FSE<->FSB transitions are handled here and at the end of EndFrame().
        if( windowActive && 
            m_windowHwnd == GetForegroundWindow() &&
            m_fullscreenMode == PRESENTATIONMODE_BORDERLESS_FULLSCREEN &&
            m_previousFullscreenMode == PRESENTATIONMODE_EXCLUSIVE_FULLSCREEN )
        {
            m_fullscreenMode = PRESENTATIONMODE_EXCLUSIVE_FULLSCREEN;
            m_previousFullscreenMode = PRESENTATIONMODE_BORDERLESS_FULLSCREEN;
            HandleFullScreen();
            m_previousFullscreenMode = m_fullscreenMode;
        }
        // *********************************************************************************
        
        if ( m_displayModesAvailable[ m_currentDisplayModeNamesIndex ] == DisplayMode::DISPLAYMODE_SDR &&
                m_displayModesAvailable[ m_previousDisplayModeNamesIndex ] == DISPLAYMODE_SDR)
            return;

        if( CheckIfWindowModeHdrOn() &&
            ( m_displayModesAvailable[ m_currentDisplayModeNamesIndex ] == DISPLAYMODE_HDR10_2048 || 
                m_displayModesAvailable[ m_currentDisplayModeNamesIndex ] == DISPLAYMODE_HDR10_SCRGB ))
            return;

        // Fall back HDR to SDR when window is fullscreen but not the active window or foreground window
        DisplayMode old = m_currentDisplayModeNamesIndex;
        m_currentDisplayModeNamesIndex = windowActive && ( m_fullscreenMode != PRESENTATIONMODE_WINDOWED ) ? m_previousDisplayModeNamesIndex : DisplayMode::DISPLAYMODE_SDR;
        if( old != m_currentDisplayModeNamesIndex )
        {
            UpdateDisplay();
            OnResize( true );
        }
    }

    void FrameworkWindows::OnWindowMove()
    {
        HMONITOR currentMonitor = MonitorFromWindow(m_windowHwnd, MONITOR_DEFAULTTONEAREST);
        if (m_monitor != currentMonitor)
        {
            m_swapChain.EnumerateDisplayModes(&m_displayModesAvailable, &m_displayModesNamesAvailable);
            m_monitor = currentMonitor;
            m_previousDisplayModeNamesIndex = m_currentDisplayModeNamesIndex = DISPLAYMODE_SDR;
            OnResize(m_width, m_height);
            UpdateDisplay();
        }
    }

    void FrameworkWindows::OnResize(uint32_t width, uint32_t height)
    {
        bool fr = (m_width != width || m_height != height);
        m_width = width;
        m_height = height;
        if (fr)
        {
            UpdateDisplay();
            OnResize(true);
        }
    }


    void FrameworkWindows::UpdateDisplay()
    {
        // Nothing has changed in the UI.
        if( m_displayModesAvailable[ m_currentDisplayModeNamesIndex] < 0 )
        {
            m_currentDisplayModeNamesIndex = m_previousDisplayModeNamesIndex;
            return;
        }

        // Flush GPU.
        m_device.GPUFlush();

        m_swapChain.OnDestroyWindowSizeDependentResources();

        m_swapChain.OnCreateWindowSizeDependentResources( m_width, m_height, m_vsyncEnabled, m_displayModesAvailable[ m_currentDisplayModeNamesIndex],
                                                           m_fullscreenMode, m_enableLocalDimming );
    }

    
}
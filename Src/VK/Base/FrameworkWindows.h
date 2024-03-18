#pragma once

#include "Common/Misc/Misc.h"
#include "VK/Base/FreeSyncHDR.h"
#include "VK/Base/Device.h"
#include "VK/Base/SwapChain.h"

namespace Engine_VK
{
	class FrameworkWindows
	{
	public:
		FrameworkWindows( LPCSTR name );
		virtual ~FrameworkWindows() {}

		// Client (Sample) application interface
		virtual void OnParseCommandLine(LPSTR lpCmdLine, uint32_t* pWidth, uint32_t* pHeight) = 0;
		
		virtual void OnCreate()						= 0;
		virtual void OnDestroy()					= 0;
		virtual void OnRender()						= 0;
		virtual bool OnEvent( MSG msg)				= 0;
		virtual void OnResize( bool resizeRender )	= 0;

		// Device & swapchain management
		void DeviceInit( HWND WindowsHandle );
		void DeviceShutdown();
		void BeginFrame();
		void EndFrame();
	
		// Fullscreen management & window events are handled by Engine instead of the client application.
		void ToggleFullScreen();
		void HandleFullScreen();
		void OnActivate(bool windowActive);
		void OnWindowMove();
		void UpdateDisplay();
		void OnResize( uint32_t width, uint32_t height);

		// Getters.
		inline LPCSTR		GetName()	const { return m_name; }
		inline uint32_t		GetWidth()	const { return m_width; }
		inline uint32_t		GetHeight() const { return m_height; }

	protected:

		// Swapchain management.
		SwapChain					m_swapChain;
		bool						m_vSyncEnabled;
		PresentationMode			m_fullscreenMode;
		PresentationMode			m_previousFullscreenMode;

		// Display management.
		HMONITOR					m_monitor;
		DisplayMode					m_previousDisplayModeNamesIndex;
		DisplayMode					m_currentDisplayModeNamesIndex;
		std::vector<DisplayMode>	m_displayModesAvailable;
		std::vector<const char*>	m_displayModesNamesAvailable;
		bool						m_enableLocalDimming;

		LPCSTR						m_name;		// Application name.
		int							m_width;	// application window width.
		int							m_height;	// application window height.

		// Simulation management.
		double						m_lastFrameTime;
		double						m_deltaTime;

		// Device Management.
		HWND						m_windowHwnd;
		Device						m_device;
		bool						m_stablePowerState;
		bool						m_isCpuValidationLayerEnabled;
		bool						m_isGpuValidationLayerEnabled;

		// System Info.
		struct SystemInfo
		{
			std::string mCPUName	= "UNAVAILABLE";
			std::string mGPUName	= "UNAVAILABLE";
			std::string mGfxAPI		= "UNAVAILABLE";
		};
		SystemInfo	m_systemInfo;
	};
} // Engine_VK

using namespace Engine_VK;
int RunFramework( HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow, FrameworkWindows* pFramework );
void SetFullscreen( HWND hWnd, bool fullScreen );


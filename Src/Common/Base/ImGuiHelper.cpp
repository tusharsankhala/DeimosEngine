#include "Common/stdafx.h"
#include "Common/Base/ImGuiHelper.h"


static bool IsAnyMouseButtonDown()
{
	ImGuiIO& io = ImGui::GetIO();
	for( int n = 0; n < IM_ARRAYSIZE( io.MouseDown); ++n )
		if( io.MouseDown[n] )
			return true;

	return false;
}

IMGUI_API LRESULT ImGUI_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	ImGuiIO& io = ImGui::GetIO();
	switch( msg )
	{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		{
			int button = 0;
			if( msg == WM_LBUTTONDOWN ) button = 0;
			else if ( msg == WM_RBUTTONDOWN ) button = 1;
			else if (msg == WM_MBUTTONDOWN ) button = 2;
			if( !IsAnyMouseButtonDown() && GetCapture() == NULL )
			{
				SetCapture( hWnd );
			}

			io.MouseDown[ button ] = true;

			return 0;
		}

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		{
			int button = 0;
			if (msg == WM_LBUTTONUP) button = 0;
			else if (msg == WM_RBUTTONUP) button = 1;
			else if (msg == WM_MBUTTONUP) button = 2;
			io.MouseDown[button] = false;
			if (!IsAnyMouseButtonDown() && GetCapture() == hWnd)
				ReleaseCapture();
			return 0;
		}

		case WM_MOUSEWHEEL:
			io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
			return 0;
		case WM_MOUSEMOVE:
			io.MousePos.x = (signed short)(lParam);
			io.MousePos.y = (signed short)(lParam >> 16);
			return 0;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if (wParam < 256)
				io.KeysDown[wParam] = true;
			return 0;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			if (wParam < 256)
				io.KeysDown[wParam] = false;
			return 0;
		case WM_CHAR:
			// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
			if (wParam > 0 && wParam < 0x10000)
				io.AddInputCharacter((unsigned short)wParam);
			return 0;
	}

	return 0;
}
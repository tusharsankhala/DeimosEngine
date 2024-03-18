#pragma once

#include <ImGui/imgui.h>

bool ImGUI_Init( void *hwnd );
void ImGUI_Shutdown();
void ImGUI_UpdateIO();
LRESULT ImGUI_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

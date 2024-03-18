// Engine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Common/stdafx.h"
#include "VK/Application/Application.h"

//--------------------------------------------------------------------------------------
//
// WinMain
//
//--------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    LPCSTR Name = "Deimos Engine v0.0.1";

    // create new Vulkan sample
    return RunFramework(hInstance, lpCmdLine, nCmdShow, new Application(Name));
}
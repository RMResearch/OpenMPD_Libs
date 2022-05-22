#pragma once
// Example low level rendering Unity plugin
#include "PlatformBase.h"
#include "RenderAPI.h"


extern "C" {
	// --------------------------------------------------------------------------
	// UnitySetInterfaces
	void UNITY_INTERFACE_EXPORT UnityPluginLoad(IUnityInterfaces* unityInterfaces);
	void UNITY_INTERFACE_EXPORT UnityPluginUnload();
	void UNITY_INTERFACE_EXPORT UnityApplicationStart();
	void UNITY_INTERFACE_EXPORT UnityPluginApplicationQuit();
	// --------------------------------------------------------------------------
	// GetRenderEventFunc, an example function we export which is used to get a rendering event callback function.
	void UNITY_INTERFACE_EXPORT CreateVisualRenderer();
	UnityRenderingEvent UNITY_INTERFACE_EXPORT GetRenderEventFunc();
	bool UNITY_INTERFACE_EXPORT VisualRenderedReady();
	long long UNITY_INTERFACE_EXPORT getOpenGLVisualRenderer();
	void UNITY_INTERFACE_EXPORT setPMatrix(float* P);
	void UNITY_INTERFACE_EXPORT setVMatrix(float* V);
};


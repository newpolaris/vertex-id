#pragma once

#include <glad/glad.h>
#include <windows.h>
#include <iostream>

// CPU Frame Timers
struct FrameTimer {
	// High level timers
	double frameTime = 0.0;
	float  deltaTime = 0.0f;
	// CPU timers
	double frameUpdate = 0.0;
	double frameRender = 0.0;
	double win32Events = 0.0;
	double imguiLogic = 0.0;
	double imguiRender = 0.0;
	double swapBuffer = 0.0;
	// GPU timers
	double imguiGPU = 0.0;
	double appGPU = 0.0;
};

struct Profile
{
	// CPU timings
	LARGE_INTEGER timerFrequency;
	LARGE_INTEGER timerStart;
	LARGE_INTEGER timerStop;
	LARGE_INTEGER frameStart;
	LARGE_INTEGER frameStop;
	LONGLONG timerDiff;

    FrameTimer display;
    FrameTimer accumulator;

    DWORD lastTick;

	// GPU Timers
	bool slowFrame = false;
	bool firstRenderSample = true;
	GLint timerResultAvailable = 0;
	GLuint64 gpuStartTime = 0;
	GLuint64 gpuStopTime = 0;

    GLuint gGpuApplicationStart = 0;
    GLuint gGpuApplicationStop = 0;
    GLuint gGpuImguiStart = 0;
    GLuint gGpuImguiStop = 0;

	int frameCounter = 0;
    double frameBudget = 0.0;
	int displayFrequency = 0;

    Profile(void* hwnd);
	~Profile();

    void EventMessageStart();
    void EventMessageStop();
    void EventUpdateStart();
    void EventUpdateStop();
    void EventRenderStart();
    void EventRenderStop();
    void EventUIUpdateStart();
    void EventUIUpdateStop();
    void EventUIRenderStart();
    void EventUIRenderStop();
    void EventSwapStart();
    void EventSwapStop();
};

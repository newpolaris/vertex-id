#include "Profile.h"

Profile::Profile(void* hwnd)
{
	memset(&display, 0, sizeof(FrameTimer));
	memset(&accumulator, 0, sizeof(FrameTimer));
	int frameCounter = 0;

	bool enableFrameTiming = true;

#if _WIN32
    if (!QueryPerformanceFrequency(&timerFrequency)) {
        std::cout << "WinMain: QueryPerformanceFrequency failed\n";
        enableFrameTiming = false;
    }

	// Get Display Frequency
	HMONITOR hMonitor = MonitorFromWindow((HWND)hwnd, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFOEX monitorInfo;
	monitorInfo.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMonitor, &monitorInfo);
	DEVMODE devMode;
	devMode.dmSize = sizeof(DEVMODE);
	devMode.dmDriverExtra = 0;
	EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode);
	displayFrequency = (int)devMode.dmDisplayFrequency;
#endif
    
	frameBudget = (1000.0 / (double)displayFrequency);
	std::cout << "Display frequency: " << displayFrequency << "\n";
	std::cout << "Frame budget: " << frameBudget << " milliseconds\n";

	// lastTick = GetTickCount();

	glGenQueries(1, &gGpuApplicationStart);
	glGenQueries(1, &gGpuApplicationStop);
	glGenQueries(1, &gGpuImguiStart);
	glGenQueries(1, &gGpuImguiStop);
}

Profile::~Profile()
{
    glDeleteQueries(1, &gGpuApplicationStart);
    glDeleteQueries(1, &gGpuApplicationStop);
    glDeleteQueries(1, &gGpuImguiStart);
    glDeleteQueries(1, &gGpuImguiStop);
}

void Profile::EventMessageStart()
{
    // Win32 events
    // QueryPerformanceCounter(&timerStart);
}

void Profile::EventMessageStop()
{
    // QueryPerformanceCounter(&timerStop);
    // timerDiff = timerStop.QuadPart - timerStart.QuadPart;
    // accumulator.win32Events += (double)timerDiff * 1000.0 / (double)timerFrequency.QuadPart;
}

void Profile::EventUpdateStart()
{
    // QueryPerformanceCounter(&frameStart);

    // Update
    // QueryPerformanceCounter(&timerStart);
    // DWORD thisTick = GetTickCount();
    // float deltaTime = float(thisTick - lastTick) * 0.001f;
    // lastTick = thisTick;
    // accumulator.deltaTime += deltaTime;
}

void Profile::EventUpdateStop()
{
    // QueryPerformanceCounter(&timerStop);
    // timerDiff = timerStop.QuadPart - timerStart.QuadPart;
    // accumulator.frameUpdate += (double)timerDiff * 1000.0 / (double)timerFrequency.QuadPart;
}

void Profile::EventRenderStart()
{
    // Render
    // QueryPerformanceCounter(&timerStart);

    if (!firstRenderSample) { // Application GPU Timer
        glGetQueryObjectiv(gGpuApplicationStop, GL_QUERY_RESULT, &timerResultAvailable);
        while (false) { // }!timerResultAvailable) {
            std::cout << "Waiting on app GPU timer!\n";
            glGetQueryObjectiv(gGpuApplicationStop, GL_QUERY_RESULT, &timerResultAvailable);
        }
        glGetQueryObjectui64v(gGpuApplicationStart, GL_QUERY_RESULT, &gpuStartTime);
        glGetQueryObjectui64v(gGpuApplicationStop, GL_QUERY_RESULT, &gpuStopTime);
        accumulator.appGPU += (double)(gpuStopTime - gpuStartTime) / 1000000.0;
    }
    glQueryCounter(gGpuApplicationStart, GL_TIMESTAMP);
}

void Profile::EventRenderStop()
{
    glQueryCounter(gGpuApplicationStop, GL_TIMESTAMP);

    // QueryPerformanceCounter(&timerStop);
    // timerDiff = timerStop.QuadPart - timerStart.QuadPart;
    // accumulator.frameRender += (double)timerDiff * 1000.0 / (double)timerFrequency.QuadPart;
}

void Profile::EventUIUpdateStart()
{
    // IMGUI Update
    // QueryPerformanceCounter(&timerStart);
}

void Profile::EventUIUpdateStop()
{
    // QueryPerformanceCounter(&timerStop);
    // timerDiff = timerStop.QuadPart - timerStart.QuadPart;
    // accumulator.imguiLogic += (double)timerDiff * 1000.0 / (double)timerFrequency.QuadPart;
}

void Profile::EventUIRenderStart()
{
    // Imgui Render
    // QueryPerformanceCounter(&timerStart);

    if (!firstRenderSample) { // Imgui GPU Timer
        glGetQueryObjectiv(gGpuImguiStop, GL_QUERY_RESULT, &timerResultAvailable);
        while (false) { // }!timerResultAvailable) {
            std::cout << "Waiting on imgui GPU timer!\n";
            glGetQueryObjectiv(gGpuImguiStop, GL_QUERY_RESULT, &timerResultAvailable);
        }
        glGetQueryObjectui64v(gGpuImguiStart, GL_QUERY_RESULT, &gpuStartTime);
        glGetQueryObjectui64v(gGpuImguiStop, GL_QUERY_RESULT, &gpuStopTime);
        accumulator.imguiGPU += (double)(gpuStopTime - gpuStartTime) / 1000000.0;
    }

    glQueryCounter(gGpuImguiStart, GL_TIMESTAMP);
}

void Profile::EventUIRenderStop()
{
    glQueryCounter(gGpuImguiStop, GL_TIMESTAMP);

    // QueryPerformanceCounter(&timerStop);
    // timerDiff = timerStop.QuadPart - timerStart.QuadPart;
    // accumulator.imguiRender += (double)timerDiff * 1000.0 / (double)timerFrequency.QuadPart;
}
        
void Profile::EventSwapStart()
{
    // Wait for GPU
    // QueryPerformanceCounter(&timerStart);
}

void Profile::EventSwapStop()
{
    // QueryPerformanceCounter(&timerStop);
    // timerDiff = timerStop.QuadPart - timerStart.QuadPart;
    // accumulator.swapBuffer += (double)timerDiff * 1000.0 / (double)timerFrequency.QuadPart;

    // QueryPerformanceCounter(&frameStop);
    // timerDiff = frameStop.QuadPart - frameStart.QuadPart;
    // double frameTime = (double)timerDiff * 1000.0 / (double)timerFrequency.QuadPart;
    // accumulator.frameTime += frameTime;

    // Profiling house keeping
    firstRenderSample = false;
    if (++frameCounter >= 60) {
        frameCounter = 0;

        display.win32Events = accumulator.win32Events / 60.0;
        display.frameUpdate = accumulator.frameUpdate / 60.0;
        display.frameRender = accumulator.frameRender / 60.0;
        display.imguiLogic = accumulator.imguiLogic / 60.0;
        display.imguiRender = accumulator.imguiRender / 60.0;
        display.swapBuffer = accumulator.swapBuffer / 60.0;
        display.frameTime = accumulator.frameTime / 60.0;
        display.deltaTime = accumulator.deltaTime / 60.0f;
        display.appGPU = accumulator.appGPU / 60.0;
        display.imguiGPU = accumulator.imguiGPU / 60.0;

        memset(&accumulator, 0, sizeof(FrameTimer));
        slowFrame = display.frameTime >= frameBudget;
	}
}


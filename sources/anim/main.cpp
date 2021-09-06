#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stdio.h>
#include <sstream>
#include <Chapter06Sample01.h>
#include <Chapter09Sample01.h>
#include <Chapter10Sample01.h>
#include <Chapter10Sample02.h>
#include <Profile.h>
#include <ImGuizmo.h>

#include <el_debug.h>

#include <nanovg.h>
#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg_gl.h>

#include "hud.h"
#include "XmlViewer.h"
#include "DecomposeSample.h"

void APIENTRY openglCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
    using namespace std;

    // ignore these non-significant error codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 131184)
        return;

    std::stringstream out;

    out << "---------------------OPENGL-CALLBACK-START------------" << endl;
    out << "message: " << message << endl;
    out << "type: ";
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        out << "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        out << "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        out << "UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        out << "PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        out << "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_OTHER:
        out << "OTHER";
        break;
    }
    out << endl;

    out << "id: " << id << endl;
    out << "severity: ";
    switch (severity) {
    case GL_DEBUG_SEVERITY_LOW:
        out << "LOW";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        out << "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        out << "HIGH";
        break;
    }
    out << endl;
    out << "---------------------OPENGL-CALLBACK-END--------------" << endl;

    el::trace(out.str().c_str());
}

void EnableOpenGLDebug() {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    if (glDebugMessageCallback) {
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
        glDebugMessageCallback(openglCallback, nullptr);
    }
}

static void glfwErrorCallback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

namespace {
    // timing
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    std::shared_ptr<Application> application;
} // namespace 

void FrameStartUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

enum class EnumSample {
    kChapter06Sample01 = 0,
    kChapter09Sample01,
    kChapter10Sample01,
    kChapter10Sample02,
    kDecomposeSample,
    kXmlViewer,
};

std::vector<const char*> SampleName {
    "Chapter06/Sample01",
    "Chapter09/Sample01",
    "Chapter10/Sample01",
    "Chapter10/Sample02",
    "Sample/Decompose",
    "Sample/XmlViewer"
};

std::shared_ptr<Application> Factory(EnumSample kSample) 
{
    switch (kSample) {
    case EnumSample::kChapter06Sample01:
        return std::make_shared<Chapter06Sample01>();
    case EnumSample::kChapter09Sample01:
        return std::make_shared<Chapter09Sample01>();
    case EnumSample::kChapter10Sample01:
        return std::make_shared<Chapter10Sample01>();
    case EnumSample::kChapter10Sample02:
        return std::make_shared<Chapter10Sample02>();
    case EnumSample::kDecomposeSample:
        return std::make_shared<DecomposeSample>();
    case EnumSample::kXmlViewer:
        return std::make_shared<XmlViewer>();
    };
    return nullptr;
}

void UpdateMenuUI()
{
    static auto sampleList = std::vector<EnumSample>{
        EnumSample::kXmlViewer,
        EnumSample::kChapter06Sample01,
        EnumSample::kChapter09Sample01,
        EnumSample::kChapter10Sample01,
        EnumSample::kChapter10Sample02,
        EnumSample::kDecomposeSample,
    };
    std::vector<const char*> items;
    for (auto kSample : sampleList)
        items.push_back(SampleName[static_cast<size_t>(kSample)]);

    ImGui::Begin("Menu");

    // Simplified one-liner Combo() using an array of const char*
    static auto index = 0, prev = -1;
    ImGui::Combo("sample list", &index, items.data(), (int)items.size());
    ImGui::End();

    if (prev != index) {
        application = Factory(static_cast<EnumSample>(sampleList[index]));
        application->Initialize();
        prev = index;
    }
}

void UpdateStatUI(const Profile& profile, bool vsynch)
{
    const ImVec4 defaultColor = ImVec4(1.f, 1.f, 1.f, 1.f);
    const ImVec4 red = ImVec4(1.f, 0.f, 0.f, 1.f);

    ImGui::Begin("Display Stats");

    ImGui::Text("Display frequency: %d", profile.displayFrequency);;

    if (vsynch != 0) {
        ImGui::Text("VSynch: on");
    }
    else {
        ImGui::Text("VSynch: off");
    }
    ImGui::Text("Frame budget: %0.2f ms", profile.frameBudget);

    ImGui::Separator();

    ImVec4 color = defaultColor;
    if (profile.slowFrame) {
        color = red;
    }
    else {
        color = defaultColor;
    }

    ImGui::TextColored(color, "Frame Time: %0.5f ms", profile.display.frameTime);
    ImGui::TextColored(color, "Delta Time: %0.5f ms", profile.display.deltaTime);

    if (profile.slowFrame)
        color = defaultColor;

    ImGui::Separator();

    ImGui::Text("Game GPU: %0.5f ms", profile.display.appGPU);
    ImGui::Text("IMGUI GPU: %0.5f ms", profile.display.imguiGPU);
    ImGui::Separator();
    ImGui::Text("Win32 Events: %0.5f ms", profile.display.win32Events);
    ImGui::Text("Game Update: %0.5f ms", profile.display.frameUpdate);
    ImGui::Text("Game Render: %0.5f ms", profile.display.frameRender);
    ImGui::Text("IMGUI logic: %0.5f ms", profile.display.imguiLogic);
    ImGui::Text("IMGUI render: %0.5f ms", profile.display.imguiRender);
    ImGui::Text("Swap Buffers: %0.5f ms", profile.display.swapBuffer);
    ImGui::End();
}

void MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
}

void ScrollCallback(GLFWwindow* window, double, double yoffset) 
{
}

#define EVENT_CLASS_TYPE(type) virtual const char* GetName() const override { return #type; }

class Event
{
public:
    bool Handled = false;

    virtual const char* GetName() const = 0;
    virtual std::string ToString() const { return GetName(); }
};

using KeyCode = char;

class KeyEvent: public Event
{
public:
    inline KeyCode GetKeyCode() const { return m_KeyCode; }

protected:
    KeyEvent(KeyCode keycode)
        : m_KeyCode(keycode) {}

    KeyCode m_KeyCode;
};

class KeyPressedEvent: public KeyEvent
{
public:
    KeyPressedEvent(KeyCode keycode, int repeatCount)
        : KeyEvent(keycode), m_RepeatCount(repeatCount) {}

    inline int GetRepeatCount() const { return m_RepeatCount; }

    std::string ToString() const override
    {
        std::stringstream ss;
        ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";
        return ss.str();
    }
    EVENT_CLASS_TYPE(KeyPressed)

private:
    int m_RepeatCount;
};

class KeyReleasedEvent: public KeyEvent
{
public:
    KeyReleasedEvent(KeyCode keycode)
        : KeyEvent(keycode) {}

    std::string ToString() const override
    {
        std::stringstream ss;
        ss << "KeyReleasedEvent: " << m_KeyCode;
        return ss.str();
    }

    EVENT_CLASS_TYPE(KeyReleased)
};

class KeyTypedEvent: public KeyEvent
{
public:
    KeyTypedEvent(KeyCode keycode)
        : KeyEvent(keycode) {}

    std::string ToString() const override
    {
        std::stringstream ss;
        ss << "KeyTypedEvent: " << m_KeyCode;
        return ss.str();
    }

    EVENT_CLASS_TYPE(KeyTyped)
};

void KeyCallback(GLFWwindow*, int key, int, int action, int mods) 
{
}

void FrameEndUI()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#ifdef __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glslVersion = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1600, 1200, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return -1;

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    // set 0 to release frame limit
    int vsynch = 1;
    glfwSwapInterval(vsynch); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glslVersion);

    NVGcontext* vg = NULL;
    vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
    if (vg == NULL) {
        printf("Could not init nanovg.\n");
        return -1;
    }
    nvgCreateFont(vg, "sans", "Assets/Roboto-Regular.ttf");

    Profile profile(glfwGetWin32Window(window));

    EnableOpenGLDebug();

    initHUD();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint gVertexArrayObject = 0;
    glGenVertexArrays(1, &gVertexArrayObject);
    glBindVertexArray(gVertexArrayObject);

    while (!glfwWindowShouldClose(window)) {
        profile.EventMessageStart();
        glfwPollEvents();
        profile.EventMessageStop();

        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        profile.EventUpdateStart();
        if (application) {
            application->Update(deltaTime);
        }
        profile.EventUpdateStop();

        int displayWidth, displayHeight;
        glfwGetFramebufferSize(window, &displayWidth, &displayHeight);

        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glPointSize(5.0f);
        glBindVertexArray(gVertexArrayObject);

        glClearColor(0.5f, 0.6f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, displayWidth, displayHeight);

        static float aspect = 1.f;
        if (displayWidth != 0)
            aspect = (float)displayWidth / (float)displayHeight;

        profile.EventRenderStart();
        if (application) {
            application->Render(aspect);
        }
        profile.EventRenderStop();

        profile.EventUIUpdateStart();
        FrameStartUI();
        UpdateMenuUI();
        UpdateStatUI(profile, vsynch);
        if (application) {
            application->ImGui(nullptr);
            nvgBeginFrame(vg, (float)displayWidth, (float)displayHeight, 1.0);
            application->NanoGui(vg);
            nvgEndFrame(vg);
        }
        profile.EventUIUpdateStop();

        profile.EventUIRenderStart();
        FrameEndUI();
        profile.EventUIRenderStop();

        profile.EventSwapStart();
        glfwSwapBuffers(window);
        if (vsynch != 0) {
            glFinish();
        }
        profile.EventSwapStop();
    }

    application = nullptr;

    nvgDeleteGL3(vg);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &gVertexArrayObject);
    gVertexArrayObject = 0;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

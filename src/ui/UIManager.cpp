#include "poorcraft/ui/UIManager.h"

#include "poorcraft/core/Logger.h"
#include "poorcraft/window/Window.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <string>

namespace PoorCraft {

UIManager& UIManager::getInstance() {
    static UIManager instance;
    return instance;
}

void UIManager::initialize(Window& window) {
    if (m_Initialized) {
        PC_WARN("UIManager already initialized");
        return;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    GLFWwindow* nativeWindow = window.getNativeWindow();
    if (!nativeWindow) {
        PC_ERROR("UIManager initialization failed: native window handle is null");
        return;
    }

    if (!ImGui_ImplGlfw_InitForOpenGL(nativeWindow, true)) {
        PC_ERROR("Failed to initialize ImGui GLFW backend");
        return;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 460 core")) {
        PC_ERROR("Failed to initialize ImGui OpenGL3 backend");
        ImGui_ImplGlfw_Shutdown();
        return;
    }

    io.Fonts->AddFontDefault();

    m_Window = &window;
    m_Initialized = true;

    PC_INFO(std::string("UIManager initialized (ImGui ") + ImGui::GetVersion() + ")");
}

void UIManager::shutdown() {
    if (!m_Initialized) {
        return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    m_Window = nullptr;
    m_Initialized = false;

    PC_INFO("UIManager shutdown");
}

void UIManager::beginFrame() {
    if (!m_Initialized) {
        return;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UIManager::endFrame() {
    if (!m_Initialized) {
        return;
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool UIManager::wantCaptureMouse() const {
    if (!m_Initialized) {
        return false;
    }
    return ImGui::GetIO().WantCaptureMouse;
}

bool UIManager::wantCaptureKeyboard() const {
    if (!m_Initialized) {
        return false;
    }
    return ImGui::GetIO().WantCaptureKeyboard;
}

void UIManager::setMouseCursor(bool visible) {
    if (!m_Initialized || !m_Window) {
        return;
    }

    GLFWwindow* nativeWindow = m_Window->getNativeWindow();
    if (!nativeWindow) {
        return;
    }

    glfwSetInputMode(nativeWindow, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

} // namespace PoorCraft

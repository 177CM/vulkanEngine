#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <stdio.h>  // printf, fprintf
#include <stdlib.h> // abort
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "cjh_device.hpp"
#include "cjh_descriptors.hpp"
#include "cjh_renderer.hpp"

#ifdef _DEBUG
#define IMGUI_VULKAN_DEBUG_REPORT
#endif

namespace cjh
{

    static void check_vk_result(VkResult err)
    {
        if (err == 0)
            return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
            abort();
    }

    class CjhUI
    {
    private:
        CjhUI(const CjhUI &) = delete;
        CjhUI &operator=(const CjhUI &) = delete;
        // Our state
        bool show_demo_window = false;
        bool show_another_window = false;
        bool show_hello_world_window = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        CjhDevice *m_pDevice;
        CjhWindow *m_pWindow;
        CjhRenderer *m_pRenderer;

    public:
        void NewFrame();
        void Draw();
        CjhUI(CjhDevice *cjhDevice, CjhWindow *cjhWindow, CjhRenderer *cjhRenderer);
        ~CjhUI();
        void Init(VkDescriptorPool descriptorPool);

        // helper function

        inline bool *getShowDemoWindow() { return &show_demo_window; }
        inline bool *getShowAnotherWindow() { return &show_another_window; }
        inline bool *getShowHelloWorldWindow() { return &show_hello_world_window; }
    };
}
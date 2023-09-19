#include "cjh_window.hpp"

// std
#include <stdexcept>
#include <iostream>

namespace cjh
{

  CjhWindow::CjhWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name}
  {
    initWindow();
  }

  CjhWindow::~CjhWindow()
  {
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  void CjhWindow::initWindow()
  {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetDropCallback(window, dropCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
  }

  void CjhWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
  {
    if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to craete window surface");
    }
  }

  void CjhWindow::framebufferResizeCallback(GLFWwindow *window, int width, int height)
  {
    auto cjhWindow = reinterpret_cast<CjhWindow *>(glfwGetWindowUserPointer(window));
    cjhWindow->framebufferResized = true;
    cjhWindow->width = width;
    cjhWindow->height = height;
  }

  void CjhWindow::dropCallback(GLFWwindow *window, int count, const char **paths)
  {
    int i;
    for (i = 0; i < count; i++)
    {
      // std::cout << (std::string(paths[i])) << std::endl;
    }
  }

  void CjhWindow::cursorPositionCallback(GLFWwindow *window, double xpos, double ypos)
  {
    // std::cout << "(" << xpos << "," << ypos << ")" << std::endl;
  }

} // namespace lve

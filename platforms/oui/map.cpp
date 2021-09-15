#include <csignal>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <array>
#include <string>
#include "tangram.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "platform.h"
#include <memory>
#include "urlClient.h"
#include <direct.h>
#include <vector>
#include <cstdlib>
#include "gl/hardware.h"
#define DEFAULT "res/fonts/NotoSans-Regular.ttf"


// OuiPlatform is only a combined version of tangrams windows/linux/android/common platforms.

namespace Tangram {

void logMsg(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

class OuiPlatform: public Platform
{
public:
  OuiPlatform() : m_urlClient(std::make_unique<UrlClient>(UrlClient::Options{}))
  {
  }

  void shutdown() override
  {
    m_urlClient.reset();

    Platform::shutdown();
  }
  void requestRender() const override
  {
  }
  std::vector<FontSourceHandle> systemFontFallbacksHandle() const  override
  {
    std::vector<FontSourceHandle> handles;
    handles.emplace_back(Url(DEFAULT));
    return handles;
  }
  bool startUrlRequestImpl(const Url& _url, UrlRequestHandle _request, UrlRequestId& _id) override
  {
    auto onURLResponse = [this, _request](UrlResponse&& response) {
        onUrlResponse(_request, std::move(response));
    };
    _id = m_urlClient->addRequest(_url.string(), onURLResponse);
    return false;
  }
  void cancelUrlRequestImpl(UrlRequestId _id)
  {
    if (m_urlClient) {
      m_urlClient->cancelRequest(_id);
    }
  }
  protected:
    std::unique_ptr<UrlClient> m_urlClient;
};

void setCurrentThreadPriority(int priority)
{   
}

void initGLExtensions() {
    Tangram::Hardware::supportsMapBuffer = true;
}

}

std::vector<Tangram::SceneUpdate> sceneUpdates;
void loadSceneFile(Tangram::Map* map, bool setPosition, std::vector<Tangram::SceneUpdate> updates) 
{
    Tangram::Url baseUrl("file:///");
    char pathBuffer[512] = {0};
    if (_getcwd(pathBuffer, 512) != nullptr) {
        baseUrl = Tangram::Url::fromWindowsFilePath(std::string(pathBuffer) + "/");
    }
    auto sceneUrl = baseUrl.resolve(Tangram::Url("res/raster-simple.yaml")).string();
    Tangram::logMsg(sceneUrl.c_str());
    map->loadScene(sceneUrl, setPosition, sceneUpdates);
}
  
#define OUI_EXPORT extern "C" __declspec(dllexport)

OUI_EXPORT GLFWwindow* init_oui_map()
{
  glfwInit();
  auto map = new Tangram::Map(std::make_unique<Tangram::OuiPlatform>());
  glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  auto win = glfwCreateWindow(100, 100, "", nullptr, nullptr);
  glfwMakeContextCurrent(win);
  glfwSetWindowUserPointer(win, static_cast<void*>(map));
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  map->setupGL();
  std::cerr << "ouimap:" << (int)glfwGetWin32Window(win) << "\n";
  return win;
}


OUI_EXPORT void oui_render_map(GLFWwindow *window, float delta) {
  auto map = static_cast<Tangram::Map*>(glfwGetWindowUserPointer(window));
  glfwMakeContextCurrent(window);
  Tangram::MapState state = map->update(delta);
  if (state.isAnimating()) {
       map->getPlatform().requestRender();
  }
   map->render();
}

OUI_EXPORT void resize_oui_map(GLFWwindow *window, float width, float height)
{
  auto map = static_cast<Tangram::Map*>(glfwGetWindowUserPointer(window));
  map->setPixelScale(1.0);
  map->resize(width, height);
}

OUI_EXPORT void run_oui_map(GLFWwindow *window)
{
  auto map = static_cast<Tangram::Map*>(glfwGetWindowUserPointer(window));
  loadSceneFile(map, true, sceneUpdates);
  while (glfwWindowShouldClose(window) == false)
  {
    oui_render_map(window, 1.0);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

OUI_EXPORT void parent_oui_map(GLFWwindow *child, HWND parent)
{
  SetWindowLong(glfwGetWin32Window(child), WS_CHILD, 1);
  UpdateWindow(SetParent(glfwGetWin32Window(child), parent));
}

int main(int argc, char* argv[])
{
  auto map = init_oui_map();
  resize_oui_map(map, 100, 100);
  parent_oui_map(map, (HWND)(atoi(argv[1])));
  run_oui_map(map);
  return 0;
}
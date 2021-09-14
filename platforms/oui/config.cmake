if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-gnu-zero-variadic-macro-arguments")
endif()

check_unsupported_compiler_version()

add_definitions(-DTANGRAM_WINDOWS)
add_definitions(-DTANGRAM_OUI)

find_package(OpenGL REQUIRED)
find_package(CURL REQUIRED)

include(cmake/glfw.cmake)

add_executable(ouimap
  platforms/common/platform_gl.cpp
  platforms/common/urlClient.cpp
  platforms/oui/map.cpp
  platforms/common/glfw/deps/glad.c
)

add_resources(ouimap "${PROJECT_SOURCE_DIR}/scenes" "res")

target_include_directories(ouimap
  PRIVATE
  platforms/common
  platforms/common/glfw/deps
  ${CURL_INCLUDE_DIRS}
)

target_link_libraries(ouimap
  PRIVATE
  tangram-core
  glfw
  ${GLFW_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${CURL_LIBRARIES}
  wsock32 ws2_32 crypt32 wldap32
)
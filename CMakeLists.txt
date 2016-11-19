cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)

project(OpenGL-Tutorial)

set(GLTUT_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")

find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_executable(opengl-tut
  src/main.cpp
  src/fragment-shader.glsl
  src/vertex-shader.glsl
)

set_property(TARGET opengl-tut PROPERTY CXX_STANDARD 14)

add_custom_command(TARGET opengl-tut
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${GLTUT_SRC_DIR}/fragment-shader.glsl" "$<TARGET_FILE_DIR:opengl-tut>"
  COMMAND ${CMAKE_COMMAND} -E copy "${GLTUT_SRC_DIR}/vertex-shader.glsl" "$<TARGET_FILE_DIR:opengl-tut>"
  COMMAND ${CMAKE_COMMAND} -E copy "${GLTUT_SRC_DIR}/sample.png" "$<TARGET_FILE_DIR:opengl-tut>"
  COMMAND ${CMAKE_COMMAND} -E copy "${GLTUT_SRC_DIR}/sample2.png" "$<TARGET_FILE_DIR:opengl-tut>"
)

target_compile_definitions(opengl-tut PUBLIC GLEW_STATIC)

target_compile_features(opengl-tut PUBLIC cxx_range_for)
target_compile_features(opengl-tut PUBLIC cxx_auto_type)
target_compile_features(opengl-tut PUBLIC cxx_lambdas)

target_include_directories(opengl-tut PRIVATE ${GLEW_INCLUDE_DIR})
target_include_directories(opengl-tut PRIVATE ${OPENGL_INCLUDE_DIR})
target_include_directories(opengl-tut PRIVATE ${SDL_INCLUDE_DIR})
target_include_directories(opengl-tut PRIVATE ${SDL_IMAGE_INCLUDE_DIR})

target_link_libraries(opengl-tut ${GLEW_LIBRARY_RELEASE})
target_link_libraries(opengl-tut ${OPENGL_gl_LIBRARY})
target_link_libraries(opengl-tut ${SDL_LIBRARY})
target_link_libraries(opengl-tut ${SDL_IMAGE_LIBRARY})


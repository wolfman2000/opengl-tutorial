#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include <memory>
#include <iostream>
#include <fstream>

struct SDL_Deleter {
  void operator()(SDL_Window* ptr) {
    if (ptr) {
      SDL_DestroyWindow(ptr);
    }
  }

  void operator()(SDL_GLContext ptr) {
    if (ptr) {
      SDL_GL_DeleteContext(ptr);
    }
  }
};

char * readShader(std::string path) {
  std::ifstream t;
  int length;
  t.open(path);
  t.seekg(0, std::ios::end);
  length = t.tellg();
  t.seekg(0, std::ios::beg);
  char *buffer = new char[length];
  t.read(buffer, length);
  t.close();
  return buffer;
}

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO);

  auto *base_path = SDL_GetBasePath();

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  auto *window = new std::unique_ptr<SDL_Window, SDL_Deleter>(
    SDL_CreateWindow("OpenGL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                     640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN)
  );

  auto context = SDL_GL_CreateContext(window->get());

  glewExperimental = GL_TRUE;
  glewInit();

  // set up the vertices. Each vertex is in (X, Y) format.
  float vertices[] = {
    0.f, 0.5f,
    0.5f, -0.5f,
    -0.5f, -0.5f
  };

  // Set up the vertex array object for storing vbo references.
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // The vertices must be loaded into a vertex buffer object.
  // Load it here.
  GLuint vbo;
  glGenBuffers(1, &vbo);

  // make the VBO active.
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  // copy the vertex data over.
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
  auto *vertexSource = readShader(std::string() + base_path + "vertex-shader.glsl");
  glShaderSource(vertexShader, 1, &vertexSource, nullptr);
  glCompileShader(vertexShader);

  auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  auto *fragmentSource = readShader(std::string() + base_path + "fragment-shader.glsl");
  glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
  glCompileShader(fragmentShader);

  // create a shader program and join the two.
  auto shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);

  // In case we end up with multiple buffers, good to set up.
  glBindFragDataLocation(shaderProgram, 0, "outColor");

  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);

  // set up the position.
  auto posAttrib = glGetAttribLocation(shaderProgram, "position");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

  auto uniColor = glGetUniformLocation(shaderProgram, "triangleColor");
  glUniform3f(uniColor, 1.f, 0.f, 0.f);

  auto t_start = std::chrono::high_resolution_clock::now();
  SDL_Event windowEvent;

  for (;;) {
    if (SDL_PollEvent(&windowEvent)) {
      if (windowEvent.type == SDL_QUIT) {
        break;
      }
      if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE) {
        break;
      }
    }

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    auto t_now = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

    glUniform3f(uniColor, (sin(time * 4.f) + 1.f) / 2.f, 0.f, 0.f);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    SDL_GL_SwapWindow(window->get());
  }

  SDL_GL_DeleteContext(context);

  SDL_free(base_path);

  SDL_Quit();

  return 0;
}

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>

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

  void operator()(SDL_Surface* ptr) {
    if (ptr) {
      SDL_FreeSurface(ptr);
    }
  }
};

std::string readShader(std::string path) {
  std::ifstream t(path);
  if (!t) {
    return "";
  }
  std::ostringstream stream;
  stream << t.rdbuf();
  return stream.str();
}

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO);
  IMG_Init(IMG_INIT_PNG);

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

  // set up the vertices. Each vertex is in (X, Y, R, G, B, double tex coords) format.
  float vertices[] = {
    -0.5f,  0.5f, 1.f, 0.f, 0.f, 0.f, 0.f,
     0.5f,  0.5f, 0.f, 1.f, 0.f, 1.f, 0.f,
     0.5f, -0.5f, 0.f, 0.f, 1.f, 1.f, 1.f,
    -0.5f, -0.5f, 1.f, 1.f, 1.f, 0.f, 1.f
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

  // Set up an element buffer to preserve vertex points in memory.
  GLuint elements[] = {
    0, 1, 2,
    2, 3, 0
  };

  // And the former element buffer object. This must happen AFTER the vertices!
  GLuint ebo;
  glGenBuffers(1, &ebo);

  // load the vertex data into the element buffer.
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

  auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
  auto vertexCode = readShader(std::string() + base_path + "vertex-shader.glsl");
  auto vertexSource = vertexCode.c_str();
  glShaderSource(vertexShader, 1, &vertexSource, nullptr);
  glCompileShader(vertexShader);

  GLint status;
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE) {
    char buffer[512];
    glGetShaderInfoLog(vertexShader, 512, nullptr, buffer);
    std::cout << buffer << std::endl;
  }

  auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  auto fragmentCode = readShader(std::string() + base_path + "fragment-shader.glsl");
  auto fragmentSource = fragmentCode.c_str();
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
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE,
                        7 * sizeof(float), 0);

  auto colAttrib = glGetAttribLocation(shaderProgram, "color");
  glEnableVertexAttribArray(colAttrib);
  glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE,
                        7 * sizeof(float), (void *)(2 * sizeof(float)));

  auto texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
  glEnableVertexAttribArray(texAttrib);
  glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE,
                        7 * sizeof(float), (void *)(5 * sizeof(float)));

  // Set up a collection of texture buffer objects.
  GLuint textures[2];
  glGenTextures(2, textures);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textures[0]);

  // Load the image with the help of SDL.
  auto origCat = new std::unique_ptr<SDL_Surface, SDL_Deleter>(IMG_Load("sample.png"));
  auto fixedCat = new std::unique_ptr<SDL_Surface, SDL_Deleter>(SDL_ConvertSurfaceFormat(origCat->get(), SDL_PIXELFORMAT_RGBA32, 0));

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
               fixedCat->get()->w, fixedCat->get()->h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               fixedCat->get()->pixels);
  glUniform1i(glGetUniformLocation(shaderProgram, "texKitten"), 0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  auto origDog = new std::unique_ptr<SDL_Surface, SDL_Deleter>(IMG_Load("sample2.png"));
  auto fixedDog = new std::unique_ptr<SDL_Surface, SDL_Deleter>(SDL_ConvertSurfaceFormat(origDog->get(), SDL_PIXELFORMAT_RGBA32, 0));

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, textures[1]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
               fixedDog->get()->w, fixedDog->get()->h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               fixedDog->get()->pixels);
  glUniform1i(glGetUniformLocation(shaderProgram, "texPuppy"), 1);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    SDL_GL_SwapWindow(window->get());
  }

  glDeleteTextures(2, textures);

  glDeleteProgram(shaderProgram);
  glDeleteShader(fragmentShader);
  glDeleteShader(vertexShader);

  glDeleteBuffers(1, &ebo);
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);

  SDL_GL_DeleteContext(context);

  SDL_free(base_path);

  SDL_Quit();

  return 0;
}

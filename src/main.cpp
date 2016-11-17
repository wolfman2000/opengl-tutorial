#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include <memory>

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

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO);

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

    SDL_GL_SwapWindow(window->get());
  }

  SDL_GL_DeleteContext(context);

  SDL_Quit();

  return 0;
}

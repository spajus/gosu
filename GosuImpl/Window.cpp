#include <Gosu/Gosu.hpp>
#include <SDL.h>
#include <cstdlib>
#include <memory>

using namespace std::tr1::placeholders;

namespace Gosu
{
    namespace FPS
    {
        void registerFrame();
    }
    
    SDL_DisplayMode desktopDisplayMode = { 0, 0 };
}

unsigned Gosu::screenWidth()
{
    // TODO - not thread-safe
    if (desktopDisplayMode.w == 0) {
        SDL_GetDesktopDisplayMode(0, &desktopDisplayMode);
    }
    return desktopDisplayMode.w;
}

unsigned Gosu::screenHeight()
{
    // TODO - not thread-safe
    if (desktopDisplayMode.h == 0) {
        SDL_GetDesktopDisplayMode(0, &desktopDisplayMode);
    }
    return desktopDisplayMode.h;
}

struct Gosu::Window::Impl
{
    SDL_Window *window;
    SDL_GLContext context;
    double updateInterval;
    
    std::auto_ptr<Graphics> graphics;
    std::auto_ptr<Input> input;
};

Gosu::Window::Window(unsigned width, unsigned height, bool fullscreen, double updateInterval)
: pimpl(new Impl)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::runtime_error("Failed to initialize SDL Video");
    
    unsigned actualWidth = width;
    unsigned actualHeight = height;
    
    if (fullscreen) {
        actualWidth = Gosu::screenWidth();
        actualHeight = Gosu::screenHeight();
    }
    
    pimpl->window = SDL_CreateWindow("",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        actualWidth, actualHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI |
            (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
    pimpl->context = SDL_GL_CreateContext(pimpl->window);
    SDL_GL_MakeCurrent(pimpl->window, pimpl->context);
    SDL_GL_SetSwapInterval(1);
    
    pimpl->graphics.reset(new Graphics(actualWidth, actualHeight, fullscreen));
    pimpl->graphics->setResolution(width, height);
    pimpl->input.reset(new Input());
    input().onButtonDown = std::tr1::bind(&Window::buttonDown, this, _1);
    input().onButtonUp = std::tr1::bind(&Window::buttonUp, this, _1);
    pimpl->updateInterval = updateInterval;
}

Gosu::Window::~Window()
{
    SDL_GL_DeleteContext(pimpl->context);
    SDL_DestroyWindow(pimpl->window);
    
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

std::wstring Gosu::Window::caption() const
{
    return utf8ToWstring(SDL_GetWindowTitle(pimpl->window));
}

void Gosu::Window::setCaption(const std::wstring& caption)
{
    std::string utf8 = wstringToUTF8(caption);
    SDL_SetWindowTitle(pimpl->window, utf8.c_str());
}

double Gosu::Window::updateInterval() const
{
    return pimpl->updateInterval;
}

void Gosu::Window::show()
{
    while (true) {
        auto startTime = milliseconds();
        
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                return;
            else
                input().feedSDLEvent(&e);
		}
        
        input().update();
        
        update();
        
        if (graphics().begin()) {
            draw();
            graphics().end();
        }
	    
        SDL_GL_SwapWindow(pimpl->window);
        
        // Sleep to keep this loop from eating 100% CPU.
        auto frameTime = milliseconds() - startTime;
        if (frameTime > 0 && frameTime < pimpl->updateInterval) {
            sleep(pimpl->updateInterval - frameTime);
        }
    }
}

void Gosu::Window::close()
{
    SDL_Event e;
    e.type = SDL_QUIT;
    SDL_PushEvent(&e);
}

const Gosu::Graphics& Gosu::Window::graphics() const
{
    return *pimpl->graphics;
}

Gosu::Graphics& Gosu::Window::graphics()
{
    return *pimpl->graphics;
}

const Gosu::Input& Gosu::Window::input() const
{
    return *pimpl->input;
}

Gosu::Input& Gosu::Window::input()
{
    return *pimpl->input;
}

// Deprecated.

class Gosu::Audio {};
namespace { Gosu::Audio dummyAudio; }

const Gosu::Audio& Gosu::Window::audio() const
{
    return dummyAudio;
}

Gosu::Audio& Gosu::Window::audio()
{
    return dummyAudio;
}

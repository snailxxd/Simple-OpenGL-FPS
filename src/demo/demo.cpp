#include "render_engine/OpenGLRenderEngine.h"
#include "application/FPSGame.h"

int main() {
    OpenGLRenderEngine engine;
    FPSGame game(engine);
    game.Run();
    return 0;
}

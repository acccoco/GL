#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>

#include "config.hpp"
#include "core/engine.h"
#include "core/model.h"

#include "./shader.h"

class TestEngine: public Engine {

};

int main() {
    auto engine = TestEngine();
    engine.engine_main();
    return 0;
}
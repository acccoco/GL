#pragma once

#include <string>
#include <fmt/format.h>


const std::string SOURCE = "${CMAKE_SOURCE_DIR}/";

const std::string EXAMPLES = SOURCE + "examples/";
const std::string SHADER   = SOURCE + "shader/";
const std::string TEXTURE  = SOURCE + "texture/";

// model
const char *MODEL_SPHERE_MATRIX = "${GL_MODEL_DIR}/sphere-matrix/sphere-matrix.obj";
const char *MODEL_202_CHAN      = "${GL_MODEL_DIR}/marry/Marry.obj";
const char *MODEL_THREE_OBJS    = "${GL_MODEL_DIR}/three-objs/cube-sphere-cylinder.obj";
const char *MODEL_DIONA         = "${GL_MODEL_DIR}/diona/diona.obj";
const char *MODEL_CUBE          = "${GL_MODEL_DIR}/cube/cube.obj";
const char *MODEL_SQUARE        = "${GL_MODEL_DIR}/square/square.obj";
const char *MODEL_LIGHT         = "${GL_MODEL_DIR}/light/light.obj";
const char *MODLE_FLOOR         = "${GL_MODEL_DIR}/floor/floor.obj";
const char *MODEL_GRAY_FLOOR    = "${GL_MODEL_DIR}/gray-floor/gray-floor.obj";
const char *MODEL_SPHERE        = "${GL_MODEL_DIR}/sphere/sphere.obj";
const char *MODEL_BUNNY         = "${GL_MODEL_DIR}/bunny/bunny.obj";
const char *MODEL_CORNER        = "${GL_MODEL_DIR}/corner/corner.obj";
const char *MODEL_LUCY          = "${GL_MODEL_DIR}/lucy/lucy.obj";


// texture
const std::string TEX_FACE         = TEXTURE + "awesomeface.jpg";
const std::string TEX_FLOOR        = TEXTURE + "floor.jpg";
const std::string TEX_SKY          = TEXTURE + "cube-sky-1/";
const std::string CUBE_CORNELL     = TEXTURE + "cube-cornel-box/";
const std::string CUBE_INDOOR      = TEXTURE + "cube-indoor/";
const std::string CUBE_CATHEDRAL   = TEXTURE + "cube-grace-cathedral/";
const std::string CUBE_SKY2        = TEXTURE + "cube-sky-2/";
const char       *TEXTURE_PBR_BALL = "${GL_TEXTURE_DIR}/pbr-ball/";

#pragma once

// antomaticly generated by cmake, don't modify this file
// 这个文件会被安装到每个 example 中
#include <string>


const std::string TEXTURE          = "${RT_TEXTURE_DIR}/";
const std::string MODEL            = "${RT_MODEL_DIR}/";
const std::string EXAMPLE_CUR_PATH = "${EXAMPLE_CUR_PATH}/";


// model
const char *MODEL_SPHERE_MATRIX = "${RT_MODEL_DIR}/sphere-matrix/sphere-matrix.obj";
const char *MODEL_202_CHAN      = "${RT_MODEL_DIR}/marry/Marry.obj";
const char *MODEL_THREE_OBJS    = "${RT_MODEL_DIR}/three-objs/cube-sphere-cylinder.obj";
const char *MODEL_DIONA         = "${RT_MODEL_DIR}/diona/diona.obj";
const char *MODEL_CUBE          = "${RT_MODEL_DIR}/cube/cube.obj";
const char *MODEL_SQUARE        = "${RT_MODEL_DIR}/square/square.obj";
const char *MODEL_LIGHT         = "${RT_MODEL_DIR}/light/light.obj";
const char *MODLE_FLOOR         = "${RT_MODEL_DIR}/floor/floor.obj";
const char *MODEL_GRAY_FLOOR    = "${RT_MODEL_DIR}/gray-floor/gray-floor.obj";
const char *MODEL_SPHERE        = "${RT_MODEL_DIR}/sphere/sphere.obj";
const char *MODEL_BUNNY         = "${RT_MODEL_DIR}/bunny/bunny.obj";
const char *MODEL_CORNER        = "${RT_MODEL_DIR}/corner/corner.obj";
const char *MODEL_LUCY          = "${RT_MODEL_DIR}/lucy/lucy.obj";
const char *MODEL_CORNELL_BOX   = "${RT_MODEL_DIR}/cornel-box/cornel-box.obj";


// texture
const std::string TEX_FACE         = TEXTURE + "awesomeface.jpg";
const std::string TEX_FLOOR        = TEXTURE + "floor.jpg";
const std::string TEX_SKY          = TEXTURE + "cube-sky-1/";
const std::string CUBE_CORNELL     = TEXTURE + "cube-cornel-box/";
const std::string CUBE_INDOOR      = TEXTURE + "cube-indoor/";
const std::string CUBE_CATHEDRAL   = TEXTURE + "cube-grace-cathedral/";
const std::string CUBE_SKY2        = TEXTURE + "cube-sky-2/";
const char       *TEXTURE_PBR_BALL = "${RT_TEXTURE_DIR}/pbr-ball/";
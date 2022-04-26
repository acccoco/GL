#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


const glm::vec3 POSITIVE_X = {1, 0, 0};
const glm::vec3 NEGATIVE_X = {-1, 0, 0};
const glm::vec3 POSITIVE_Y = {0, 1, 0};
const glm::vec3 NEGATIVE_Y = {0, -1, 0};
const glm::vec3 POSITIVE_Z = {0, 0, 1};
const glm::vec3 NEGATIVE_Z = {0, 0, -1};


inline void glViewport_(GLint x, GLint y, GLsizei width, GLsizei height)
{
#ifdef __APPLE__
    glViewport(x * 2, y * 2, width * 2, height * 2);
#else
    glViewport(x, y, width, height);
#endif
}

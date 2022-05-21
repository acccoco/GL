#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

struct PointLight
{
    glm::vec3 pos;
    glm::vec3 color;
};


struct DirectionalLight {
    glm::vec3 pos;
    glm::vec3 dir;
    glm::vec3 color;
};
#include "../rt-object.h"


void RTObjectBase::set_pos(const glm::vec3 &pos)
{
    _position = pos;
    update_matrix();
}


void RTObjectBase::set_matrix(const glm::mat4 &matrix)
{
    _matrix = matrix;

    _position = glm::vec3(matrix[3]);

    const glm::mat3 RS{matrix};    // 旋转和缩放的矩阵
    _scale = {glm::length(RS[0]), glm::length(RS[1]), glm::length(RS[2])};
    if (_scale.x == 0 || _scale.y == 0 || _scale.z == 0)
        LOG_AND_THROW("scale is invalid: ({}, {}, {})", _scale.x, _scale.y, _scale.z);

    const glm::mat3 R = {RS[0] / _scale.x, RS[1] / _scale.y, RS[2] / _scale.z};
    _rotate           = glm::quat_cast(R);
}


void RTObjectBase::update_matrix()
{
    _matrix = glm::translate(glm::mat4(1.f), _position) * glm::mat4_cast(_rotate) *
              glm::scale(glm::mat4(1.f), _scale);
}
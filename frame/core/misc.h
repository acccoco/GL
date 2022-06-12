#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>


/**
 * 使用 spdlog 打印错误日志，并且抛出异常
 */
#define LOG_AND_THROW(str, args...)                                                                \
    do                                                                                             \
    {                                                                                              \
        SPDLOG_ERROR(str, ##args);                                                                 \
        throw std::exception();                                                                    \
    } while (false)


const glm::vec3 POSITIVE_X = {1, 0, 0};
const glm::vec3 NEGATIVE_X = {-1, 0, 0};
const glm::vec3 POSITIVE_Y = {0, 1, 0};
const glm::vec3 NEGATIVE_Y = {0, -1, 0};
const glm::vec3 POSITIVE_Z = {0, 0, 1};
const glm::vec3 NEGATIVE_Z = {0, 0, -1};



/**
 * 合并两个 vector
 */
template<class T>
inline void combine(std::vector<T> &a, const std::vector<T> &b)
{
    a.insert(a.end(), b.begin(), b.end());
}


/**
 * 元素是否在列表中
 */
template<typename T>
inline bool is_one_of(T a, const std::vector<T> &list)
{
    for (const T &b: list)
        if (a == b)
            return true;
    return false;
}

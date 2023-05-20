#pragma once

#include <tuple>
#include <iostream>
#include "glm/matrix.hpp"

std::ostream& operator<<(std::ostream &stream, glm::vec3 v);
std::ostream& operator<<(std::ostream &stream, glm::vec2 v);

glm::vec2 perp(const glm::vec2& v);

// p, q positions, v, w directions
std::pair<float, float> lineIntersect(glm::vec2 p, glm::vec2 v, glm::vec2 q, glm::vec2 w);

// Math helpers
#pragma once

#include <vector>
#include <stdint.h>
#include <SFML/System/Vector2.hpp>

namespace math 
{

using point2d = sf::Vector2<double>;

// Bezier curve
class BCurve {
public:

    // constructs a Bezier curve of the specific order
    BCurve(uint32_t order);

    // evaluates bezier point for a parameter value t, 0 <= t <= 1
    point2d operator()(double t) const;

    // sets the array of control points for the curve
    void set_control_points(const std::vector<point2d>& p);

private:
    // computes an array of binomial coefficients of the order n
    std::vector<uint64_t> compute_coefficients(uint32_t n) const;

    // computes a binomial coefficient B(n, k)
    uint64_t binomial_coefficient(uint32_t n, uint32_t k) const;

    // computes the value of the Bernstein polynomial for a parameter t
    double basis_polynomial(uint32_t k, double t) const;

    // binomial coefficients
    const std::vector<uint64_t> b_coeff;

    // set of control points of the curve
    std::vector<point2d> points;
};

}
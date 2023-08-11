
#include "math.hpp"
#include <cmath>
#include <stdexcept>

namespace math
{

BCurve::BCurve(uint32_t order)
    : b_coeff(compute_coefficients(order)) {}

void BCurve::set_control_points(const std::vector<point2d>& p) {
    if(p.size() != b_coeff.size())
        throw std::runtime_error("invalid argument");

    points = p;
}

point2d BCurve::operator()(double t) const {
    point2d sum = { 0.0, 0.0 };
    
    for(uint32_t k = 0; k < points.size(); ++k) {
        sum.x += basis_polynomial(k, t) * points[k].x;
        sum.y += basis_polynomial(k, t) * points[k].y;
    }

    return sum;
}

std::vector<uint64_t> BCurve::compute_coefficients(uint32_t n) const {
    std::vector<uint64_t> cf(n+1);

    for( uint32_t k = 0; k <= n; ++k ) {
        cf[k] = binomial_coefficient(n, k);
    }

    return cf;
}

uint64_t BCurve::binomial_coefficient(uint32_t n, uint32_t k) const {
    if (k > n)
        return 0;

    if (k == 0 || k == n)
        return 1;

    k = std::min(k, n - k); // Take advantage of symmetry
    uint64_t c = 1;

    for ( uint32_t i = 0; i < k; ++i) {
        c = c * (n - i) / (i + 1);
    }

    return c;
}

double BCurve::basis_polynomial(uint32_t k, double t) const {
    // TODO: use the recurrent formula
    const size_t n = b_coeff.size() - 1;
    return b_coeff[k] * pow(t, k) * pow(1.0 - t, n - k);
}

}
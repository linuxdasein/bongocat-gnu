#include "cats.hpp"
#include <stdexcept>

namespace cats
{

std::unique_ptr<ICat> get_cat(int mode)
{
    switch (mode) {
    case 2:
        return std::make_unique<TaikoCat>();
    case 3:
        return std::make_unique<CtbCat>();
    default:
        throw std::runtime_error("Invalid mode value has been encountered");
    }
}

}

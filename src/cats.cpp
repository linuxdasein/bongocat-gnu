#include "cats.hpp"
#include <stdexcept>

namespace cats
{

std::unique_ptr<ICat> get_cat(int mode)
{
    switch (mode) {
    case 1:
        return std::make_unique<OsuCat>();
    case 2:
        return std::make_unique<TaikoCat>();
    case 3:
        return std::make_unique<CtbCat>();
    case 4:
        return std::make_unique<ManiaCat>();
    default:
        throw std::runtime_error("Invalid mode value has been encountered");
    }
}

}

#include "header.hpp"

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
    case 5:
        return std::make_unique<CustomCat>();
    default:
        data::error_msg("Mode value is not correct", "Error reading configs");
        return nullptr;
    }
}

}

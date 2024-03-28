#include "header.hpp"

namespace cats
{

std::unique_ptr<ICat> get_cat(cats::CatModeId mode)
{
    switch (mode) {
    case CatModeId::osu:
        return std::make_unique<OsuCat>();
    case CatModeId::taiko:
        return std::make_unique<TaikoCat>();
    case CatModeId::ctb:
        return std::make_unique<CtbCat>();
    case CatModeId::mania:
        return std::make_unique<ManiaCat>();
    case CatModeId::custom:
        return std::make_unique<CustomCat>();
    default:
        logger::error("Fatal Error! Cat mode with the requested id not found");
        return nullptr;
    }
}

}

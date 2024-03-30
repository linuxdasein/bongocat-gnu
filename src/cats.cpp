#include "header.hpp"

namespace cats
{

std::unique_ptr<ICat> get_cat(cats::CatModeId mode)
{
    switch (mode) {
    case CatModeId::custom:
        return std::make_unique<CustomCat>();
    default:
        logger::error("Fatal Error! Cat mode with the requested id not found");
        return nullptr;
    }
}

}

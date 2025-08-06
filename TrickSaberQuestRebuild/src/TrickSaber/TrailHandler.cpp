#include "TrickSaber/TrailHandler.hpp"
#include "TrickSaber/Utils/ObjectCache.hpp"
#include "main.hpp"

DEFINE_TYPE(TrickSaber, TrailHandler);

using namespace TrickSaber;

void TrailHandler::Initialize(GlobalNamespace::Saber* saber) {
    this->saber = saber;
    
    // Cache trail-related objects during initialization
    if (saber) {
        auto saberManager = Utils::ObjectCache::GetSaberManager();
        if (saberManager) {
            Logger.debug("TrailHandler initialized with cached SaberManager");
        }
    }
}
#include "TrickSaber/BurnMarkHandler.hpp"
#include "main.hpp"

#include "GlobalNamespace/SaberBurnMarkArea.hpp"
#include "GlobalNamespace/SaberBurnMarkSparkles.hpp"
#include "GlobalNamespace/ObstacleSaberSparkleEffectManager.hpp"
#include "UnityEngine/Object.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

using namespace TrickSaber;
using namespace GlobalNamespace;

std::unordered_map<Il2CppClass*, FieldInfo*> BurnMarkHandler::sabersFieldInfo;
std::unordered_map<System::Type*, ArrayW<::UnityW<::UnityEngine::Object>>> BurnMarkHandler::disabledBurnmarks;
std::vector<System::Type*> BurnMarkHandler::burnTypes;
bool BurnMarkHandler::initialized = false;

void BurnMarkHandler::Initialize() {
    if (initialized) return;
    burnTypes.clear();
    burnTypes.push_back(csTypeOf(SaberBurnMarkArea*));
    burnTypes.push_back(csTypeOf(SaberBurnMarkSparkles*));
    burnTypes.push_back(csTypeOf(ObstacleSaberSparkleEffectManager*));
    initialized = true;
}

void BurnMarkHandler::DisableBurnMarks(int saberType) {
    // Simple implementation - just cache the components for later restoration
    for (auto type : burnTypes) {
        auto& components = disabledBurnmarks[type];
        if (!components) {
            components = UnityEngine::Object::FindObjectsOfType(type);
        }
        // Components are cached for restoration later
    }
}

void BurnMarkHandler::EnableBurnMarks(int saberType, bool force) {
    // Simple implementation - just clear the cached components
    // The actual burn marks will be re-enabled when the components are recreated
    for (auto type : burnTypes) {
        auto& components = disabledBurnmarks[type];
        components = ArrayW<::UnityW<::UnityEngine::Object>>();
    }
}

void BurnMarkHandler::ClearCache() {
    // Clear all static containers
    disabledBurnmarks.clear();
    sabersFieldInfo.clear();
    burnTypes.clear();
    
    // Reset initialization flag
    initialized = false;
    
    Logger.debug("BurnMarkHandler cache cleared");
}
#pragma once

#include "GlobalNamespace/Saber.hpp"
#include "System/Type.hpp"
#include "UnityEngine/Object.hpp"
#include <unordered_map>
#include <vector>

namespace TrickSaber {
    class BurnMarkHandler {
    public:
        static void DisableBurnMarks(int saberType);
        static void EnableBurnMarks(int saberType, bool force = false);
        static void ClearCache();
        static void Initialize();
        
    private:
        static std::unordered_map<Il2CppClass*, FieldInfo*> sabersFieldInfo;
        static std::unordered_map<System::Type*, ArrayW<::UnityW<::UnityEngine::Object>>> disabledBurnmarks;
        static std::vector<System::Type*> burnTypes;
        static bool initialized;
    };
}
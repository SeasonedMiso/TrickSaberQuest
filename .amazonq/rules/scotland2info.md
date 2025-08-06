# scotland2

A Quest/Android specific modloader for Beat Saber.

## Features

- "something regarding scene changes similar to how bsipa has on enable and whatnot"
- plain check for other mods being installed
- unload method except maybe too crashy
- on enable / on unity init method
- maybe do something with registration instead of extern c stuff?
- [thing](https://discord.com/channels/629851957439365131/629851957439365135/975450045585440788)

## Installation/Usage

This is a [LibProxyMain](https://github.com/sc2ad/LibMainLoader) compatible modloader. Thus, it should be placed at: `/sdcard/ModData/<APP ID>/Modloader/libsl2.so`.
This modloader has several call in points. It performs a topological sort of all depenents that need to be loaded and loads them in turn. `libs` are loaded very early, on `modloader_load`, while `early_mods` are also constructed at this time. `early_mods` have their `load` function called on them after `il2cpp_init`, which is hooked via [Flamingo](https://github.com/sc2ad/Flamingo) to allow us to perform loading at a sufficiently late time. early mods will also have a function named `late_load` called when mods are constructed, but more on that in the next paragraph.

`mods` will be constructed when the libunity.so method `DestroyObjectHighLevel` is called the first time. This method is what destroys unity objects, and it happens that the first destroyed object is within the first scene load call. This is the perfect timing for allowing creation of GameObjects and other unity assets at dlopen time, much like how it would happen on PC. This method is hooked with flamingo through an xref trace from the DestroyImmediate icall for gameobjects, through to the internal method `DestroyObjectHighLevel`. Here early mods get their late_load method called, after which mods will get their late_load method called.

This way of initializing at unity init is inspired by what BSIPA does on pc, where a gameobject is created and destroyed, and when its OnDestroy happens things are loaded in.

Here is a table containing what gets opened and called when.
 - `dlopen` means the .so file will be opened at that time.
 - `setup` means the setup method which fills the mod info is called at that point.
 - `load` means the load method is called at that point
 - `late_load` means the late_load method is called at that point.

|                         | Lib            | Early Mod         | Mod                            |
|-------------------------|----------------|-------------------|--------------------------------|
| app start               | `dlopen`       | `dlopen`, `setup` | not yet initialized            |
| il2cpp_init             | nothing called | `load`            | not yet initialized            |
| DestroyObjectHighLevel  | nothing called | `late_load`       | `dlopen`, `setup`, `late_load` |

All libs, early mods, and mods are loaded in _sortedd_ order, as opposed to directory order. This allows the load order to be consistent across the same sets of files. Strictly speaking, this uses [std::filesystem::path::compare](https://en.cppreference.com/w/cpp/filesystem/path/compare) to sort and thus will load in that order.


2. Update Mod Initialization
Scotland2 has two key loading phases:

Early Mods (early_mods folder) – Loaded during il2cpp_init (for low-level hooks).

Regular Mods (mods folder) – Loaded during Unity’s first scene load (DestroyObjectHighLevel).

Changes Needed:
If your mod depends on Unity (e.g., UI, GameObjects), ensure it uses late_load instead of direct load():

cpp
extern "C" void late_load() {
    // Your Unity-dependent initialization here
}
If it hooks game functions early, place it in early_mods and use standard load().

3. Check Dependencies
Scotland2 requires libmain.so patches. Ensure your mod doesn’t conflict with:

QuestUI (for UI elements)

Codegen (for method patching)

Custom Types (if using il2cpp type lookup)

Run adb logcat to detect missing dependencies.

4. Fix Common Issues
Issue	Solution
Mod crashes on launch	Missing late_load or incorrect hooking. Check logs.
Mod loads but doesn’t work	Dependency missing (e.g., libbeatsaber-hook.so not updated).
UI elements fail	Ensure QuestUI is installed and your mod waits for DestroyObjectHighLevel.
5. Recompile with Scotland2-Specific Patches
Update your mod’s CMakeLists.txt or build script to link against Scotland2’s libraries (if needed).

Example (for hooking game methods):

cpp
#include "scotland2/shared/loader.hpp"
Scotland2::register_mod(&your_mod_init);
6. Test & Debug
Logging: Scotland2 outputs logs to adb logcat. Filter with:

bash
adb logcat -s Scotland2
Fallback: If issues persist, try:

Moving the mod to early_mods.

Renaming the .so file (e.g., aaa_Mod.so to load first).


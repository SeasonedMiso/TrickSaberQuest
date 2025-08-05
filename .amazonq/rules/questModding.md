# Introduction to Modding Beat Saber for Oculus Quest

This guide is intended to serve as a starting point for writing your own mods for Beat Saber for Oculus Quest. The modding scene for Quest is fairly small and fast-moving, so certain tools, techniques, and best practices may quickly become outdated, and instructional guides can be hard to find. To successfully write mods you'll need to be scrappy and resourceful. The goal of this guide is to give you the tools to *find* the information you need, not necessarily to give you all the information.

Certain assumptions are made, like that you are at least vaguely familiar with software development, and can navigate a terminal. Most resources have been created with this in mind. Windows is not a hard requirement, but tools and build scripts referenced here are all designed for use on Windows, so being in that environment may make your life easier. Mods themselves are written in C++, and this guide does not aim to teach the language.

When in doubt, check out the [BSMG Discord #quest-mod-dev channel](https://discord.gg/beatsabermods), and try searching on Discord before asking questions. In addition, most mods posted on BSMG include their source code, and browsing through those sources is a great way to familiarize yourself with certain patterns and techniques.

Last updated: 6 August 2021, Beat Saber 1.16.4

**Table of Contents**
- [Introduction to Modding Beat Saber for Oculus Quest](#introduction-to-modding-beat-saber-for-oculus-quest)
  - [Prerequisites](#prerequisites)
    - [Oculus Quest setup](#oculus-quest-setup)
    - [Development environment](#development-environment)
  - [Starting a new project](#starting-a-new-project)
    - [Using VSCode Project Templates](#using-vscode-project-templates)
    - [Without Project Templates](#without-project-templates)
  - [Build scripts](#build-scripts)
  - [Modding concepts](#modding-concepts)
    - [Hooks](#hooks)
      - [Finding methods to hook](#finding-methods-to-hook)
        - [Browsing codegen headers](#browsing-codegen-headers)
        - [Dumping DLLs from the Quest](#dumping-dlls-from-the-quest)
        - [Browsing Quest or PC DLLs using dnSpy](#browsing-quest-or-pc-dlls-using-dnspy)
    - [Il2Cpp and codegen](#il2cpp-and-codegen)
    - [Unity engine](#unity-engine)
    - [custom-types](#custom-types)
    - [QuestUI](#questui)
  - [Basic examples](#basic-examples)
    - [Modifying a gameplay attribute](#modifying-a-gameplay-attribute)
    - [Modifying a menu](#modifying-a-menu)
  - [Going further](#going-further)
    - [Logging](#logging)
    - [Sharing and distribution](#sharing-and-distribution)
      - [Distributing mods with dependencies](#distributing-mods-with-dependencies)
  - [Links](#links)
    - [Tools and resources](#tools-and-resources)
    - [Example repositories](#example-repositories)
- [BeatSaber C# porting guide for Quest](#beatsaber-c-porting-guide-for-quest)
  - [QPM-Rust migration](#qpm-rust-migration)
  - [NOTICE: BS-HOOKS 2.0 AND CUSTOM-TYPES 0.12.0 ARE NOW OUT](#notice-bs-hooks-20-and-custom-types-0120-are-now-out)
  - [PRs are welcome!](#prs-are-welcome)
  - [Using vcpkg with CMake and QPM-Rust](#using-vcpkg-with-cmake-and-qpm-rust)
  - [Configuring intellisense in VSCode](#configuring-intellisense-in-vscode)
  - [Objects and Codegen](#objects-and-codegen)
  - [Pointers](#pointers)
  - [Custom types and classes](#custom-types-and-classes)
    - [Custom destructor](#custom-destructor)
  - [Harmony Patch to BS-Hooks](#harmony-patch-to-bs-hooks)
  - [Pre fix](#pre-fix)
    - [Post Fix](#post-fix)
  - [Coroutines](#coroutines)
  - [SafePtr](#safeptr)
    - [How to use SafePtr?](#how-to-use-safeptr)
      - [Methods available to use](#methods-available-to-use)
      - [Scenarios](#scenarios)
  - [Diagnosing crashes](#diagnosing-crashes)
    - [Nullptr dereference (the equivalent of NullPointerException on C#)](#nullptr-dereference-the-equivalent-of-nullpointerexception-on-c)
    - [Nullptr dereference/SEGV\_MAPERR (but this code shouldn't be null, what gives?)](#nullptr-dereferencesegv_maperr-but-this-code-shouldnt-be-null-what-gives)
      - [Custom-types](#custom-types-1)
      - [SafePtr to avoid GC](#safeptr-to-avoid-gc)
    - [SEGV\_MAPERR (similar to ClassCastException or NullPtr)](#segv_maperr-similar-to-classcastexception-or-nullptr)
    - [SIGABRT (intentional crash)](#sigabrt-intentional-crash)
    - [ACCERR](#accerr)
  - [Optimizations](#optimizations)
    - [Recreate/Replace functions](#recreatereplace-functions)
    - [Use C++ alternative types](#use-c-alternative-types)
      - [List/Arrays](#listarrays)
      - [Hash Maps](#hash-maps)
    - [C++ threads](#c-threads)
    - [Lazy Initialize](#lazy-initialize)

---

## Prerequisites

### Oculus Quest setup

Beat Saber should already be modded with the latest [BMBF](https://bmbf.dev/stable), with Developer Mode enabled on your Quest. You should also have [SideQuest](https://sidequestvr.com/setup-howto) installed on your computer, though if you've gotten BMBF installed without it then you likely do not need this guide.

**Tips and tricks**:
- To keep the Quest display from turning off when you remove it, you can place a piece of tape over the black sensor in the inside top-center of the headset, above the nose.
- Video can be streamed from your Quest to your computer through SideQuest to see the VR display without putting the headset on.
- Wireless debugging can be enabled through SideQuest to develop without the Quest plugged in. This will quickly drain your battery and is not recommended, but can be useful for play testing.
- Keep some extra AA batteries (or a batter charger!) handy, as your controllers will spend a lot of time drawing power while developing.


### Development environment

This guide assumes you're using [Visual Studio Code](https://code.visualstudio.com/), however it's not a hard requirement and plenty of modders use other IDEs, though some steps in this guide may differ. Note that, no matter your IDE, you may encounter some issues with Intellisense: when in doubt, double check your include paths and try refreshing your Intellisense cache (Ctrl+Shift+P > )

- [Android NDK](https://developer.android.com/ndk/downloads) must be installed. Note its path for project template setup later.
- [Android SDK](https://developer.android.com/studio/releases/platform-tools#downloads) is strongly recommended and this guide assumes it is present in your `PATH`. It's necessary for running `adb` commands.
- [qpm (Quest Package Manager)](https://github.com/sc2ad/QuestPackageManager) is required and should be in your `PATH`. *Note*: qpm releases can be found by going to Actions on GitHub, to the latest build, and downloading the appropriate artifact for your system.


---

## Starting a new project

We will start from [Lauriethefish's project template](https://github.com/Lauriethefish/quest-mod-template). It's made to use the VSCode [Project Templates extension by cantonios](https://marketplace.visualstudio.com/items?itemName=cantonios.project-templates). If you're using VSCode and the extension, see the next section ([Using VSCode Project Templates](#using-vscode-project-templates)), otherwise skip to the one after ([Without Project Templates](#without-project-templates))

### Using VSCode Project Templates

With the VSCode extension installed:
- Open the project templates folder (Ctrl+Shift+P > `Project: Open Templates Folder`) and extract the Quest mod template into a new folder inside the templates folder.
- Create a new empty folder for your project somewhere and open it in VSCode (File > `Open Folder...`)
- Create a project from the template (Ctrl+Shift+P > `Project: Create Project from Template`)
  - For `ndkpath`, add the path to your NDK _using forward slashes_.
  - `id` is the internal name of your mod (no spaces).
  - `name` is the readable name of your mod.
  - `author` is your name or handle.
  - `description` is a short description of your mod.

### Without Project Templates

- Extract the template ZIP into a new folder somewhere.
- For each `.ps1` file, right click > Properties > Check "Unblock" on the lower right. This will prevent a security confirmation every time you run one of the scripts.
- Find and replace each of the project template tags throughout the project:
  - Replace `#{ndkpath}` with the path to your NDK _using forward slashes_.
  - Replace `#{id}` with the internal name of your mod (no spaces).
  - Replace `#{name}` with the readable name of your mod.
  - Replace `#{author}` with your name or handle.
  - Replace `#{description}` with a short description of your mod.
- In case the template may be updated with additional tags, you may want to do an extra project wide search for the template tag format (e.g. `#\{([^}]+)\}` if you have regex search)

---

Once your project directory is set up and all template tags have been replaced.

- Run `qpm restore`. This will download the dependencies defined in `qpm.json`. When you add or change a dependency, rerun the command. See [the qpm repository](https://github.com/sc2ad/QuestPackageManager) for more information on using qpm.
- To run an initial test build, run `build.ps1`.

**Tips and tricks**:
- Commands can be run from a PowerShell terminal inside VSCode (Terminal > New Terminal or Ctrl+Shift+\`)


---

## Build scripts

- `build.ps1` will just build the mod (generates `.so` file) and nothing else. Useful for confirming code validity, but you may want to use other scripts to do more useful things with the build output.
- `buildQMOD.ps1` will build the mod and package it into a `.qmod` that can be installed via BMBF. Once you are ready to share the mod with others, this is the thing you distribute.
- `copy.ps1` will build the mod and copy it directly to your Quest's mods directory (`/sdcard/Android/data/com.beatgames.beatsaber/files/mods`). Run `copy.ps1 --log` to begin logging from the Quest to your terminal after the files are copied and the game restarts.

---

## Modding concepts

Beat Saber is made using Unity and most of the game is written in C#. This C# is compiled down to C++ (thus the so-called "il2cpp"), and it's this bit which you can interface with to mod the game. [beatsaber-hook](https://github.com/sc2ad/beatsaber-hook) provides utilities to interact with this layer and the underlying C# code for both the game and the Unity engine.

At a high level, the modding process goes like this:
  - Find some game method(s) to hook, acting as a sort of entry point. Your code will be injected and executed when the hooked method is run.
  - Interact with the game code (largely in codegen's `GlobalNamespace`) and `UnityEngine` to make changes inside the game world.


### Hooks

Hooks are the primary way of interfacing with the game. You find a method that the game calls, and run some of your code whenever that method is called. The hooks themselves are written in two parts. First, you create the hook using the `MAKE_HOOK_MATCH` macro, then you install the hook using the `INSTALL_HOOK` macro.

`MAKE_HOOK_MATCH` takes the args: `hook_name, methodPointer, return_type, ...args`, where `hook_name` is whatever you want it to be, `methodPointer` is the method you want to hook, `return_type` is the actual type that the original function returns, and `...args` is all of the arguments passed to the original method. When hooking an *instance method*, the first argument will always be a pointer to the class instance itself, and this self-reference is _not_ included in the number of args specified when installing the hook.

Hooks effectively replace the original function call, so you generally need to call the original function at some point in your hook. Make sure to return the appropriate type for the function you are hooking:
```c++
// For a void hook, just make sure to call the original at some point
MAKE_HOOK_MATCH(MyHook, &Namespace::Class::Method, void, Namespace::Class* self, SomeType arg1, SomeType arg2) {
  // your code here
  MyHook(self, arg1, arg2);
  // or here
}
// When the hooked function returns a value, make sure to return something of that type
MAKE_HOOK_MATCH(MyHook2, &Namespace::Class::Method, int, Namespace::Class* self, SomeType arg1, SomeType arg2) {
  int original_value = MyHook2(self, arg1, arg2);
  // your code here
  return original_value;
}
```

`INSTALL_HOOK` is where you install your hook code onto the actual method you want to hook. It will look something like this:
```c++
INSTALL_HOOK(getLogger(), MyHook);
```

As an example to put these together, let's say you want to a hook a method in the `Foo` class called `SomeMethod` that returns a `float` and takes one `Il2CppString*` argument:
```c++
MAKE_HOOK_MATCH(MyHook, &GlobalNamespace::Foo::SomeMethod, float, GlobalNamespace::Foo* self, Il2CppString* some_arg) {
  /* do something */
  return MyHook(self, some_arg);
}
extern "C" void load() {
  INSTALL_HOOK(getLogger(), MyHook);
}
```

**Important note**: When using any of the **unsafe** hook macros mistakes in hook definitions and installation are a *very* common source of issues and crashes. If your game crashes on startup with a null pointer dereference after creating a new hook, double (and triple!) check that everything is correct, including the class name, method name, number of arguments (surprisingly easy to miscount), and the function signature of the hook itself. Even if your game doesn't crash, if you do not have the exact same function signature for your hook, weird things may happen!


#### Finding methods to hook

There are two main approaches to searching through the game's methods: by browsing the headers in codegen, or by browsing the game's DLLs. If you do not have access to the PC version of the game, then either approach will yield very similar results. However, the PC version's DLLs can be hugely beneficial to modding, as they include the actual source code of the methods, allowing you to analyze code flow and more easily explore your options.

If you *are* using PC sources, you may want to double check that the methods you're working on are actually present in the Quest version, either by dumping them as well or by checking the codegen headers. Most, but not all, of the function signatures are identical between the two.

##### Browsing codegen headers

Instead of dumping the code yourself, one alternative is to search through what's available in the `codegen` Quest package, which is a core library on BMBF and thus available to all users automatically. This library is a set of headers generated via basically the same method as previously described ([Dumping DLLs from the Quest](#dumping-dlls-from-the-quest)), so either method works well as a source of truth.

- Add `codegen` as a dependency: `qpm dependency add "codegen"`, then run `qpm restore` to download it.
- Use your IDE to search through the codegen headers to find hookable methods.

![Searching through codegen headers using VSCode](vscode-codegen-example.png)
*Searching through codegen headers can be quite effective, and will provide similar function signatures as dnSpy.*

##### Dumping DLLs from the Quest

The process of getting the code off your Quest is basically to dump the APK, extract some files from it, and dump DLLs out of them.

- *Get the Beat Saber APK*: From SideQuest, go to "Currently Installed Apps", click the cog icon next to Beat Saber, and then click "Backup APK file".
- *Extract from APK*: Use an archive tool such as [7zip](https://www.7-zip.org/) to extract `lib/arm64-v8a/libil2cpp.so` and `assets/bin/Data/Managed/Metadata/global-metadata.dat` from the APK.
- *Dump DLLs*: Run [il2CppDumper](https://github.com/Perfare/Il2CppDumper) and select the two files from the previous step. This will generate numerous DLLs into a `DummyDll` directory.


##### Browsing Quest or PC DLLs using dnSpy

Once you have your sources, whether from PC or Quest, it's time to fire up [dnSpy](https://github.com/dnSpy/dnSpy) to browse through them. Most of the relevant game code can be found in `Main.dll`, though there are also Unity libraries worth taking a look at. You can open multiple sources at a time into dnSpy and search through all of them simultaneously.

![dnSpy usage example](dnSpy-example.png)
*dnSpy can be used to browse or search through game classes and methods. These signatures are needed to properly hook methods. With PC DLLs, the method bodies will be present, while dumped Quest DLLs will have stubs.*


### Il2Cpp and codegen

`il2cpp` is the mechanism that Unity uses to compile game code into C++. `libil2cpp` and beatsaber-hook's `il2cpp_utils` can be used to interface with the game's original C# types and methods. Important to note that most of the values returned by these methods are pointers to objects with C# types.

Here's a super high-level view at commonly used types and methods:
```c++
Il2CppObject* generic_object; // untyped reference to an arbitrary C# object
std::optional<Il2CppObject*> member_value = il2cpp_utils::GetPropertyValue(generic_object, "property_name");
std::optional<T> typed_property = il2cpp_utils::GetPropertyValue<T>(generic_object, "string_property");
MethodInfo* member_method = il2cpp_utils::FindMethod(generic_object, "method_name");
std::optional<T> method_retval = il2cpp_utils::RunMethod<T>(generic_object, "method_name", some_arg);
// FindMethod and RunMethod also have unsafe versions which do not do type checking
Il2CppClass* cs_class = il2cpp_utils::GetClassFromName("Namespace", "ClassName");
System::Type class_type_reference = cs_class::GetType();

Il2CppString* string_from_game; // C# strings are pointers to UTF-16 strings
std::string native_string = to_utf8(csstrtostr(string_from_game));
```

For a full view of the interface, look into [the header for il2cpp_utils](https://github.com/sc2ad/beatsaber-hook/blob/master/shared/utils/il2cpp-utils.hpp).

---

[codegen](https://github.com/sc2ad/BeatSaber-Quest-Codegen) is a QPM package and core mod that contains auto-generated headers of the full Beat Saber C# interface. These can often be preferable to using il2cpp directly, namely increased type-safety and intellisense method checking.

Here's an brief comparison between some of the methods shown above and their codegen equivalents:
```c++
// In a hook: say you're hooking GlobalNamespace::Foo::Bar
INSTALL_HOOK(getLogger(), MyHook);
// If you include that class from codegen
#include "GlobalNamespace/Foo.hpp"
// Then instead of an Il2CppObject*, you can use an actual instance of Foo*
MAKE_HOOK_MATCH(MyHook, &GlobalNamespace::Foo::Bar, void, GlobalNamespace::Foo* self) { MyHook(self); }

void something(Foo* instance) {
  // Methods can be called directly, so instead of this:
  auto result = il2cpp_utils::RunMethod<T>(instance, "bar");
  // You can use this:
  auto result = instance->bar();

  // For fields:
  auto field = il2cpp_utils::GetFieldValue<T>(instance, "someField");
  // Field values can generally be accessed directly
  auto field = instance->someField;

  // For properties:
  auto prop = il2cpp_utils::GetPropertyValue<T>(instance, "someProperty");
  // You will generally have a generated getter method:
  auto prop = instance->get_someProperty();
}
```


### Unity engine

Since the game itself is made with Unity, you can use everything available in the [Unity Scripting API](https://docs.unity3d.com/ScriptReference/index.html) to interface with the game world. While an in-depth look at Unity is not in the scope of this document, here are a few pointers to get started.

- Everything in the world is a `GameObject`. GameObjects all have a `Transform` which controls their place in the hierarchy and position in the game world. `Component`s may be added to GameObjects which provide scripting functionality with a [consistent lifecycle](https://docs.unity3d.com/Manual/ExecutionOrder.html).
- If you have the PC version of Beat Saber, you can use [Runtime Unity Editor](https://github.com/ManlyMarco/RuntimeUnityEditor) to explore the Unity game world, scene hierarchy, and even a C# REPL in-game. Most of the PC and Quest versions are identical, so information from here can be useful for your Quest mod. If you _don't_ have the PC version, you may be able to get help from someone in Discord to screenshot the GameObject hierarchy from a particular screen.

Here's a few quick examples of interacting with the Unity API:
```c++
// Find an existing GameObject
UnityEngine::GameObject* existing_object = UnityEngine::GameObject::Find(il2cpp_utils::createcsstr("GameObjectName"));
if (existing_object == nullptr) return; // the object was not found in the scene

// Find a Component in a GameObject by type
HMUI::CurvedTextMeshPro* child_text = existing_object->FindObjectOfType<HMUI::CurvedTextMeshPro*>();
Il2CppString* child_text_value = child_text->get_text();

// Create a new GameObject and give it a parent
UnityEngine::GameObject* my_object = UnityEngine::GameObject::New_ctor(il2cpp_utils::createcsstr("MyNewGameObject"));
my_object->get_transform()->SetParent(existing_object->get_transform(), false);

// Adding a Component to a GameObject
HMUI::CurvedTextMeshPro* my_text = my_object->AddComponent<HMUI::CurvedTextMeshPro*>();
my_text->set_text(il2cpp_utils::createcsstr("hello world"));
```

### custom-types

custom-types is a (core) library that can be used to create C# classes from within C++. This is necessary for certain use cases, for example:
- inheriting Unity types, for example to make a `MonoBehaviour` that can be added to `GameObjects`
- creating UI elements that can be controlled within the same system as everything else, such as `ViewControllers` and `FlowCoordinators`

To install custom-types using QPM, run `qpm dependency add custom-types`

Broadly speaking, custom types are created in three parts:
- declaring the type in a header using the macros from `custom-types/shared/macros.hpp`,
- defining the implementation within a source file,
- and registering the type inside `load()` using `custom_types::Register::AutoRegister()`

To find examples of custom-types being created and used, try [searching GitHub for DECLARE_CLASS_CODEGEN](https://github.com/search?q=DECLARE_CLASS_CODEGEN&type=code).


### QuestUI

QuestUI is a (core) library that can be used to create UI elements, such as text and input controls, as well as manage settings menus for your mod.

To install QuestUI using QPM, run `qpm dependency add questui`

Most of the information you'll need to experiment with these elements lives in the [BeatSaberUI.hpp header](https://github.com/darknight1050/questui/blob/master/shared/BeatSaberUI.hpp).

To create a simple mod settings menu, all you need is a `DidActivate` function matching the signature [`void DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)`](https://github.com/darknight1050/questui/blob/master/shared/QuestUI.hpp#L25). When `firstActivation` is true, you can add your UI elements onto the ViewController. Finally, in your mod's `load()`, register it with QuestUI by calling `QuestUI::Register::RegisterModSettingsViewController<T*>(modInfo)`, and a button will be added to the Mod Settings menu.

To create a more complex mod settings menu, you'll need to make a Custom Type extending `HMUI::ViewController` and overriding the `DidActivate` method to add your UI elements. After registering the custom type, call `QuestUI::Register::RegisterModSettingsViewController<T*>(modInfo)`,

You can also create a _much_ more complex mod settings menu by making your own `HMUI::FlowCoordinator` (as well as corresponding ViewControllers), and registering that with `QuestUI::Register::RegisterModSettingsFlowCoordinator<T*>(modInfo)`.

As always, examples can be found with a GitHub search, for example [searching for RegisterModSettingsViewController](https://github.com/search?q=RegisterModSettingsViewController&type=code).


---

## Basic examples

With a basic understanding of what's going on, let's walk through a few examples of simple modding operations so that you can make some changes to the game and see your efforts in action. Along the way, you'll encounter some new concepts that might not be elaborated on - you should be well-equipped enough from here to investigate those on your own.


### Modifying a gameplay attribute

In this walkthrough, we'll modify gameplay by decreasing note jump speed. Begin by creating a new project from the mod template (as described in [Starting a new project](#starting-a-new-project))

Let's start out by searching for "note jump speed" with dnSpy. There are quite a few results. In order to find which is the right method to hook,  you'll need a bit of intuition, maybe to dig through methods (especially if you have PC DLLs), and possibly to write some code to hook a bunch of different methods and log out when they are called.

Eventually you'll come across the `BeatmapObjectSpawnMovementData` class. If you inspect its `Update` method, it appears to use `this._startNoteJumpMovementSpeed` to calculate the actual NJS, and that property is set in the `Init` method. You can see the whole signature here, so it's time to write a little hook:

```c++
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "UnityEngine/Vector3.hpp"
MAKE_HOOK_MATCH(BeatmapObjectSpawnMovementData_Init, &GlobalNamespace::BeatmapObjectSpawnMovementData::Init, void,
  GlobalNamespace::BeatmapObjectSpawnMovementData* self,
  int noteLinesCount,
  float startNoteJumpMovementSpeed,
  float startBpm,
  float noteJumpStartBeatOffset,
  float jumpOffsetY,
  UnityEngine::Vector3 rightVec,
  UnityEngine::Vector3 forwardVec
) {
  BeatmapObjectSpawnMovementData_Init(self, noteLinesCount, startNoteJumpMovementSpeed, startBpm, noteJumpStartBeatOffset, jumpOffsetY, rightVec, forwardVec);
  getLogger().info(
    "BeatmapObjectSpawnMovementData_Init called. startNoteJumpMovementSpeed is: %.2f",
    startNoteJumpMovementSpeed
  );
}

extern "C" void load() {
  il2cpp_functions::Init();
  INSTALL_HOOK(
    getLogger(),
    BeatmapObjectSpawnMovementData_Init
  );
}
```

Now, if you run `copy.ps1` to build this and load it into your game, then start playing a song, you should see something similar to this logged just before the song loads in:
```
BeatmapObjectSpawnMovementData_Init called. startNoteJumpMovementSpeed is: 17.000000
```

All you need to do now is modify the value of `startNoteJumpSpeed` being passed to the original method in the hook. Try passing `startNoteJumpSpeed * 2.0f` or `startNoteJumpSpeed / 0.5f` and see what happens!


### Modifying a menu

In this walkthrough, we'll modify the main menu and change the text on one of its options. To do this, we need to know two things: which function to hook, and what GameObject to change.

For finding the function to hook, search around using dnSpy or through the codegen headers for the MainMenu (left as an exercise for the reader 😉). From this you'll find a `MainMenuViewController` class. You may eventually find that all menus have an associated `ViewController` class, and these all have a `DidActivate` method that gets called when the menus become active.

For finding the GameObject, this is a bit harder without the PC version of the game and Runtime Unity Editor. One exercise you can try is to write some code to iterate through text components in the scene (generally `HMUI::CurvedTextMeshPro`) and find their text and parent GameObjects:

```c++
std::optional<UnityEngine::GameObject*> FindComponentWithText(std::string_view find_text) {
  // find root components in scene
  auto scene = UnityEngine::SceneManager::GetActiveScene();
  auto root_objs = scene->GetRootGameObjects();
  for (int i = 0; i < root_objs->Length(); i++) {
    auto text_components = root_objs->values[i]->GetComponentsInChildren<HMUI::CurvedTextMeshPro>();
    for (int j = 0; j < text_components->Length(); j++) {
      auto text = to_utf8(csstrtostr(text_components->values[i]->get_text()));
      if (text == find_text) {
        return text_components->values[i]->get_gameObject();
      }
    }
  }
  return std::nullopt;
}
```

At any rate, let's say we want to replace the text on the main menu button that says "Solo". This happens to be on a GameObject called `SoloButton`, so with our hook in the right place, it's just a matter of finding that object, getting its text component, and setting the text:

```c++
#include "GlobalNamespace/MainMenuViewController.hpp"
#include "HMUI/CurvedTextMeshPro.hpp"
MAKE_HOOK_MATCH(MainMenuViewController_DidActivate, &GlobalNamespace::MainMenuViewController::DidActivate, void,
  GlobalNamespace::MainMenuViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling
) {
  MainMenuViewController_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
  auto solo_button = UnityEngine::GameObject::Find(il2cpp_utils::createcsstr("SoloButton"));
  auto solo_text = solo_button->FindComponent<HMUI::CurvedTextMeshPro>();
  solo_text->set_text("Hello World");
}
extern "C" void load() {
  il2cpp_functions::Init();
  INSTALL_HOOK(
    getLogger(),
    MainMenuViewController_DidActivate
  );
}
```

With any luck, after compiling and copying this to your Quest, you'll now have a Hello World button on the main menu!

---

## Going further

With your environment setup and a basic understanding of hooks, you're well on your way to proficiency writing mods. Spend some time exploring the game's methods and where they're called. If you get stuck and can't Google your way out of it, try asking in #quest-mod-dev on the BSMG Discord.


### Logging

Make liberal use of logs to debug when calls are happening and why crashes are occurring. Logs can be viewed in realtime using `adb logcat`. Unfiltered, `logcat` will provide a lot of information but may be very noisy. Here are a couple helpful `logcat` recipes:

- `adb logcat QuestHook[my_mod|v0.1.0]:* AndroidRuntime:E *:S` - Log only messages from `my_mod`, as well as crash dumps.
- `adb logcat | Select-String QuestHook` - Log all `QuestHook` messages, including those of your mod, other mods, and `UtilsLogger` (which can provide useful information on what's going on behind the scenes, especially for crashes)

You can also pipe logcat to a file to generate dumps which may be helpful in having someone else help you.


### Sharing and distribution

When you're ready to share your work, package it into an installable zip file with `buildBMBF.ps1` and share it with the world!

- Share it in #quest-mods on the [BSMG Discord](https://discord.gg/beatsabermods)
- Submit your mod to [QuestBoard](https://questboard.xyz) via the [QuestBoard Hub Submission Form](https://docs.google.com/forms/d/e/1FAIpQLScT0pGWOIcifDALjuuefF7L9JgcH5epUaCcDj228DSs3KIyHg/viewform)


#### Distributing mods with dependencies

The `mod.json` file contains metadata for BMBF to allow your mod to be installed and managed correctly. Any dependencies _that are not core mods_ must be defined in your mod.json, and copied into your distributable .qmod in `buildQMOD.ps1`.
The schema for it can be found [here](https://github.com/Lauriethefish/QuestPatcher/blob/main/QuestPatcher.Core/Resources/qmod.schema.json).

---

## Links

### Tools and resources
- [BSMG Discord](https://discord.gg/beatsabermods) - primary source of mods, resources, help, and discussion about Beat Saber modding
- [SideQuest](https://sidequestvr.com/setup-howto) - for GUI access to Quest's filesystem, and other tools including ScrCpy
- [BMBF](https://bmbf.dev/stable) - must be installed on your Quest to start modding Beat Saber
- [qpm (Quest Package Manager)](https://github.com/sc2ad/QuestPackageManager) - dependency repository for Quest mods
- [il2CppDumper](https://github.com/Perfare/Il2CppDumper) - dumps Beat Saber's interface into browseable DLLs
- [dnSpy](https://github.com/dnSpy/dnSpy) - .NET decompiler and browser to view DLL contents
- [7zip](https://www.7-zip.org/) - archive tool which can be used to open an APK
- [Visual Studio Code](https://code.visualstudio.com/) - relatively lightweight cross-platform code editor
- [Lauriethefish's project template](https://github.com/Lauriethefish/quest-mod-template) - starting point for writing a mod
- [Runtime Unity Editor](https://github.com/ManlyMarco/RuntimeUnityEditor) - extension for PC Beat Saber to explore the Unity game world
- [Fern's BeatSaber C# porting guide](https://github.com/Fernthedev/beatsaber-quest-porting-guide) - a guide for porting BeatSaber C# PC mods to Quest il2Cpp C++ mods

### Example repositories
- [GitHub search for MAKE_HOOK_OFFSETLESS is fairly effective](https://github.com/search?q=MAKE_HOOK_OFFSETLESS&type=code)
- [GitHub search for DECLARE_CLASS_CODEGEN to find custom-types examples](https://github.com/search?q=DECLARE_CLASS_CODEGEN&type=code)
- [GitHub search for RegisterModSettingsViewController to find mod settings menu examples](https://github.com/search?q=RegisterModSettingsViewController&type=code)


# BeatSaber C# porting guide for Quest

This guide aims to do the following things:

- Be a complementary guide with [Laurie/Danrouse' modding guide](https://github.com/danrouse/beatsaber-quest-modding-guide)
- Teach some _pointers_ on how to properly port mods (get it?)
- Discipline the reader with proper programming practices (no var p = 5)

This assumes that you have a basic understanding of C++ and a bit of C#. You will also be using codegen in this tutorial.
This guide will explain different practices in no particular order

## QPM-Rust migration
Take a look at [this guide](qpm-rust.md)

## NOTICE: BS-HOOKS 2.0 AND CUSTOM-TYPES 0.12.0 ARE NOW OUT
For both Beat Saber Quest modding and Gorilla Tag modding (tell Red to force everyone on GT to these new libraries), there have been some major changes to modding in Quest.

The most notable changes are:
- [Codegen hooks](https://github.com/sc2ad/beatsaber-hook/releases/tag/v2.0.0)
  - Hooks are now type checked at runtime!
  - Include `beatsaber-hooks/shared/utils/hooking.hpp`
  - Use `MAKE_HOOK_MATCH` for codegen hooks as so: `MAKE_HOOK_MATCH(HookName, &Namespace::Clazz::Foo, ret, self if needed, args...)`
  - If codegen doesn't work, use `MAKE_HOOK_FIND_CLASS(Hook, classof(MyClass*), "MethodName", ret, self, args...)`
  - If you need to hook a constructor or use the legacy FindMethod, use `MAKE_HOOK_FIND_CLASS(Hook, "Namespace", "Class", "MethodName", ret, self, args...)`.
  - Use `INSTALL_HOOK` instead of `INSTALL_HOOK_OFFSETLESS`
- [Custom-types](https://github.com/sc2ad/Il2CppQuestTypePatching/releases/tag/v0.9.0)
  - No more `custom_types::Register::RegisterType<>();` or `custom_types::Register::RegisterTypes<>();`
    - Use `custom_types::Register::AutoRegister();`
  - No more `REGISTER_` macros
  - `DEFINE_TYPE(CLASS)` is now `DEFINE_TYPE(NAMESPACE, CLASS)`
  - You must now use either `DECLARE_STATIC_METHOD` or `DECLARE_INSTANCE_METHOD`

## PRs are welcome!

You can make PRs to this repo, though your new documentation should have the following requirements:

- [x] Properly describe the practice and its use cases. Specify when to/not to use said practice
- [x] The reader should be aware of the consequences/advantages of said method
- [x] Show examples from C# identical/similar code if applicable

## Using vcpkg with CMake and QPM-Rust
You can use vcpkg with Android, though there are a few caveats. 
- Not all packages support Android NDK
- [VS Build Tools 2022](https://aka.ms/vs/17/release/vs_BuildTools.exe) with MSVC/Windows 10 SDK is required for some packages such as protobuf
- Many packages support Android NDK through the community unofficially

Add these lines before your `project(${COMPILE_ID})`
```cmake
set(VCPKG_TARGET_TRIPLET arm64-android)
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE ${CMAKE_ANDROID_NDK}/build/cmake/android.toolchain.cmake)
string(REPLACE "\\" "/" VCPKG_ROOT_WINDOWS_FIX $ENV{VCPKG_ROOT})
set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOT_WINDOWS_FIX}/scripts/buildsystems/vcpkg.cmake)
```
Then add these lines after `add_library`
```cmake
find_package(protobuf CONFIG REQUIRED)
target_link_libraries(${COMPILE_ID} PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
```

## Configuring intellisense in VSCode

VSCode is a popular code editor and development environment, but its intellisense can be less than ideal, especially in quest modding projects. The best way to remove all the error squiggles is to provide the compile_commands file to the intellisense. This of course assumes that you have the basic C++ extensions installed.

First, add this line to your CMakeLists.txt to create a file named `compile_commands.json` inside your build folder the next time the mod is built:
```cmake
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
```
Next, set the compile commands field in your intellisense configuration, either through the extension UI or directly in the json file. For the extension UI, you will need to add the path to your `compile_commands.json` file to the `Compile commands` setting in the advanced settings. You can also add this line directly to the configuration in `c_cpp_properties.json` (modifying the path if necessary):
```json
"compileCommands": "${workspaceFolder}/build/compile_commands.json"
```
It might not be perfectly fixed immediately, but after a build to generate the file and potentially a few VSCode restarts, your project should have working type and include intellisense. Notably, due to using the `compile_commands` file, newly created files might not have correct intellisense until they are included in at least one build.

## Objects and Codegen

To understand why we use codegen, we have to understand how il2cpp works behind the scenes. BeatSaber uses the Unity engine which has 2 different ways of compiling: Mono and il2cpp. The PC version is compiled in Mono which allows mods to be created in C#, while also having a garbage collector and JIT optimizations. However, the Quest version uses il2cpp. Why you may ask?

Simply, the Quest (and even Quest 2) are not very powerful compared to their PC counterparts, so running BeatSaber in Mono will be pretty bad performance (even on Q2). So, what is Unity's solution to this dilemma? Il2cpp is a program developed by Unity that allows IL (the code that C# is compiled to which Mono or another runtime interprets) and converts it to C++. This allows C++ compilers such as clang/LLVM to further optimize the code to an extreme and even allows Unity games to run on platforms that do not support Mono altogether. The problem with this? Quest Modding can't access these classes the same way PC can, we have to paradigms similar to reflection. Take a look at this:

```cpp
static const MethodInfo* method = CRASH_UNLESS(il2cpp_utils::FindMethodUnsafe("UnityEngine", "GameObject", "Find", 1));
// nullptr since it's a static method, otherwise you provide the instance
Il2cppObject* object = il2cpp_utils::RunMethod(nullptr, method, il2cpp_utils::createcssstr("someObject"))
```

As you can see, this is problematic in many ways. For one, this has no type-safety therefore a compile doesn't guarantee a run. Second, this assumes that your code is even correct and can break easily between updates without notice. The code you may need to change may be even hard to test which may cause a crash you don't realize until it's too late.

Thanks to sc2ad's wonderful work on codegen, we can avoid avoid all these issues. Simply put, this is the code with codegen installed:

```cpp
#include "UnityEngine/GameObject.hpp"

UnityEngine::GameObject* go = UnityEngine::GameObject::Find("someObject");
```

This code is safe, compact and easy to read. Codegen has the added side-benefit of making mods easier to port from C# since the code can be very similar to identical in some ways.

Codegen behind the scenes wraps around Il2CppObject\* and gives easy access to its fields and methods. Every codegen object is actually a `Il2cppObject*`

## Pointers

In C++, we use pointers due to the way il2cpp works. In C#, you knowingly or unknowingly are (most of the time) using pointers in your code.

For example, C#

```csharp
GameObject go = UnityEngine.GameObject.Find("name");
```

while on C++ we use the following (assuming you are using codegen)

```cpp
#include "UnityEngine/GameObject.hpp"

UnityEngine::GameObject* go = UnityEngine::GameObject::Find(il2cpp_utils::newcsstr("name"));
```

## Custom types and classes

In the PC mods, you will know that it's quite easy to create your own MonoBehaviours. However, on Quest we cannot just use C++ classes to extend C# classes or create Unity components. Instead, we rely on the useful library called [custom-types by sc2ad](https://github.com/sc2ad/Il2CppQuestTypePatching). This library allows us (albeit still more work than our PC counterpart) to create C# classes. Take the following for example, which is a simple MonoBehaviour:

```hpp
// OurClass.hpp
#pragma once

#include "custom-types/shared/types.hpp"
#include "custom-types/shared/macros.hpp"

#include "UnityEngine/MonoBehaviour.hpp"

DECLARE_CLASS_CODEGEN(OurNamespace, OurClass, UnityEngine::MonoBehaviour,
  public:
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_CTOR(ctor);
    DECLARE_SIMPLE_DTOR();

    DECLARE_INSTANCE_FIELD(float, floatVar);
)
```

```cpp
// OurClass.cpp
#include "OurClass.hpp"

// Necessary
DEFINE_TYPE(OurNamespace, OurClass);

void OurNamespace::OurClass::ctor() {
  floatVar = 1.0f
  // Constructor!
}


void OurNamespace::OurClass::Update() {
  // Update method! YAY
}
```

This MonoBehaviour has a constructor, destructor, a method called "Update" and an instance field called `floatVar`.
It should be known that you cannot use non-il2cpp types in custom-types methods or fields (DECLARE\_ methods or fields), such as C++ structs or classes. You can use pointers, other custom-types and convertible value types though.

You should also know that you can use C++ methods and fields like any normal C++ class, but no constructors.

You also should not directly call the `ctor` method, and instead you should use `il2cpp_utils::New<OurNamespacce::OurClass*>(parametersHere);` (you technically can call ctor, but that won't actually construct the instance)

It is also important to know that the C++ destructor is never called when the type is freed by the GC, so if you have for example a `std::vector` or anything that needs deletion (such as std::unordered_map or anything that isn't [trivially constructed](https://en.cppreference.com/w/cpp/language/default_constructor)), it will never get freed and thus causes a memory leak. Thankfully, we can call the destructor ourselves very easily. This is a simple example as a solution to this kind of problem: (kindly provided by sc2ad)

```hpp
// type.hpp
#pragma once

DECLARE_CLASS_CODEGEN(Does, Stuff, Il2CppObject,
  std::vector<int> aCppVec;

  // This requires either INVOKE_CTOR or placement new to actually initialize the variable with the default value.
  DECLARE_INSTANCE_FIELD_DEFAULT(float, floatVar, 1.2f);

  DECLARE_CTOR(ctor);
  DECLARE_SIMPLE_DTOR();
)
```

```cpp
// type.cpp
DEFINE_TYPE(Does, Stuff);

void Does::Stuff::ctor() {
  // you should only use this is if your constructor is non-trivial or contains non-trivial constructible fields such as vectors. very tiny performance impact
  INVOKE_CTOR();
}
```

**Remember to register your custom type, which should be done in the load method as follows:**

```cpp
load() {
    custom_types::Register::AutoRegister();
}
```

Voila, we're done.

What about adding MonoBehaviours now? Well, it's simple too.

```cpp
UnityEngine::GameObject* go = getGameObjectSomehow(); // This is a placeholder, you'll be fixing this in your code

go->AddComponent<OurNamespace::OurClass*>(); // The * is necessary
```

And now our component is in the game.

Note that because we are defining a new method for construction (the C# ctor), we are not calling our C++ constructor. This means that our fields are uninitialized, including all calls to `DECLARE_INSTANCE_FIELD_DEFAULT` and non trivially constructible fields. We can fix this by calling the C++ constructor ourself, IN our C# one (ctor), via INVOKE_CTOR.

What does `INVOKE_CTOR` and `DECLARE_SIMPLE_DTOR` do behind the scenes? Well, first `INVOKE_CTOR` calls your C++ constructor at the cost of a _**very tiny**_ performance impact to initialize your fields. You do not need this call if you do not have [non-trivial constructible fields](https://en.cppreference.com/w/cpp/language/default_constructor) such as `std::vector`. `DECLARE_SIMPLE_DTOR` on the other hand causes the C++ destructor to be called by the C# destructor, which _in theory_ should have no memory leaks 🤞. Of course, if you have manually allocated data, you'll need your own destructor.

> Tip: If you plan on making a C# constructor just to invoke `INVOKE_CTOR`, you can use `DECLARE_DEFAULT_CTOR` the same way you would use `DECLARE_DEFAULT_DTOR` respectively.

You should **not** call either C# or C++ destructor outside of the destructor itself (e.g, just calling it anywhere in your code)
The GC will call it for you when it is no longer needed (do note that this does not apply to manually managed il2cpp created types)

### Custom destructor

This is a simple example using a custom destructor: (kindly provided by sc2ad)

```hpp
// type.hpp
#pragma once

DECLARE_CLASS_CODEGEN(Does, Stuff, Il2CppObject,
  std::vector<int> aCppVec;
  DECLARE_CTOR(ctor);
  DECLARE_DESTRUCTOR(dtor);
)
```

```cpp
// type.cpp
DEFINE_TYPE(Does, Stuff);

void Does::Stuff::ctor() {
  // create vector
  aCppVec = std::vector<int>();
}

void Does::Stuff::dtor() {
  // explicitly call the destructor, this almost always shouldn't be done outside of the destructor itself
  aCppVec.~vector();
}
```

## Harmony Patch to BS-Hooks

As you may already know, on Quest we do not have such things as post fix or pre fix like HarmonyPatches. So, how do we follow the same behaviour?

We use `MAKE_HOOK_MATCH(hookName, &Namesspace::Class::Method, returnType, instance, parameters...)` to define the hook and its code and `INSTALL_HOOK(getLogger(), hookName);` to register the hook.
You should NOT uninstall hooks, and beware for methods too small to be hooked. Ocassionally, there are methods in the game you CANNOT hook since they are too small, however you can workaround it by hooking other methods that ARE big enough.

Hook names are inconsequential, however I personally believe hook names should be as so: ```Class_Method```. If you hook the same method with different parameters, just suffix with a random number.

My recommended hook structure for mods is as follows (though always feel free to follow your own):
- include folder
  - ModName.hpp
    ```hpp
    #pragma once

    namespace ModName {
      namespace Hooks {
        void Class();
      }

      void InstallHooks();
    }
    ```
- src folder
  - hooks folder
    - Class.cpp
      ```cpp
      #include ModName.hpp
      
      MAKE_HOOK_MATCH(Class_Method, &Namespace::Class::Method, returnType, selfIfInstance, args...) {

      } 

      void ModName::Hooks::Class() {
        INSTALL_HOOK(getLogger(), Class_Method);
      }
      ```
  - ModName.cpp
    ```cpp
    #include "ModName.hpp"

    void ModName::InstallHooks() {
      ModName::Hooks::Class();
    }
    ```

P.S I learned this from StackDoubleFlow, thanks buddy ;)

## Pre fix

TODO:

### Post Fix

Let's take a look at Kaitlyn's SoundExtensions mod as an example (thanks for letting me use your mod for this)

Let's look at this [post fix harmony patch](https://github.com/nyamimi/SoundExtensions/blob/518624e607c7660ae40b03ce10a25859d377b9f7/SoundExtensions/HarmonyPatches/StandardLevelScenesTransitionSetupDataSO.cs)

Essentially, what this patch does is the following in order:

- Hooks into `StandardLevelScenesTransitionSetupDataSO.Init`
- After `StandardLevelScenesTransitionSetupDataSO.Init` is called, grabs the instance field parameters and `SoundExtensionsController.Instance.Init(difficultyBeatmap, previewBeatmapLevel);` is called

Now, to recreate this in C++ is quite simple (we'll ignore the fact we don't have the SoundExtensionsController class)
First, we will look at `GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp` and find the Init method.

[Here](https://github.com/sc2ad/BeatSaber-Quest-Codegen/blob/fc7ad7e6e7c65210251f2665f628740d612c5157/include/GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp#L118), we see that it has 10 parameters. As you may already know, we have to make our hook have ALL 10 parameters. It also returns void, making it easy to do a post fix (though with methods that don't it's still easy)

```cpp
#include "main.hpp" // Include main for hooks, avoid problems by including the headers directly

#include "GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp"

MAKE_HOOK_MATCH(StandardLevelScenesTransitionSetupDataSO_Init, &GlobalNamespace::StandardLevelScenesTransitionSetupDataSO::Init, void, GlobalNamespace::StandardLevelScenesTransitionSetupDataSO* self,
                    Il2CppString* gameMode, GlobalNamespace::IDifficultyBeatmap* difficultyBeatmap, GlobalNamespace::IPreviewBeatmapLevel* previewBeatmapLevel, GlobalNamespace::OverrideEnvironmentSettings* overrideEnvironmentSettings, GlobalNamespace::ColorScheme* overrideColorScheme, GlobalNamespace::GameplayModifiers* gameplayModifiers, GlobalNamespace::PlayerSpecificSettings* playerSpecificSettings, GlobalNamespace::PracticeSettings* practiceSettings, ::Il2CppString* backButtonText, bool useTestNoteCutSoundEffects
                     ) {
  // Anything here is pre fix since it is running before the original method

  // We call the original method by putting the name of our hook, self (which is the instance) as the first parameter and the rest of the parameters.
  StandardLevelScenesTransitionSetupDataSO_Init(self, gameMode, difficultyBeatmap, previewBeatmapLevel, overrideEnvironmentSettings, overrideColorScheme, gameplayModifiers, playerSpecificSettings, practiceSettings, backButtonText, useTestNoteCutSoundEffects);
  // Anything after this is running after the original method runs, just as post fix would.
  SoundExtensionsController::Instance.Init(difficultyBeatmap, previewBeatmapLevel);
}


void InstallHook() {
  // This will register our hook
  INSTALL_HOOK(getLogger(), StandardLevelScenesTransitionSetupDataSO_Init);
}
```

This does the following in order:

- Installs the hook
- When the hook runs, it will call the original method.
- Once the original method runs, you will run your code

What if the method you're hooking does NOT return void? It's simple to post fix too (and even change the result)

```cpp
#include "main.hpp" // Include main for hooks, avoid problems by including the headers directly

#include "GlobalNamespace/TrackLaneRingsRotationEffect.hpp"

MAKE_HOOK_OFFSETLESS(TrackLaneRingsRotationEffect_GetFirstRingRotationAngle, &GlobalNamespace::TrackLaneRingsRotationEffect::GetFirstRingRotationAngle, float, GlobalNamespace::TrackLaneRingsRotationEffect* self) {
  // Anything here is pre fix since it is running before the original method

  // We call the original method by putting the name of our hook, self (which is the instance) as the first parameter and the rest of the parameters.
  float result = TrackLaneRingsRotationEffect_GetFirstRingRotationAngle(self);
  // Anything after this is running after the original method runs, just as post fix would.

  // However, since this hook returns a variable, we cannot return null. So what can we do?

  float someVariable = 2;

  if (weWantToManipulateReturn) {
    // We can just add +2 to the return because we want to.
    // This does actually change the result of the original method and hooks that run after yours.
    return result + 2;
  } else {
    // Return the original
    return result;
  }
}


void InstallHook() {
  // This will register our hook
  INSTALL_HOOK(getLogger(), TrackLaneRingsRotationEffect_GetFirstRingRotationAngle);
}
```

## Coroutines

In games, we might need to run code on the main thread later or at a specific interval. Usually, we keep track using state management such as counters or switches, though this can sometimes be hard to safely implement or just plain annoying. Unity attempts to alleviate the issue by using coroutines, which are "fake threads" as some call them. Your code will run in an Update method and every `yield return` will pause the method until the next Update. Your coroutine will end at the function's end or `yield break;`.
For more information about coroutines, take a look at the [Unity Coroutine docs](https://docs.unity3d.com/Manual/Coroutines.html)

However, as you may already know, not everything is as easy in C++ like C#. Luckily, this is one of those moments where C++ is on-par with syntax sugar and performance. Let's take a look at the following coroutine in C#:

```csharp
IEnumerator coroutine() {
  for (int i = 0; i < 30; i++) {
    // Timer
    secondsPassed++;
    doSomethingEverySecond();
    yield return new WaitForSeconds(1);
  }
}
```

Then you'd call it as `monoBehaviour.StartCoroutine(coroutine());`
This is a simple way to track time without having to measure it in every Update call yourself. We can also do this in C++, thanks to the wonderful work of `custom-types` with similar but different syntax. The following is a direct port of the C# coroutine from above, notice how it is very similar:

```cpp
#include "System/Collections/IEnumerator.hpp"
#include "custom-types/shared/coroutine.hpp"
#include "UnityEngine/WaitForSeconds.hpp"

custom_types::Helpers::Coroutine coroutine() {

    for (int i = 0; i < 30; i++) {
      // Timer
      secondsPassed++;
      doSomethingEverySecond();
      co_yield reinterpret_cast<System::Collections::IEnumerator*>(CRASH_UNLESS(WaitForSeconds::New_ctor(1)));
    }

    co_return;
}
```

and you'd run `monoBehaviour->StartCoroutine(reinterpret_cast<System::Collections::IEnumerator*>(CoroutineHelper::New(coroutine())));` akin to the C# code above

Congrats, you've just made a coroutine in C++!

What exactly are `co_return` and `co_yield` in C++ though? Well `co_return` in C++ is the direct port of `yield break;` for C#. `co_yield`, as its name suggests, is a direct port to `yield return` for C#. You cannot use `return` in coroutines (which is a C++ 20 reason I don't want to delve into here)

_Do note that if your coroutine does NOT contain a `co_yield` or `co_return`, you WILL have LOTS of strange behaviour (believe me, I know it myself all too well). But you shouldn't use a coroutine if your method doesn't contain a `co_yield` or `co_return` to begin with, unless you're trying to run code in the Update method once (which would be weird)._

**Do note that the general rule of coroutines still apply here. You should avoid heavy work such as I/O or web requests on the main thread and instead use il2cpp threads (if you need to run il2cpp/Unity code in the thread) or use C++ threads for better performance. Coroutines are still on the main thread.**

## SafePtr

Il2Cpp is very fast, though don't fool yourself. One of the major reasons its fast on the Quest devices is due to the way the GC works and how agressive it is. This is good for game developers since it allows them to not worry about memory management while using il2cpp, but it does bring an issue to the table for modders: how do _we_ as modders ensure the GC does not delete the memory we are using? We come to the problem by having stored pointers that lose their references and get GC'ed. Another cause for this problem is when we instantiate an object but it gets GC'ed before it even finishes instantiating such as it is with `UnityEngine::ScriptableObject::CreateInstance<>();` for example.

We **have** to tell the GC there is a reference to it existing so it doesn't get freed. One way to do this is to declare the pointer, register and store it in `custom_types`. `custom_types` allocations are done by the GC, allowing it to recognize the references to other pointers and such. This might be tedious and annoying, especially when you only need to pass around the variable through functions or less places.

SafePtr is smart pointer similar to `shared_ptr` and `unique_ptr` which can alleviate this problem. It does this by forcing a reference in il2cpp so the GC never frees it. Once SafePtr goes out of scope and its destructor is called, this reference is freed and therefore gone, allowing the GC to free the pointer if there are no other references anymore. A SafePtr is fundamentally just a fancier `shared_ptr<>` as it does reference incrementing.

There are some caveats however that you should be aware of:

- This does NOT stop the pointer from being assigned nullptr, that is fundamentally different from it being freed. A pointer points to a region of memory in C++, which means that if it is freed, it still points to that memory. It is just now invalid, and there is currently no way to check if a pointer is still valid. When a pointer is assigned nullptr, that memory is not freed. It just means that the pointer now points to nothing in memory.
- Do not use SafePtr everywhere, especially in static variables with infinite lifetime if you're not careful. This is a perfectly good use case where you may want to ensure that the memory will never be freed, however this is technically a memory leak and if you're not careful you'll cause more problems than you'll solve.
- A SafePtr does not stop an explicit GC_free destruction as that forces memory to be freed unlike a GC checking if it has any references.

Be sure to use a SafePtr carefully, it is not a holy grace or a silver bullet. It is a specific tool for solving specific problems, be wise with it.

It should also be known that components and game objects shouldn't be used with SafePtr as those are fundamentally supposed to have their lifetimes tied to the game itself and destroyed when needed. Nothing is stopping you from using it, it's just bad practice fundamentally.

> From sc2ad's own words: "The 'extraneous' lifetime IS important to note, however, because SafePtr simply adds a reference to the instance, it does NOT stop explicit GC_Free calls to the instance from destroying it, nor does it ensure the instance will be garbage collected immediately after the SafePtr holding it is destroyed. It is possible that the game holds more references to the instance also within the GC-heap, in which case a SafePtr may not be necessary, and will slow down your code. 

> Also note that having three SafePtr instances for three different instances (for example) is worse as far as memory and performance goes than a custom type that simply has all three instances as fields. This is because each unique SafePtr performs an allocation and deallocation, whereas a single custom type will only perform this once."

For more specifics of SafePtr and it's nature, read up on [this wiki post by sc2ad, the author of SafePtr](https://github.com/sc2ad/beatsaber-hook/wiki/SafePtr)

### How to use SafePtr?
#### Methods available to use
- `SafePtr<T>(T* ptr);` Constructs a SafePtr keeping a reference to `T*` to stop it from being freed by implicit GC.
- `SafePtr<T>();` Constructs a SafePtr with no value, this allows for static or lazy initialization.
- `SafePtr<T>.emplace(T* ptr)` or `safePtrInstance = ptr` assign an existing SafePtr instance a pointer to keep alive.
- `if (safePtrInstance)` or `bool alive = safePtrInstance` return true if the SafePtr has an assigned pointer, false if not as is the case with the default constructor.
- `T* ptr = (T*) safePtrInstance` Returns the raw pointer held by SafePtr. Be wary as this means SafePtr is no longer in charge of keeping the pointer alive. This means if you continue using the `SafePtr` after it has been deconstructed, it is possible for it be to be freed. This is undefined behaviour.
- `SafePtr<T>.cast<ChildOrParentOfT>()` will cast the SafePtr to a `SafePtr<ChildOrParentOfT>`. This will throw an exception if it's not a valid cast. Preferrably use this when confident it is of the type or if else it's unexpected/bug behaviour.
- `SafePtr<T>.try_cast<ChildOrParentOfT>()` similar to cast however this return an optional with the casted value or `std::nullopt` if it's not a valid cast. Preferrably use this when you are aware of the possibility and are ready to handle when it's not castable.

#### Scenarios

Well one very common way to use SafePtr is as follows:
```cpp
// This is one common method that benefits from SafePtr
// I've encountered issues with this code since the GC
// doesn't find a reference to it WHILE you're instantiating it,
// causing it to be freed before you can even run the next line!
// This is one way to solve the problem
SafePtr<ScriptableObject> so(ScriptableObject::CreateInstance<ScriptableObject*>()); 
```
Another common use case might be as follows:
```cpp
// This initialization is designed to work like this.
// This is not assigned nullptr either, it's referred to as "empty"
static SafePtr<SomeClass> someClassPtr;

void hook(SomeOtherClass_Method, SomeOtherClass* self) {
  // Since we don't own this object, we don't explicitly control it's lifetime
  someClassPtr = SafePtr(self.someOtherClassPtr); // this can be done with .emplace instead of =, they're the same
}

void repeatedHookThatRunsEarlier(BClass_Method, BClass* self) {
  if (someClassPtr) {
    // this means that someClassPtr has been assisgned a pointer and is keeping it alive.
    std::optional<SafePtr<SomeClassChild>> someClassChildPtr = someClassPtr.try_cast<SomeClassChild>();

    if (someClassChildPtr) 
    {
      // It has been casted, yay!
    } 
    else 
    {
      // it's not a valid cast, likely due to it not actually being the type we asked to cast
    }
  } else {
    // it has not been assigned
  }
}
```

## Diagnosing crashes

On Quest you'll notice that it's far less forgiving for mistakes. Your mod will crash even for the slightest error, and sometimes you might even spend hours scratching your head, wondering why your code isn't working when it works in the original mod.

One of the best ways to understand the problem is by using tombstones. Tombstones are files created when the game crashes and are stored in `sdcard/Android/data/com.beatgames.beatsaber/files`. In conjuction with the tombstone, we can use a script that will try its best to parse it and refer back to your source code, making it look more like a C# stacktrace.
Simply use the `ndk-stack.ps1` script as follows:

`ndk-stack.ps1 /path/to/tombstone`

and then you'll have `path/to/tombstone_unstripped.txt`. The new log _should_ reference your source code now, assuming your mod was to blame for the crash.

You will also notice that they contain something along the lines of `signal (Code) (ERROR_NAME)` or nullptr dereference as example:
`signal 11 (SIGSEGV), code 1 (SEGV_MAPERR), fault addr 0x7753b8faf0`

You can read the [Android NDK docs](https://source.android.com/devices/tech/debug/native-crash) for more detailed information about crashes and their meaning:

Here are some common codes and their meanings (though they often depend)

**Do note, that you should be skeptical with these explanations. Some crashes can be caused for other reasons such as ACCERR caused by nullptr for example**

### Nullptr dereference (the equivalent of NullPointerException on C#)

It means that you tried to use a ptr which is equal to nullptr

A very simple way to check if it's null is:

```cpp
A* ptr = nullptr;
if (ptr) {
  // YAY IT'S NOT NULLPTR
} else {
  // NO IT'S NULLPTR!!!!
}
```

### Nullptr dereference/SEGV_MAPERR (but this code shouldn't be null, what gives?)

One problem we have in Quest modding is that while il2cpp may be compiled to C++, it does bring a garbage collector. This garbage collector is very agressive due to the limitations of the Quest devices. As modders, this causes a problem in which if we try to store references to pointers, there is no guarantee the GC (garbage collector) won't delete the memory later while we use it. So what's our solution to this problem?

#### Custom-types

One of the answers lies again in custom-types. Even if you don't plan on extending a C# class, you can use custom-types to store pointers to C# classes. The GC will recognize the fields and won't delete them from memory as long as the instance is still alive.

#### SafePtr to avoid GC

[SafePtr](#SafePtr)

### SEGV_MAPERR (similar to ClassCastException or NullPtr)

This usually means that you are assuming your variable is of type `B*` but in reality it is `A*`. Since you are assuming it's `B*`, the memory or functions you are trying to access does not exist therefore you get a MAP ERROR (memory isn't mapped as you'd expect).

SEGV_MAPERR strictly speaking is a crash that occurs due to a pointer dereference to an unmapped region of memory. [You can read the Android crash docs for more details.](https://source.android.com/devices/tech/debug/native-crash#lowaddress)

Another reason why this crash may occur is due to the [GC which is explained more thoroughly here](#nullptr-dereferencesegv_maperr-but-this-code-shouldnt-be-null-what-gives).

The best way to check your classes before assuming/casting them is to do a simple if check as follows:

```cpp
// right is the class
// left is the parent class
if (il2cpp_functions::class_is_assignable_from(classof(B*), objectA->klass)) {
    auto *objectB = reinterpret_cast<B*>(objectA);
}
```

However, it should be noted that if the GC yeets the memory in the pointer, it will usually throw a [SEGV_MAPERR and it cannot be at checked at runtime (yet)](nullptr-dereference-segv_mapperr-but-this-code-shouldnt-be-null-what-gives). Instead of checking to see if it has been yeeted, you should instead try to avoid it altogether by making a custom-type or SafePtr for it.

### SIGABRT (intentional crash)

There are two main ways this crash occur (though there are many others, these are the ones I'll focus on here)

- An exception was thrown in the C# code (either called by your mod or another mod) and wasn't caught
  - Usually contains a `cxa_throw` in the callstack (though it varies)
  - Can be caught using `Il2CppExceptionWrapper&` in C++ code; though you should most of the time attempt to fix what causses it, not avoid it.
- A C++ exception was thrown and wasn't caught
  - Usually contains a `std::terminate` in the callstack (though it varies)
  - Most of the time (though not limited to) done intentionally by `CRASH_UNLESS/SAFE_ABORT`

This crash is unique in that a tombstone alone won't help you (it will show you the location, but not the reason)

When the crash occurs, a crash message will be printed before it explaining why it crashed (maybe a C# exception for example). This message is not captured by the tombstone (and it's a system service so we can't do it ourselves). You will have to manually log using `adb logcat` and reproduce the bug to get the message.

You cannot fix it easily either. You either fix your CRASH\_UNLESS condition or fix your C# method call, depending what is the cause.
You _can_ try to catch the exception, though this is undefined behaviour and we don't support it _yet_ in the community.

### ACCERR

While this crash is a rare error, it can occur by attempting to access memory which is not allowed. The GC is a probable cause, though it can also be bad casting. For example, it can be caused by accesing memory in the kernel spacce or `.rodata` section, which is usually prohibited.

_This is rather vague since I'm not very familiar with the crash myself_

## Optimizations

There's many ways we can optimize our mod to be as fast, or even _faster_ than the PC counterpart.

### Recreate/Replace functions

If the function is simple, such as a math method, you are _**encouraged**_ to use a C++ implementation of it. Either by the `std` library or rewriting it yourself. While the code is functionally similar, you avoid the overhead il2cpp incurs and these minor changes make a big difference in the long run.

### Use C++ alternative types

This is similar to the previous method, except instead of methods we use C++ native types or structs.

#### List/Arrays

We may use `std::vector` instead of a C# array list or array.

#### Hash Maps

We can use a `std::unordered_map` for hash maps (though they are not insertion order)

### C++ threads

Do not use if you need to access Unity/il2cpp in the thread
TODO:

### Lazy Initialize

In C++, we have the ability of doing the following:

```cpp
void func() {
  static auto someExpensiveVar = someExpensiveMethod();
}
```

What does this do? It initializes the variable the first time the method runs, and keeps it for every other run. This makes the variable constant however it is very helpful for improving performance. Avoid using it unless you are certain the lifetime of the object lasts as long or longer than you need.
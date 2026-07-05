# Engine changelog

## Klemmgine 2.0.0-dev2

### Graphics

- Reworked graphics code to no longer call OpenGL directly.
- Optimized OpenGL API calls for a slight performance increase.

### Editor

- Script editor improvements:
	- Added file search, using Ctrl+F.
	- Added keyboard tab switcher, accessed using Ctrl+Tab, which can be navigated using arrow keys.
	- Improved the controls of auto complete results.
	- Added a shortcut to close the selected tab, Ctrl+W.

### Fixes

- Fixed a crash when calling the virtual functions of native SceneObjects (such as update()) directly from a script.

## Klemmgine 2.0.0-dev1

### General

- Added the `SceneManage` class, which controls the logic of a scene itself.
- Added logging to files. Always enabled in the editor. For non-editor builds writing log files is enabled with `-writeLogs`.
- Added code analysis using MSVC's /analyze flag.
- Object reflection system now works for more classes than just SceneObjects.

### Graphics

- Added point lights, `LightObject` object class and `LightComponent`.
- Moved all graphics members from the `Scene` class into it's own `GraphicsScene` class.
- Added frustum culling using a dynamically calculated bounding volume hierarchy.
- Changed `ObjectComponents` to only recalculate their world transform if necessary.
- Added `#unlit` shader preprocessor directive that skips sending shading information to the
  shader to save performance.
- Optimized passing shadow information to the same shader repeatedly.
- Added `BillboardComponent`.
- Split native `Scene` class into `Scene` and `GraphicsScene` to separate rendering logic.
- Fixed rendering bugs with ambient occlusion.
- Fixed rendering bugs with shadows.

### Editor

- Added floating Scripts panel.
- Added horizontal script tabs.
- Improvements to how keyboard shortcuts are handled.
- Added in editor tooltips for script classes.
- Added more object icons and changed some icons to not look like radio buttons when they don't function like one.
- Added WIP option to launch the game in a separate process.
- The editor binary can now launch the engine without the editor subsystem, using the `-noEditor` launch argument.
- Added options for clearing the console or doing it automatically when a game starts.
- Added default cube and plane meshes as models for selection.
- Added a prompt for a file name when creating assets instead of always creating an asset with a default name.
- List embedded shaders as options for selecting a shader in materials.
- Fixed scene camera starting at 0,0,2 instead of 0,2,0.
- Added an option to show bounding box of models in the model editor.
- Improved how "Open in file explorer" option opens the folder on windows.
- Show the "Open in file explorer" option consistently in the tree view.

### Script

- Added new script functions:
	- Bindings for newly added `LightComponent`, `SoundComponent` and `BillboardComponent`.
	- Added `engine::assets` module, currently only for basic model data reading.
	- Added `engine::serialize` module, containing functions for reading and writing Text, Binary and JSON files
	  using the engine's serialization functions.
	- Added `engine::sound` module, containing functions for playing sounds with a sound context.
	- Added the `engine::Transform` class, equivalent to the native `Transform` class, storing a 4x4 matrix.
	- Added methods to `engine::Scene`:
		- `getObjectByName<T>()`:
			- Searches for an object with type T and the given name.
		- `getSoundContext()`
			- Gets the scene's sound context.
	- Added `engine::isPlaying()`.
	- Added `virtual SceneObject.onBeginPlay()` which is called after `begin()`, when the game is playing.
	- Added methods to `engine::CameraComponent`:
		- `screenToWorldDirection()`: Converts screen coordinates into a 3D vector pointing in the matching direction
		  in 3D space from that camera's perspective.
		- `worldPositionToScreen()`: Converts world coordinates to screen coordinates from that camera's perspective.

- Reworked existing script functions:
	- `engine::input::isKeyDown()` is now split into 3 functions:
		- `isKeyDown()` returns true if the key was pressed this frame.
		- `isKeyHeld()` returns true if the key is currently pressed.
		- `isKeyUp()` returns true if the key was released this frame.
- Experimental support for just-in-time compilation, enabled with the `-useJIT` launch argument.

### Sound

- Added sound. Currently only WAV files are supported for playback.
- Added reverb volumes.
- Sounds can either be loaded using a SoundComponent, or played directly using playSound() and playSoundAt().
- 

### Debugging

- Added debug rendering features internally.

### Fixes

- Fixed the engine trying to load plugins from the wrong path when using the editor.
- Fixed the editor theme sometimes being reloaded incorrectly.
- Fixed "Cannot add an item of type '' into the scene" notification appearing when it shouldn't.
- Fixed CLRF/LF issues when trying to read JSON files.
- Fixed many script bugs and crashes.
- Fixed `rpath` issues on Linux, so executables will now use the shared library files included with the engine by default.
- Updated UI library to a newer version, fixing issues with rendering, scrolling and text input/selection.
- Fixed crashes appearing when reloading scripts.
- Fixed multiple crashes appearing when pressing "Open Project" in the editor after a project is already
  loaded and opening another one.
- Fixed a rare crash on windows when closing a main window.
- Fixed script constructors being able to be called when they shouldn't be.

## Klemmgine 2.0.0-dev0

First development release.
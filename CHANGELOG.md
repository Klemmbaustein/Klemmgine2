# Engine changelog

## Klemmgine 2.0.0-dev1

### Graphics

- Added point lights, `LightObject` object class and `LightComponent`.
- Moved all graphics members from the `Scene` class into it's own `GraphicsScene` class.
- Added frustum culling using a dynamically calculated bounding volume hierarchy.
- Changed `ObjectComponents` to only recalculate their world transform if necessary.
- Added `#unlit` shader preprocessor directive that skips sending shading information to the shader to save performance.
- Optimized passing shadow information to the same shader repeatedly.

### Debugging

- Added debug rendering features.

### Fixes

- Fixed the engine trying to load plugins from the wrong path when using the editor.

## Klemmgine 2.0.0-dev0

First development release.
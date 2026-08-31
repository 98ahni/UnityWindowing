## Unity Windowing  
---  

A windowing library for Unity because sometimes you just want more windows.

### Features
#### Simple by design
This plugin was made to feel like a part of Unity. All you need to do to create a window is to add the `WindowRenderer` component to an object with a `Camera` and hit Play. The settings in the Inspector can be changed during play mode and it's easy for your code to react to changes and input with an event driven design. 
#### User input
Any user input made to a window is handed to a simulation layer that forwards the inputs to the Unity InputSystem ~or the legacy Input~ (coming soon). This forwarding also enables UI components to be interacted with right out of the box. The only extra step is that each active window that wants UI mouse input should have their cameras set to different displays as this is how Unity handles UI inputs. 
#### Cross-platform
The plugin works with any desktop graphics API (except OpenGL) supported on Windows, macOS and Linux.  
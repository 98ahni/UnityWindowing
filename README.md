# UnityWindowing
A Unity plugin to allow the creation of multi-window applications and games!

### Features
#### Simple by design
This plugin was made to feel like a part of Unity. All you need to do to create a window is to add the `WindowRenderer` component to an object with a `Camera` and hit Play. The settings in the Inspector can be changed during play mode and it's easy for your code to react to changes and input with an event driven design. 
#### User input
Any user input made to a window is handed to a simulation layer that forwards the inputs to the Unity InputSystem ~or the legacy Input~ (coming soon). This forwarding also enables UI components to be interacted with right out of the box. The only extra step is that each active window that wants UI mouse input should have their cameras set to different displays as this is how Unity handles UI inputs. 
#### Cross-platform
The plugin works with any desktop graphics API (except OpenGL) supported on Windows, macOS and Linux.  
\* I don't have a mac or Linux machine and so I can't compile or test for those platforms. See [Contributing](.#Contributing) for details on how to compile. 

### Install Instructions
TBD

### Contributing
- The Windows version was compiled using Visual Studio 2022 and C/C++ language version set to 23. It also has the macro `_GLFW_WIN32` defined and unicode turned off. 
- The Linux version should use similar settings and have `_GLFW_WAYLAND` and/or `_GLFW_X11` defined. It would be nice to have both in one build, chosen at runtime, but I don't know if that's possible. 
- The macOS version can only be compiled using clang with the macro `_GLFW_COCOA` defined.
- OpenGL is included but (at least on Windows) doesn't work yet since it's not thread safe. 
- If you need to force a rendered to be included/excluded you can define these in the compiler to either 1 or 0:
- - `HAS_DX11` - DirectX 11
- - `HAS_DX12` - DirectX 12
- - `HAS_VK` - Vulcan
- - `HAS_METAL` - Metal
- - `HAS_GL` - OpenGL

### LICENCE TL;DR
- You can use this plugin for commercial or non-commercial works, regardless of your team size.
- You may modify source code to suit your needs.
- You may **not** distribute a compiled version, stand-alone or part of a larger product, without making the source code of your modified version available to the public under the GPL licence.
- You may **not** misrepresent your contributions or impersonate any authors that has made contributions to this project.
- Attributions **must** be present and clearly visible with any distribution, partial or whole, source or compiled. For example in the credits or an authors file like the one in this project.
- You may **not** remove or edit any existing copyright notices or attributions present in any file.
- You may add copyright a copyright notice to any file you edit with a statement of what you changed.
- No author can be held liable for unexpected program behavior or bugs and there is no warranty.
- The Unity, GLFW and GLEW files come with their own licences. 

### AI Disclosure
The code for the rendering backends was written mostly by AI. AI was also used to aid in the debugging of these backends. I used it to help my understanding of Direct3D 12 and to translate it to different APIs once it was working. I do not support replacing knowledge with generative AI but, at the time of this project, I have little desire to learn the intricacies of all of these APIs for something this simple.  
If you also don't condone replacing knowledge with AI, sign [this petition](https://stoptheracetoreplace.org/).

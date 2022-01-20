![In-Engine Screenshot](Sample_Img/CGFinal_Screen.png)

# CGFinal-A_Calm_Neighborhood

A simple project that worked as the last practical exam of my Computer Graphics course 
where it was required that a project contained at least the principles of OpenGL's graphics rendering pipeline,
which are comprised of: 
* `Lighting` (in its static form, shadows not required),
* `Shading` (gouraud or phong, implemented in shaders)
* `Texturing`
* `Camera` (to look around the environment)

 
I decided to go the extra mile and also implemented multiple light types,
moving lights, a skybox, a ton of static lights, vertex shaders that allow
the renderer to manipulate a gigantic amount of grass blades and tree leaves
without tanking performance and also implemented a lighting model closer to PBR
through shaders that support specular, height, emission and normal maps besides
the default diffuse texture. (maybe a few other things, but i can't remember).


I'm sure a lot of optimizations could be done to make it render much faster,
but this project was a sole effort and had around one month of development time
so compromises had to be made.

# So, how does one compile such a project?

For the linux branch, and the one i'm currently on, the dependencies and steps to compile are a bit more complicated, but
are guaranteed to work compared to the Windows branch at least on Arch/Manjaro.

First you will need the following:

* The Code::Blocks IDE, since it's the one i made this project on.
* libassimp to load the models (Arch/Manjaro has an assimp package in the AUR).
* glfw3 which is the main renderer (Arch/Manjaro has a glfw package in the AUR, it's glfw3).
* A graphics card with support for OpenGL4.3 and at least 256MB of VRAM.

While the GL 4.3 requirement is a bit high for such a project, you can set the version as low as 3.3 in the main.cpp file and 
not face any major problems.

Ok, so with all that done, you'll only need to open the project in Code::Blocks, right-click in the "CG-Final" project right
below the "Workspace" dropdown, select "Build Options", go to the "Linker settings" tab and add the following links if they're
not there: glfw, dl, assimp. With those in place, you should be ready to Compile and Run the project. To get the compiled 
executable running without the IDE, you can just drag it from Compiled/Release to the project's root (alongside "main.cpp"),
open a Terminal in that folder and run it by typing in "./CGFinal" and pressing enter. If you have the libs above installed
correctly, it should work fine.

# After having the project running, there's some ways to control it

It will capture your mouse by default, but you can alt+tab to remove it's focus. The following keys are used by the camera:

* WASD -> Movement
* Shift -> Move faster
* Ctrl -> "Crouch" (basically, move slower, i was trying to make a mini game at the start)
* Esc -> Exit

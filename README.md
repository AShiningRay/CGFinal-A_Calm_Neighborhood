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

For the linux branch, and the one i'm currently on, the dependencies and steps to compile will be coming soon.
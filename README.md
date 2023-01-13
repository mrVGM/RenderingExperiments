# Render Experiments

A project, I made to play around with shaders and visual effects. It is entirely written in C++ and uses DirectX 12 to draw stuff on the screen.

One can compile the project, using Visual Studio 2022 and CMake 3.8. Binaries are also available in the release section of the repository.
To run them, just download and unzip the archive and double-click on the *run.bat* file. Keep in mind, that you should be running a 64-bit Windows operating system.

The first things, I tried rendering, are some real-time procedurally generated clouds, inspired by some YouTube videos and these articles:
- [The Real-time Volumetric Cloudscapes of Horizon: Zero Dawn](https://advances.realtimerendering.com/s2015/The%20Real-time%20Volumetric%20Cloudscapes%20of%20Horizon%20-%20Zero%20Dawn%20-%20ARTR.pdf).
- [Physically Based Sky, Atmosphere & Cloud Rendering](https://www.ea.com/frostbite/news/physically-based-sky-atmosphere-and-cloud-rendering)
- [Oz: The Great and Volumetric](http://magnuswrenninge.com/wp-content/uploads/2010/03/Wrenninge-OzTheGreatAndVolumetric.pdf)

Here is a render preview:

![Clouds Preview](images/clouds.gif)

Here is also a render of the Mandelbulb, made using the method of raymarching:

![Clouds Preview](images/mandelbulb.gif)

My next goals with this project are to implement deffered shading and be able to load custom geometry from COLLADA files.

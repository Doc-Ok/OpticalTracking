OpticalTracking
===============

Dump of Oculus Rift DK2 optical tracking code as-is, including a current snapshot of Vrui-3.2-001.

Important note: Vrui-3.2-001 is obsolete. Do not use the Vrui sources included in this repository (they will be removed as time allows). Instead, build and install Vrui from its official GitHub repository, https://github.com/vrui-vr/vrui.

The OpticalTracking package is compatible with Vrui-14.0 and newer.

Build Instructions
------------------

1. Build and install Vrui from https://github.com/vrui-vr/vrui.

While building Vrui, note the location of the build system directory that is printed early on during the build:
```
...
Build system installation directory: <path to Vrui build system>
...
```

2. Build the optical tracking package
```
$ cd <directory containing this README.md file>
$ cd OpticalTracking
```

Then run make with the location of Vrui's build system directory as printed in step 1:
```
$ make VRUI_MAKEDIR=<path to Vrui build system>
```

Alternatively, edit the makefile and set VRUI_MAKEDIR to the location of Vrui's build system, and then simply run make:
```
$ make
```

3. Run the optical tracking main program:
```
$ ./bin/LEDFinder <video device name>
```

where <video device name> is the Video-4-Linux device name of the tracking
camera. This *should* be "Camera DK2", in which case the command line is:
```
$ ./bin/LEDFinder "Camera DK2"
```

There need to be .ldp (lens distortion) and .icp (intrinsic camera parameter)
files with the same name as the video device in the current directory. These need
to be created by per-device calibration, but I included the files from my camera
into the distribution. They might be a good-enough approximation to get working.

Once in the program, enable the DK2's LEDs by selecting, in the "Rift LED Control"
dialog, the buttons for "Modulate" and "Flash LED IDs," then drag the "Pattern"
slider to 1, and then select the "Enable" button. This will turn on the camera,
and should quickly label all visible LEDs and draw the wireframe 3D HMD model,
which should fit the observed LEDs well.

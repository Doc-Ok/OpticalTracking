OpticalTracking
===============

Dump of Oculus Rift DK2 optical tracking code as-is, including a current snapshot of Vrui-3.2-001.

To build:

1. Build Vrui, which is included at the root level of the repository. Install
   the required and optional dependencies, and run "make" followed by "make
   install":

Dependencies:

* On Ubuntu:
$ sudo apt-get install build-essential zlib1g-dev mesa-common-dev libgl1-mesa-dev
  libglu1-mesa-dev libudev-dev libusb-1.0-0-dev libpng12-dev libjpeg-dev
  libtiff5-dev libasound2-dev libdc1394-22-dev libspeex-dev libogg-dev
  libtheora-dev libbluetooth-dev libopenal-dev

* On Fedora:
$ sudo yum install zlib-devel mesa-libGL-devel mesa-libGLU-devel libudev-devel
  libusb1-devel libpng-devel libjpeg-devel libtiff-devel alsa-lib-devel
  libv4l-devel libdc1394-devel speex-devel libogg-devel libtheora-devel
  bluez-libs-devel openal-soft-devel

Building:

$ cd OpticalTracking-1.0
$ make
$ make install

By default, Vrui installs itself into ~/Vrui-3.2. To place it, say, in
  /usr/local/Vrui-3.2, run instead:

$ make INSTALLDIR=/usr/local/Vrui-3.2
$ make INSTALLDIR=/usr/local/Vrui-3.2 install

To speed up the build, you can run make in parallel. For example, to use 8
processes appropriate for a hyperthreaded 4-core CPU, add -j8 to make's command
line.

2. (Optional) Build Vrui example programs

$ cd ExamplePrograms
$ make
$ ./bin/ShowEarthModel
(shows a spinning globe)

If you installed Vrui in a non-default location, such as /usr/local/Vrui-3.2,
run instead:

$ make VRUI_MAKEDIR=<Vrui_directory>/share/make
$ ./bin/ShowEarthModel

where <Vrui_directory> is the path passed as INSTALLDIR during Vrui installation.

2. Build the optical tracking package

$ cd OpticalTracking
$ make

Again, to use Vrui from a non-default location, do as above.

3. Run the optical tracking main program:

$ ./bin/LEDFinder <video device name>

where <video device name> is the Video-4-Linux device name of the tracking
camera. This *should* be "Camera DK2", in which case the command line is:

$ ./bin/LEDFinder "Camera DK2"

There need to be .ldp (lens distortion) and .icp (intrinsic camera parameter)
files with the same name as the video device in the current directory. These need
to be created by per-device calibration, but I included the files from my camera
into the distribution. They might be a good-enough approximation to get working.

Once in the program, enable the DK2's LEDs by selecting, in the "Rift LED Control"
dialog, the buttons for "Modulate" and "Flash LED IDs," then drag the "Pattern"
slider to 1, and then select the "Enable" button. This will turn on the camera,
and should quickly label all visible LEDs and draw the wireframe 3D HMD model,
which should fit the observed LEDs well.

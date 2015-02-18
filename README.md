# Panoptic

Copyright &copy; 2011-2014 [Robert Kooima](http://kooima.net)

Panoptic is a giga-pixel omni-stereoscopic panorama viewer and a planetary data exploration tool. It embeds the [Spherical Cube Map](https://github.com/rlk/scm) library within a [Thumb](https://github.com/rlk/thumb) application, giving an SCM renderer with broad support for virtual reality devices and head-mounted displays.

## Dependencies

The Panoptic build requires the presence of [Spherical Cube Map](https://github.com/rlk/scm) and [Thumb](https://github.com/rlk/thumb) in adjacent directories.

	$ ls
	panoptic
	scm
	thumb

## Build

Panoptic may be build with either a Debug or Release configuration. The adjacent Thumb and SCM source trees *must* be build with the same configuration. In all cases, debug builds are placed in `Debug` while release builds are placed in `Release`, so these configurations may cleanly coexist.

### Linux and OS X

To build `Release/panoptic` under Linux or OS X:

	make

To build `Debug/panoptic`:

	make DEBUG=1

### Windows

To build `Release\panoptic.exe` under Windows, use the Visual Studio project or the included `Makefile.vc`:

	nmake /f Makefile.vc

To build `Debug\panoptic.exe`:

	nmake /f Makefile.vc DEBUG=1

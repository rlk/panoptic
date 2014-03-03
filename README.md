# SCM View

Copyright &copy; 2011-2014 [Robert Kooima](http://kooima.net)

The SCM View repository contains a series of experiments in the rendering of [SCM](https://github.com/rlk/scm) data using the [Thumb](https://github.com/rlk/thumb) interactive application framework. These include a giga-pixel omni-stereoscopic panorama viewer and a planetary data exploration tool, both of which benefit from Thumb's broad support for virtual reality devices and head-mounted displays.

The SCM View build requires the presence of [Thumb](https://github.com/rlk/thumb) in an adjacent directory.

	$ ls
	thumb
	scmview

The repo has a submodule ([SCM](https://github.com/rlk/scm)) with a submodule ([util3d](https://github.com/rlk/util3d)). These need to be explicitly added to a fresh clone. To do so...

    git submodule update --init --recursive

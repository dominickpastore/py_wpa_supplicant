# pyi\_wpa\_supplicant
CPython module for wpa\_supplicant UNIX domain socket interface

This project provides a module named `wpa_supplicant` that implements an
object-oriented Python interface to wpa\_supplicant using
[wpa\_ctrl.h](https://w1.fi/wpa_supplicant/devel/ctrl_iface_page.html), similar
to the wpa\_cli program that wpa\_supplicant comes with.

**Note:** This project uses submodules. Make sure you use the `--recursive` flag
when you clone. If you did not, you can also use
`git submodule update --init --recursive` to fetch the submodules for an
already-cloned repository. For more tips, see [Github's blog post on
submodules](https://github.blog/2016-02-01-working-with-submodules/).

## Requirements

### Python Version

This is written for Python 3 and tested with 3.5. It might work with older
versions or Python 2, but I have not tested this.

It uses the CPython C/C++ API, so it is likely tied to that interpreter. This
should not be a problem unless you are using PyPy, Jython, etc.

### Operating System

I developed this for use on Linux (specifically, Debian, but it should be
distro-agnostic). It should build on that platform out of the box. I believe
building for other platforms should be possible with the following changes:

 * The os\_unix.c source file needs to be changed to the appropriate platform in
   setup.py
 * The preprocessor defines need to be changed to the appropriate platform in
   setup.py
 * `IFace_recv` currently uses the `poll` system call. This needs to be replaced
   on platforms where this is not available (e.g. Windows). The associated
   support code (for detecting errors in `poll`) needs to be changed, too.

### Prerequisites

Make sure you have the Python development headers available or you will not be
able to build this. In Debian-based distributions, you can install the
`python3-dev` package to obtain these.

## Build and Install

This package uses distutils. Make sure you have satisfied all the requirements
aove, then the standard build/install instructions apply:

    python3 setup.py build
    sudo python3 setup.py install

**Q:** Why distutils instead of setuptools?

**A:** Because I just needed to make sure this builds and works before
incorporating the code into a larger project. Docs on how to build a native
extension using distutils were easy to find. Docs on doing this with setuptools,
not so much.

## Usage

Once the module is built and installed, you should be able to use it in your
Python programs. Here is an example:

## License

This project is licensed under a BSD license. See LICENSE for more details.

This code makes use of wpa\_supplicant code from the hostap repository. That
code is distributed separately as a submodule. It is also under a BSD license.
See [the wpa\_supplicant website](https://w1.fi/wpa_supplicant/).

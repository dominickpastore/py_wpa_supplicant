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

**A:** Because I just needed to make sure this built and worked before
incorporating the code into a larger project. At the time I wrote this, I was
new to both tools, and docs for distutils were easier to find.

## Usage

Once the module is built and installed, you should be able to use it in your
Python programs. Here is an example:

```
#!/usr/bin/env python3

import wpa_supplicant

request_iface = wpa_supplicant.IFace('/run/wpa_supplicant/wlp4s0')
event_iface = wpa_supplicant.IFace('/run/wpa_supplicant/wlp4s0')
event_iface.attach()
print(request_iface.request("SCAN"))
while True:
    ev = event_iface.recv(-1)
    if ev.startswith('CTRL-EVENT-SCAN-RESULTS', 3):
        break
print(request_iface.request("SCAN_RESULTS"))
request_iface.close()
event_iface.detach()
event_iface.close()
```

This imports the module, creates two control interfaces (one for requests and
one for events), does a scan, waits for the "CTRL-EVENT-SCAN-RESULTS" event
signaling that the scan is complete, fetches and prints the scan results, then
cleans up.

Creating separate control interfaces for commands and events is the recommended
way to use this interface. The interface that is attached will receive events
and the other is used for commands.

The alternate way is to use the same interface for both. Simply attach the
interface and then use both `recv()` and `request()` methods on the same
interface. You must provide a callback for `request()` in case events come while
waiting for the response. This has the potential to lose events or the response
if the callback raises an exception. It is also less performant.

The `IFace` type is all that this module provides--there are no other objects at
the module level (types, functions, or data). The `IFace` type provides the
following methods:

 * `__init__()`
 * `__init__(ctrl_path)`
 * `__init__(ctrl_path, cli_path)`
 * `open(ctrl_path)`
 * `open(ctrl_path, cli_path)`
 * `close()`
 * `attach()`
 * `detach()`
 * `pending()`
 * `recv()`
 * `recv(timeout)`
 * `request(cmd)`
 * `request(cmd, callback)`

This is very close to the interface provided by
[wpa\_ctrl.h](https://w1.fi/wpa_supplicant/devel/wpa__ctrl_8h.html). The
differences are:

 * This is an object oriented interface. The constructors can optionally open
   the interface as well (if arguments are provided).
 * Responses (for `recv()` and `request()` are returned instead of using
   pass-by-reference parameters.
 * Errors are indicated through exceptions, not return codes. Timeouts raise
   TimeoutError, while other I/O errors raise IOError. Other non-I/O exceptions
   are also possible (e.g. when parameters are the wrong type).
 * `recv()` can accept a timeout. If the timeout is zero, it will return
   immediately. If the timeout is negative, it will wait forever.

See wpa\_supplicant.c and the link above for more detail about how to use these
functions.

## License

This project is licensed under a BSD license. See LICENSE for more details.

This code makes use of wpa\_supplicant code from the hostap repository. That
code is distributed separately as a submodule. It is also under a BSD license.
See [the wpa\_supplicant website](https://w1.fi/wpa_supplicant/).

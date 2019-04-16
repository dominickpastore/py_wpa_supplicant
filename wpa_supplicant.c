/*
 * py_wpa_supplicant - A Python module to interface with wpa_supplicant through
 * its control interface (the default interface, not DBus).
 *
 * Copyright (c) 2019 Dominick C. Pastore
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <Python.h>
#include "wpa_supplicant/wpa_ctrl.h"

/******************************************************************************
 * Create the IFace type (a.k.a. "class" in C/C++ terminology)
 ******************************************************************************/

typedef struct {
    PyObject_HEAD
    struct wpa_ctrl *ctrl;
    int attached;
} IFaceObject;

/*
 * __init__()
 * __init__(ctrl_path)
 * __init__(ctrl_path, cli_path)
 *
 * Initializes a control interface and optionally opens it
 */
static int IFace_init(IFaceObject *self, PyObject *args, PyObject *kwds) {
    // Initialize control interface to NULL in case we don't open it
    self->ctrl = NULL;
    self->attached = 0;

    // Get arguments
    const char *ctrl_path = NULL;
    const char *cli_path = NULL;
    static char *kwlist[] = {"ctrl_path", "cli_path", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ss", kwlist,
                &ctrl_path, &cli_path)) {
        return -1;
    }

    // Open control interface if path was given
    if (ctrl_path != NULL) {
        if (cli_path != NULL) {
            self->ctrl = wpa_ctrl_open2(ctrl_path, cli_path);
        } else {
            self->ctrl = wpa_ctrl_open(ctrl_path);
        }
        if (self->ctrl == NULL) {
            PyErr_SetString(PyExc_IOError, "Could not open interface");
            return -1;
        }
    }

    return 0;
}

/*
 * open(ctrl_path)
 * open(ctrl_path, cli_path)
 *
 * Open the control interface with wpa_ctrl_open() and wpa_ctrl_open2().
 */
static PyObject * IFace_open(IFaceObject *self, PyObject *args) {
    // Check if already open
    if (self->ctrl != NULL) {
        PyErr_SetString(PyExc_IOError, "Interface already open");
        return NULL;
    }

    // Get the path(s) from the arguments
    const char *ctrl_path = NULL;
    const char *cli_path = NULL;
    if (!PyArg_ParseTuple(args, "s|s:open", &ctrl_path, &cli_path)) {
        return NULL;
    }

    // Open control interface
    //TODO release GIL?
    if (cli_path != NULL) {
        self->ctrl = wpa_ctrl_open2(ctrl_path, cli_path);
    } else {
        self->ctrl = wpa_ctrl_open(ctrl_path);
    }
    //TODO acquire GIL?
    if (self->ctrl == NULL) {
        PyErr_SetString(PyExc_IOError, "Could not open interface");
        return NULL;
    }

    Py_RETURN_NONE;
}

/*
 * attach()
 *
 * Register as an event monitor with wpa_ctrl_attach().
 */
static PyObject * IFace_attach(IFaceObject *self) {
    // Check if interface is open yet
    if (self->ctrl == NULL) {
        PyErr_SetString(PyExc_IOError, "Interface not open");
        return NULL;
    }

    // Check if interface is attached yet
    if (self->attached == 1) {
        PyErr_SetString(PyExc_IOError, "Interface already attached");
        return NULL;
    }

    //TODO release GIL?
    int result = wpa_ctrl_attach(self->ctrl);
    //TODO acquire GIL?

    if (result == -1) {
        // Failure
        PyErr_SetString(PyExc_IOError, "Could not attach");
        return NULL;
    } else if (result == -2) {
        // Timeout
        PyErr_SetString(PyExc_TimeoutError, "Timeout while attaching");
        return NULL;
    }

    self->attached = 1;
    Py_RETURN_NONE;
}

/*
 * detach()
 *
 * Unregister as an event monitor with wpa_ctrl_detach().
 */
static PyObject * IFace_detach(IFaceObject *self) {
    // Check if interface is open yet
    if (self->ctrl == NULL) {
        PyErr_SetString(PyExc_IOError, "Interface not open");
        return NULL;
    }

    // Check if interface is attached yet
    if (self->attached == 0) {
        PyErr_SetString(PyExc_IOError, "Interface not attached");
        return NULL;
    }

    //TODO release GIL?
    int result = wpa_ctrl_detach(self->ctrl);
    //TODO acquire GIL?

    if (result == -1) {
        // Failure
        PyErr_SetString(PyExc_IOError, "Could not detach");
        return NULL;
    } else if (result == -2) {
        // Timeout
        PyErr_SetString(PyExc_TimeoutError, "Timeout while detaching");
        return NULL;
    }

    self->attached = 0;
    Py_RETURN_NONE;
}

/*
 * close()
 *
 * Close the control interface with wpa_ctrl_close().
 */
static PyObject * IFace_close(IFaceObject *self) {
    // Check if interface is open yet
    if (self->ctrl == NULL) {
        PyErr_SetString(PyExc_IOError, "Interface not open");
        return NULL;
    }

    wpa_ctrl_close(self->ctrl);
    self->ctrl = NULL;

    Py_RETURN_NONE;
}

/*
 * pending()
 *
 * Check whether there are pending event messages with wpa_ctrl_pending() and
 * return True or False.
 */
static PyObject * IFace_pending(IFaceObject *self) {
    // Check if interface is open yet
    if (self->ctrl == NULL) {
        PyErr_SetString(PyExc_IOError, "Interface not open");
        return NULL;
    }

    // Check if interface is attached
    if (self->attached == 0) {
        PyErr_SetString(PyExc_IOError, "Interface not attached");
        return NULL;
    }

    int result = wpa_ctrl_pending(self->ctrl);

    if (result == 0) {
        Py_RETURN_FALSE;
    } else if (result == 1) {
        Py_RETURN_TRUE;
    } else {
        // Failure
        PyErr_SetString(PyExc_IOError, "Could not check for pending event messages");
        return NULL;
    }
}

/*
 * recv()
 *
 * Receive a pending event message with wpa_ctrl_recv() and return it as a
 * string.
 */
static PyObject * IFace_recv(IFaceObject *self) {
    // Check if interface is open yet
    if (self->ctrl == NULL) {
        PyErr_SetString(PyExc_IOError, "Interface not open");
        return NULL;
    }

    // Check if interface is attached
    if (self->attached == 0) {
        PyErr_SetString(PyExc_IOError, "Interface not attached");
        return NULL;
    }

    char buf[4096];
    size_t len = sizeof(buf) - 1;
    //TODO release GIL
    int result = wpa_ctrl_recv(self->ctrl, buf, &len);
    //TODO acquire GIL
    if (result < 0) {
        // Failure
        PyErr_SetString(PyExc_IOError, "Could not receive event message");
        return NULL;
    }

    return PyUnicode_FromStringAndSize(buf, len);
}

/*
 * Callback used for IFace_request
 */
PyObject *req_callback = NULL;
void request_callback(char *msg, size_t len) {
    // The callback can't return anything, so we need to manually check if a
    // prior invocation of the callback raised an exception before we call more
    // Python code.
    if (!PyErr_Occurred()) {
        PyObject *args;
        PyObject *result;
        // Build tuple with message
        args = Py_BuildValue("(s#)", msg, len);
        if (args == NULL) {
            return NULL;
        }
        // Call callback
        result = PyObject_CallObject(req_callback, args);
        Py_DECREF(args);
        if (result == NULL) {
            // Normally we'd return here but we can't since this is a callback
            // and the caller doesn't check. See the top of this function and
            // the comment in IFace_request for how we are handling this.
            return;
        }
        Py_DECREF(result);
    }
}

/*
 * request(cmd)
 * request(cmd, msg_cb)
 *
 * Send a command with wpa_ctrl_request() and return the reply as a string.
 *
 * Optionally, msg_cb can be a callback to be called if event messages are
 * received while waiting for the command response. This can only happen if the
 * interface has been attach()ed.
 *
 * Alternatively, programs can simply use separate interfaces for commands and
 * event messages. This is the recommended way due to performance reasons and
 * because an exception in the callback function will cause further event
 * messages and the response to be lost.
 */
static PyObject * IFace_request(IFaceObject *self, PyObject *args) {
    // Check if interface is open yet
    if (self->ctrl == NULL) {
        PyErr_SetString(PyExc_IOError, "Interface not open");
        return NULL;
    }

    // Get the arguments
    const char *cmd = NULL;
    int cmd_len;
    PyObject *callback;
    if (!PyArg_ParseTuple(args, "s#|O:request", &cmd, &cmd_len, &callback)) {
        return NULL;
    }

    // Send request and get response
    char buf[4096];
    size_t len = sizeof(buf) - 1;
    int result;
    if (callback == NULL) {
        //TODO release GIL
        result = wpa_ctrl_request(self->ctrl, cmd, cmd_len, buf, &len, NULL);
        //TODO acquire GIL

        if (result == -1) {
            // Failure
            PyErr_SetString(PyExc_IOError, "Could not send command");
            return NULL;
        } else if (result == -2) {
            // Timeout
            PyErr_SetString(PyExc_TimeoutError, "Timeout while sending command");
            return NULL;
        }

        return PyUnicode_FromStringAndSize(buf, len);
    } else {
        if (!PyCallable_Check(callback)) {
            PyErr_SetString(PyExc_TypeError, "Callback must be callable");
            return NULL;
        }
        // We need not INCREF nor DECREF because although we are storing the
        // callback in a global function, we are still discarding it before
        // we return to the caller.

        // Note that the next few lines are not thread-safe. Luckily, the GIL
        // should protect us. Less fortunately, it's hard to release the GIL
        // because we have a Python callback we might use, which might return an
        // exception we need to keep. And holding the GIL is going to hurt
        // performance. We should avoid using the callback version of this
        // function.
        req_callback = callback;
        result = wpa_ctrl_request(self->ctrl, cmd, cmd_len, buf, &len,
                request_callback);
        req_callback = NULL;
        // The Python callback might have raised an exception. This is our only
        // way to detect. See comments in request_callback.
        if (PyErr_Occurred()) {
            return NULL;
        }
        // End of thread unsafeness

        if (result == -1) {
            // Failure
            PyErr_SetString(PyExc_IOError, "Could not send command");
            return NULL;
        } else if (result == -2) {
            // Timeout
            PyErr_SetString(PyExc_TimeoutError, "Timeout while sending command");
            return NULL;
        }

        return PyUnicode_FromStringAndSize(buf, len);
    }
}

// Note: wpa_ctrl_get_fd() does not seem necessary.

static PyMethodDef IFace_methods[] = {
    {"open", (PyCFunction) IFace_open, METH_VARARGS,
        "Open the control interface with wpa_ctrl_open() or wpa_ctrl_open2()."},
    {"attach", (PyCFunction) IFace_attach, METH_NOARGS,
        "Register as an event monitor with wpa_ctrl_attach()."},
    {"detach", (PyCFunction) IFace_detach, METH_NOARGS,
        "Unregister as an event monitor with wpa_ctrl_detach()."},
    {"close", (PyCFunction) IFace_close, METH_NOARGS,
        "Close the control interface with wpa_ctrl_close()."},
    {"pending", (PyCFunction) IFace_pending, METH_NOARGS, "Check whether there "
        "are pending event messages with wpa_ctrl_pending() and return True or "
        "False."},
    {"recv", (PyCFunction) IFace_recv, METH_NOARGS, "Receive a pending event "
        "message with wpa_ctrl_recv() and return it as a string."},
    {"request", (PyCFunction) IFace_request, METH_VARARGS, "Send a command "
        "with wpa_ctrl_request() and return the reply as a string.\n\n"
        "Optionally, msg_cb can be a callback to be called if event messages\n"
        "are received while waiting for the command response. This can only\n"
        "happen if the interface has been attach()ed.\n\n"
        "Alternatively, programs can simple use separate interfaces for\n"
        "commands and event messages. This is the recommended way due to\n"
        "performace reasons and because an exception in the callback function\n"
        "will cause further event messages and the response to be lost."},
    {NULL}
};

/*
 * Helpers for garbage collection (we have no member objects that can produce
 * cycles, so these are trivial)
 */
static int IFace_traverse(IFaceObject *self, visitproc visit, void *arg) {
    return 0;
}
static int IFace_clear(IFaceObject *self) {
    return 0;
}

/*
 * Destructor
 */
static void IFace_dealloc(IFaceObject *self) {
    // Close stuff if necessary
    if (self->ctrl != NULL) {
        if (self->attached == 1) {
            wpa_ctrl_detach(self->ctrl);
        }
        wpa_ctrl_close(self->ctrl);
    }

    PyObject_GC_UnTrack(self);
    IFace_clear(self);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyTypeObject IFaceType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "wpa_supplicant.IFace",
    .tp_doc = "A control interface for wpa_supplicant.",
    .tp_basicsize = sizeof(IFaceObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc) IFace_init,
    .tp_dealloc = (destructor) IFace_dealloc,
    .tp_traverse = (traverseproc) IFace_traverse,
    .tp_clear = (inquiry) IFace_clear,
    .tp_methods = IFace_methods,
};

/******************************************************************************
 * Create the module
 ******************************************************************************/

/*
 * Module definition
 */
static PyModuleDef wpa_supplicantmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "wpa_supplicant",
    .m_doc = "A module to interface with wpa_supplicant through its control "
             "interface (the default interface, not DBus).",
    .m_size = -1,
};

/*
 * Module __init__ function
 */

PyMODINIT_FUNC
PyInit_wpa_supplicant(void)
{
    // Initialize the IFace type
    if (PyType_Ready(&IFaceType) < 0) {
        return NULL;
    }

    // Create the module object
    PyObject *m;
    m = PyModule_Create(&wpa_supplicantmodule);
    if (m == NULL) {
        return NULL;
    }

    // Add the IFace type to the module
    Py_INCREF(&IFaceType);
    PyModule_AddObject(m, "IFace", (PyObject *) &IFaceType);
    return m;
}

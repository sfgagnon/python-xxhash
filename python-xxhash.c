/*
 * Copyright (c) 2014, Yue Du
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <string.h>
#include <Python.h>

#include "xxhash/xxhash.h"

#define TOSTRING(x) #x
#define VALUE_TO_STRING(x) TOSTRING(x)

#ifndef Py_TYPE
#define Py_TYPE(ob) (((PyObject*)(ob))->ob_type)
#endif

/*****************************************************************************
 * Module Functions ***********************************************************
 ****************************************************************************/

static char *keywords[] = {"string", "start", NULL};

static PyObject *xxh32(PyObject *self, PyObject *args, PyObject *kwargs)
{
    unsigned int seed = 0, digest = 0;
    Py_buffer view = { 0 };
    unsigned int ns;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s*|I:xxh32", keywords, &view, &seed)) {
        return NULL;
    }

    if (view.len > 1073741824) {
        PyErr_Format(PyExc_ValueError,
            "Input string length must be less or equal to 1GByte");
        return NULL;
    }

    ns = (unsigned int) view.len;
    digest = XXH32((char *) view.buf, ns, seed);
    return Py_BuildValue("I", digest);
}

static PyObject *xxh64(PyObject *self, PyObject *args, PyObject *kwargs)
{
    unsigned long long seed = 0, digest = 0;
    Py_buffer view = { 0 };
    unsigned int ns;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s*|K:xxh64", keywords, &view, &seed)) {
        return NULL;
    }

    if (view.len > 1073741824) {
        PyErr_Format(PyExc_ValueError,
            "Input string length must be less or equal to 1GByte");
        return NULL;
    }

    ns = (unsigned int) view.len;
    digest = XXH64((char *) view.buf, ns, seed);
    return Py_BuildValue("K", digest);
}

/*****************************************************************************
 * Module Types ***************************************************************
 ****************************************************************************/

/* XXH32 */

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    void *xxhash_state;
} PYXXH32Object;

static PyTypeObject PYXXH32Type;

static void PYXXH32_dealloc(PYXXH32Object *self)
{
    if (self->xxhash_state != NULL) {
        free(self->xxhash_state);
    }

    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *PYXXH32_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    PYXXH32Object *self;

    self = (PYXXH32Object *)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

static int PYXXH32_init(PYXXH32Object *self, PyObject *args, PyObject *kwargs)
{
    unsigned int seed = 0;
    Py_buffer view = { 0 };
    Py_ssize_t n, nbytes;
    char *buf;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|s*I:init", keywords, &view, &seed)) {
        return -1;
    }

    if ((self->xxhash_state = XXH32_init(seed)) == NULL) {
        PyBuffer_Release(&view);
        PyErr_Format(PyExc_MemoryError,
            "Can't allocate hash state data structure");
        return -1;
    }

    n = view.len;
    buf = (char *) view.buf;
    while (n > 0) {
        if (n > 1073741824)
            nbytes = 1073741824;
        else
            nbytes = n;
        XXH32_update(self->xxhash_state, buf,
                     Py_SAFE_DOWNCAST(nbytes, Py_ssize_t, unsigned int));
        buf += nbytes;
        n -= nbytes;
    }

    PyBuffer_Release(&view);
    return 0;
}

static PyObject *PYXXH32_update(PYXXH32Object *self, PyObject *args)
{
    Py_buffer view = { 0 };
    Py_ssize_t n, nbytes;
    char *buf;

    if (!PyArg_ParseTuple(args, "s*:update", &view)) {
        return NULL;
    }

    n = view.len;
    buf = (char *) view.buf;
    while (n > 0) {
         if (n > 1073741824)
            nbytes = 1073741824;
        else
            nbytes = n;
        XXH32_update(self->xxhash_state, buf,
                     Py_SAFE_DOWNCAST(nbytes, Py_ssize_t, unsigned int));
        buf += nbytes;
        n -= nbytes;
    }

    PyBuffer_Release(&view);
    Py_RETURN_NONE;
}

static PyObject *PYXXH32_digest(PYXXH32Object *self)
{
    unsigned int digest = XXH32_intermediateDigest(self->xxhash_state);

#if PY_MAJOR_VERSION < 3
    return PyString_FromStringAndSize((char*)&digest, 4);
#else
    return PyBytes_FromStringAndSize((char*)&digest, 4);
#endif
}

static PyObject *PYXXH32_hex_digest(PYXXH32Object *self)
{
    unsigned char hexdigest[8];
    unsigned int digest;
    int i, j;

    digest = XXH32_intermediateDigest(self->xxhash_state);

    /* Make hex version of the digest */
    for(i=0,j=0; i<4; i++) {
        char c;
        c = (digest >> 28) & 0xf;
        c = (c>9) ? c+'a'-10 : c + '0';
        hexdigest[j++] = c;
        c = (digest >> 24) & 0xf;
        c = (c>9) ? c+'a'-10 : c + '0';
        hexdigest[j++] = c;
        digest = digest << 8;
    }

#if PY_MAJOR_VERSION < 3
    return PyString_FromStringAndSize((char*)hexdigest, 8);
#else
    return PyUnicode_FromStringAndSize((char*)hexdigest, 8);
#endif
}

static PyObject *PYXXH32_copy(PYXXH32Object *self)
{
    PYXXH32Object *hashObject;
    hashObject = (PYXXH32Object *)PyObject_CallFunction((PyObject*)&PYXXH32Type, NULL);
    if (hashObject == NULL)
        return NULL;

    memcpy(hashObject->xxhash_state, self->xxhash_state, XXH32_sizeofState());
    return (PyObject *)hashObject;
}

static PyMethodDef PYXXH32_methods[] = {
    {"update", (PyCFunction)PYXXH32_update, METH_VARARGS, "Update this hash object's state with the provided string."},
    {"digest", (PyCFunction)PYXXH32_digest, METH_NOARGS, "Return the digest value as a string of binary data."},
    {"hexdigest", (PyCFunction)PYXXH32_hex_digest, METH_NOARGS, "Return the digest value as a string of hexadecimal digits."},
    {"copy", (PyCFunction)PYXXH32_copy, METH_NOARGS, "Return a copy of the hash object."},
    {NULL, NULL, 0, NULL}
};

static PyObject *
PYXXH32_get_block_size(PyObject *self, void *closure)
{
    return Py_BuildValue("I", 16);
}

static PyObject *
PYXXH32_get_digest_size(PyObject *self, void *closure)
{
    return Py_BuildValue("I", 4);
}

static PyObject *
PYXXH32_get_name(PyObject *self, void *closure)
{
#if PY_MAJOR_VERSION < 3
    return PyString_FromStringAndSize("XXH32", 5);
#else
    return PyUnicode_FromStringAndSize("XXH32", 5);
#endif
}

static PyGetSetDef PYXXH32_getseters[] = {
    {"digest_size",
     (getter)PYXXH32_get_digest_size, NULL,
     NULL,
     NULL},
    {"block_size",
     (getter)PYXXH32_get_block_size, NULL,
     NULL,
     NULL},
    {"name",
     (getter)PYXXH32_get_name, NULL,
     NULL,
     NULL},
    {NULL}  /* Sentinel */
};

PyDoc_STRVAR(XXH32_doc,
"An XXH32 represents the object used to calculate the xxh64 hash of a\n\
string of information.\n\
\n\
Methods:\n\
\n\
update() -- updates the current digest with an additional string\n\
digest() -- return the current digest value\n\
hexdigest() -- return the current digest as a string of hexadecimal digits\n\
copy() -- return a copy of the current md5 object");

static PyTypeObject PYXXH32Type = {
#if PY_MAJOR_VERSION >= 3
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
#else
    PyObject_HEAD_INIT(NULL)
    0,                             /* ob_size */
#endif
    "xxhash.XXH32",                /* tp_name */
    sizeof(PYXXH32Object),         /* tp_basicsize */
    0,                             /* tp_itemsize */
    (destructor)PYXXH32_dealloc,   /* tp_dealloc */
    0,                             /* tp_print */
    0,                             /* tp_getattr */
    0,                             /* tp_setattr */
    0,                             /* tp_compare */
    0,                             /* tp_repr */
    0,                             /* tp_as_number */
    0,                             /* tp_as_sequence */
    0,                             /* tp_as_mapping */
    0,                             /* tp_hash */
    0,                             /* tp_call */
    0,                             /* tp_str */
    0,                             /* tp_getattro */
    0,                             /* tp_setattro */
    0,                             /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,            /* tp_flags */
    XXH32_doc,                     /* tp_doc */
    0,                             /* tp_traverse */
    0,                             /* tp_clear */
    0,                             /* tp_richcompare */
    0,                             /* tp_weaklistoffset */
    0,                             /* tp_iter */
    0,                             /* tp_iternext */
    PYXXH32_methods,               /* tp_methods */
    0,                             /* tp_members */
    PYXXH32_getseters,             /* tp_getset */
    0,                             /* tp_base */
    0,                             /* tp_dict */
    0,                             /* tp_descr_get */
    0,                             /* tp_descr_set */
    0,                             /* tp_dictoffset */
    (initproc)PYXXH32_init,        /* tp_init */
    0,                             /* tp_alloc */
    PYXXH32_new,                   /* tp_new */
};


/* XXH64 */

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    void *xxhash_state;
} PYXXH64Object;

static PyTypeObject PYXXH64Type;

static void PYXXH64_dealloc(PYXXH64Object *self)
{
    if (self->xxhash_state != NULL) {
        free(self->xxhash_state);
    }

    ((PyObject *)self)->ob_type->tp_free(self);
}

static PyObject *PYXXH64_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    PYXXH64Object *self;

    self = (PYXXH64Object *)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

static int PYXXH64_init(PYXXH64Object *self, PyObject *args, PyObject *kwargs)
{
    unsigned long long seed = 0;
    Py_buffer view = { 0 };
    Py_ssize_t n, nbytes;
    char *buf;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|s*K:init", keywords, &view, &seed)) {
        return -1;
    }

    if ((self->xxhash_state = XXH64_init(seed)) == NULL) {
        PyBuffer_Release(&view);
        PyErr_Format(PyExc_MemoryError,
            "Can't allocate hash state data structure");
        return -1;
    }

    n = view.len;
    buf = (char *) view.buf;
    while (n > 0) {
        if (n > 1073741824)
            nbytes = 1073741824;
        else
            nbytes = n;
        XXH64_update(self->xxhash_state, buf,
                     Py_SAFE_DOWNCAST(nbytes, Py_ssize_t, unsigned int));
        buf += nbytes;
        n -= nbytes;
    }

    PyBuffer_Release(&view);
    return 0;
}

static PyObject *PYXXH64_update(PYXXH64Object *self, PyObject *args)
{
    Py_buffer view = { 0 };
    Py_ssize_t n, nbytes;
    char *buf;

    if (!PyArg_ParseTuple(args, "s*:update", &view)) {
        return NULL;
    }

    n = view.len;
    buf = (char *) view.buf;
    while (n > 0) {
         if (n > 1073741824)
            nbytes = 1073741824;
        else
            nbytes = n;
        XXH64_update(self->xxhash_state, buf,
                     Py_SAFE_DOWNCAST(nbytes, Py_ssize_t, unsigned int));
        buf += nbytes;
        n -= nbytes;
    }

    PyBuffer_Release(&view);
    Py_RETURN_NONE;
}

static PyObject *PYXXH64_digest(PYXXH64Object *self)
{
    unsigned long long digest = XXH64_intermediateDigest(self->xxhash_state);

#if PY_MAJOR_VERSION < 3
    return PyString_FromStringAndSize((char*)&digest, 8);
#else
    return PyBytes_FromStringAndSize((char*)&digest, 8);
#endif
}

static PyObject *PYXXH64_hex_digest(PYXXH32Object *self)
{
    unsigned char hexdigest[16];
    unsigned long long digest;
    int i, j;

    digest = XXH64_intermediateDigest(self->xxhash_state);

    /* Make hex version of the digest */
    for(i=0,j=0; i<8; i++) {
        char c;
        c = (digest >> 60) & 0xf;
        c = (c>9) ? c+'a'-10 : c + '0';
        hexdigest[j++] = c;
        c = (digest >> 56) & 0xf;
        c = (c>9) ? c+'a'-10 : c + '0';
        hexdigest[j++] = c;
        digest = digest << 8;
    }

#if PY_MAJOR_VERSION < 3
    return PyString_FromStringAndSize((char*)hexdigest, 16);
#else
    return PyUnicode_FromStringAndSize((char*)hexdigest, 16);
#endif
}

static PyObject *PYXXH64_copy(PYXXH64Object *self)
{
    PYXXH64Object *hashObject;
    hashObject = (PYXXH64Object *)PyObject_CallFunction((PyObject*)&PYXXH64Type, NULL);
    if (hashObject == NULL)
        return NULL;

    memcpy(hashObject->xxhash_state, self->xxhash_state, XXH64_sizeofState());
    return (PyObject *)hashObject;
}

static PyMethodDef PYXXH64_methods[] = {
    {"update", (PyCFunction)PYXXH64_update, METH_VARARGS, "Update this hash object's state with the provided string."},
    {"digest", (PyCFunction)PYXXH64_digest, METH_NOARGS, "Return the digest value as a string of binary data."},
    {"hexdigest", (PyCFunction)PYXXH64_hex_digest, METH_NOARGS, "Return the digest value as a string of hexadecimal digits."},
    {"copy", (PyCFunction)PYXXH64_copy, METH_NOARGS, "Return a copy of the hash object."},
    {NULL, NULL, 0, NULL}
};

static PyObject *
PYXXH64_get_block_size(PyObject *self, void *closure)
{
    return Py_BuildValue("I", 32);
}

static PyObject *
PYXXH64_get_digest_size(PyObject *self, void *closure)
{
    return Py_BuildValue("I", 8);
}

static PyObject *
PYXXH64_get_name(PyObject *self, void *closure)
{
#if PY_MAJOR_VERSION < 3
    return PyString_FromStringAndSize("XXH64", 5);
#else
    return PyUnicode_FromStringAndSize("XXH64", 5);
#endif
}

static PyGetSetDef PYXXH64_getseters[] = {
    {"digest_size",
     (getter)PYXXH64_get_digest_size, NULL,
     NULL,
     NULL},
    {"block_size",
     (getter)PYXXH64_get_block_size, NULL,
     NULL,
     NULL},
    {"name",
     (getter)PYXXH64_get_name, NULL,
     NULL,
     NULL},
    {NULL}  /* Sentinel */
};

PyDoc_STRVAR(XXH64_doc,
"An XXH64 represents the object used to calculate the xxh64 hash of a\n\
string of information.\n\
\n\
Methods:\n\
\n\
update() -- updates the current digest with an additional string\n\
digest() -- return the current digest value\n\
hexdigest() -- return the current digest as a string of hexadecimal digits\n\
copy() -- return a copy of the current md5 object");

static PyTypeObject PYXXH64Type = {
#if PY_MAJOR_VERSION >= 3
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
#else
    PyObject_HEAD_INIT(NULL)
    0,                             /* ob_size */
#endif
    "xxhash.XXH64",                /* tp_name */
    sizeof(PYXXH64Object),         /* tp_basicsize */
    0,                             /* tp_itemsize */
    (destructor)PYXXH64_dealloc,   /* tp_dealloc */
    0,                             /* tp_print */
    0,                             /* tp_getattr */
    0,                             /* tp_setattr */
    0,                             /* tp_compare */
    0,                             /* tp_repr */
    0,                             /* tp_as_number */
    0,                             /* tp_as_sequence */
    0,                             /* tp_as_mapping */
    0,                             /* tp_hash */
    0,                             /* tp_call */
    0,                             /* tp_str */
    0,                             /* tp_getattro */
    0,                             /* tp_setattro */
    0,                             /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,            /* tp_flags */
    XXH64_doc,                     /* tp_doc */
    0,                             /* tp_traverse */
    0,                             /* tp_clear */
    0,                             /* tp_richcompare */
    0,                             /* tp_weaklistoffset */
    0,                             /* tp_iter */
    0,                             /* tp_iternext */
    PYXXH64_methods,               /* tp_methods */
    0,                             /* tp_members */
    PYXXH64_getseters,             /* tp_getset */
    0,                             /* tp_base */
    0,                             /* tp_dict */
    0,                             /* tp_descr_get */
    0,                             /* tp_descr_set */
    0,                             /* tp_dictoffset */
    (initproc)PYXXH64_init,        /* tp_init */
    0,                             /* tp_alloc */
    PYXXH64_new,                   /* tp_new */
};


/*****************************************************************************
 * Module Init ****************************************************************
 ****************************************************************************/

/* ref: https://docs.python.org/2/howto/cporting.html */

struct module_state {
    PyObject *error;
};

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

PyDoc_STRVAR(module_doc,
"This module is a Python binding for the xxHash library\n\
(http://code.google.com/p/xxhash/) by Yann Collet.\n\n\
Two functions (xxh32 and xxh64) return 32bit or 64bit hash\n\
integer of an input string. For example:\n\
\n\
    >>> import xxhash\n\
    >>> xxhash.xxh64(\"Nobody inspects the spammish repetition\")\n\
    18144624926692707313L\n\
\n\
The module also includes the XXH32 and XXH64 objects that have hashlib\n\
compatible interfaces:\n\
 - update(arg): Update the hash object with the string arg. Repeated calls\n\
                are equivalent to a single call with the concatenation of all\n\
                the arguments.\n\
 - digest():    Return the digest of the strings passed to the update() method\n\
                so far. This may contain non-ASCII characters, including\n\
                NUL bytes.\n\
 - hexdigest(): Like digest() except the digest is returned as a string of\n\
                double length, containing only hexadecimal digits.\n\
 - copy():      Return a copy (clone) of the hash object. This can be used to\n\
                efficiently compute the digests of strings that share a common\n\
                initial substring.\n\
\n\
For example, to obtain the digest of the string 'Nobody inspects the\n\
spammish repetition':\n\
\n\
    >>> import xxhash\n\
    >>> m = xxhash.XXH64()\n\
    >>> m.update(\"Nobody inspects\")\n\
    >>> m.update(\" the spammish repetition\")\n\
    >>> m.digest()\n\
    ''\\xf1\\x8b7\\x8a<\\xa8\\xce\\xfb''\n\
\n\
More condensed:\n\
\n\
    >>> xxhash.XXH64(\"Nobody inspects the spammish repetition\").hexdigest()\n\
    'fbcea83c8a378bf1'\n");

PyDoc_STRVAR(xxh32_doc,
"xxh32(string, start)\n\
Calculate the 32-bits hash of a string or memory buffer\n\
start can be used to alter the result predictably.\n\
Return: Long integer hash value.\n");

PyDoc_STRVAR(xxh64_doc,
"xxh64(string, start)\n\
Calculate the 64-bits hash of a string or memory buffer\n\
start can be used to alter the result predictably.\n\
Return: Long integer hash value.\n");

static PyMethodDef methods[] = {
    {"xxh32", (PyCFunction)xxh32, METH_VARARGS | METH_KEYWORDS, xxh32_doc},
    {"xxh64", (PyCFunction)xxh64, METH_VARARGS | METH_KEYWORDS, xxh64_doc},
    {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION >= 3

static int myextension_traverse(PyObject *m, visitproc visit, void *arg)
{
    Py_VISIT(GETSTATE(m)->error);
    return 0;
}

static int myextension_clear(PyObject *m)
{
    Py_CLEAR(GETSTATE(m)->error);
    return 0;
}


static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "xxhash",
    module_doc,
    sizeof(struct module_state),
    methods,
    NULL,
    myextension_traverse,
    myextension_clear,
    NULL
};

#define INITERROR return NULL

PyObject *PyInit_xxhash(void)

#else
#define INITERROR return

void initxxhash(void)
#endif
{
    PyObject *module;
    struct module_state *st;

#if PY_MAJOR_VERSION >= 3
    module = PyModule_Create(&moduledef);
#else
    module = Py_InitModule3("xxhash", methods, module_doc);
#endif

    if (module == NULL) {
        INITERROR;
    }

    st = GETSTATE(module);

    st->error = PyErr_NewException("xxhash.Error", NULL, NULL);

    if (st->error == NULL) {
        Py_DECREF(module);
        INITERROR;
    }

    PYXXH32Type.tp_new = PyType_GenericNew;

    if (PyType_Ready(&PYXXH32Type) < 0) {
        INITERROR;
    }

    Py_INCREF(&PYXXH32Type);
    PyModule_AddObject(module, "XXH32", (PyObject *)&PYXXH32Type);


    PYXXH64Type.tp_new = PyType_GenericNew;

    if (PyType_Ready(&PYXXH64Type) < 0) {
        INITERROR;
    }

    Py_INCREF(&PYXXH64Type);
    PyModule_AddObject(module, "XXH64", (PyObject *)&PYXXH64Type);

    PyModule_AddStringConstant(module, "VERSION", VALUE_TO_STRING(VERSION));
    PyModule_AddStringConstant(module, "XXHASH_VERSION", VALUE_TO_STRING(XXHASH_VERSION));

#if PY_MAJOR_VERSION >= 3
    return module;
#endif
}

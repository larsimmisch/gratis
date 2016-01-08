/* -*- Mode: C; c-basic-offset: 4; indent-tabs-mode: nil -*- */ 

/*
 * epdirect - a python interface to the eInk paper driver from repaper.org
 *
 * Author: Lars Immisch
 *
 * License: Python Software Foundation License
 *
 */

#include "Python.h"
#if PY_MAJOR_VERSION < 3 && PY_MINOR_VERSION < 6
#include "stringobject.h"
#define PyUnicode_FromString PyString_FromString
#endif
#include <stdio.h>
#include "spi.h"
#include "epd.h"

PyDoc_STRVAR(epdirect_module_doc,
             "This modules gives direct access to the ePaper drivers from Pervasive displays");

typedef struct {
    PyObject_HEAD;
    SPI_type spi;
    EPD_type epd;
} epdirect_t;

/******************************************/
/* EPDirect object wrapper                */
/******************************************/

static PyTypeObject EPDirectType;
static PyObject *EPDirectError;

static PyObject *
epdirect_new(PyTypeObject *type, PyObject *args, PyObject *kwds) 
{
    int res;
    epdirect_t *self;
    char *kw[] = { "size", "spipath", "spibps", NULL };
    int size = (int)EPD_2_7;
    char *spipath = NULL;
    int spibps = 0;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|izi", kw,
                                     &pcmtype, &pcmmode, &cardname)) 
        return NULL;
    
    if (!(self = (epdirect_t *)PyObject_New(epdirect_t, &EPDirectType))) 
        return NULL;

    if (size < 0 || size > EPD_2_7)
    {
        PyErr_SetString(EPDirectError, "Invalid size");
        return NULL;
    }

    self->spi = SPI_create(spipath, spibps);
    if (!spi) {
        PyErr_SetString(EPDirectError, "Cannot allocate SPI");
        return NULL;
    }
    
    self->epd = EPD_create((EPD_SIZE)size,
                           self->spi);
    
    if (!self->epd)
    {
        PyErr_SetString(EPDirectError, "Cannot allocate EPD");
        return NULL;
    }
    return (PyObject *)self;
}

static void
epdirect_dealloc(alsapcm_t *self) 
{
    if (self->epd) {
        EPD_destroy(self->epd);
        self->epd = NULL;
    }
    if (self->spi) {
        SPI_destroy(self->spi);
        self->spi = NULL;
    }
    PyObject_Del(self);
}

static PyObject *
epdirect_close(alsapcm_t *self, PyObject *args) 
{
    if (!PyArg_ParseTuple(args,":close")) 
        return NULL;

    if (self->epd) {
        EPD_destroy(self->epd);
        self->epd = NULL;
    }
    if (self->spi) {
        SPI_destroy(self->spi);
        self->spi = NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

PyDoc_STRVAR(close_doc,
"close() -> None\n\
\n\
Close a EPDirect object.");

static PyObject *
epdirect_set_temperature(epdirect_t *self, PyObject *args) 
{
    int temperature;
    if (!PyArg_ParseTuple(args,"i:settemperature", &temperature)) {
        return NULL;
    }

    if (!self->epd) {
        PyErr_SetString(EPDirectError, "EPD device is closed");
        return NULL;
    }

    EPD_set_temperature(self, temperature);

    Py_INCREF(Py_None);
    return Py_None;
}

PyDoc_STRVAR(set_temperature_doc,
"set_temperature(temperature) -> None\n\
\n\
Set the temperature compensation (call before begin).\n\
The unit is degree Celsius");

static PyObject *
epdirect_clear(epdirect_t *self, PyObject *args) 
{
    int res;

    if (!PyArg_ParseTuple(args,":clear")) {
        return NULL;
    }

    if (!self->epd)
    {
        PyErr_SetString(EPDirectError, "EPD device is closed");
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    EPD_set_clear(self);
    Py_END_ALLOW_THREADS
    
    Py_INCREF(Py_None);
    return Py_None;
}

PyDoc_STRVAR(clear_doc,
"clear() -> None\n\
\n\
Clear the display.");

static PyObject *
epdirect_image_0(epdirect_t *self, PyObject *args) 
{
    int res;
    int imagelen;
    char *image;
    
#if PY_MAJOR_VERSION < 3
    if (!PyArg_ParseTuple(args,"s#:image_0", &image, &imagelen)) 
        return NULL;
#else
    Py_buffer buf;

    if (!PyArg_ParseTuple(args,"y*:image_0",&buf)) 
        return NULL;

    image = buf.buf;
    imagelen = buf.len;
#endif

    if (!self->epd)
    {
        PyErr_SetString(EPDirectError, "EPD device is closed");
        return NULL;
    }
    
    if (imagelen != self->epd->bytes_per_line * self->epd->lines_per_display) {
        PyErr_SetString(EPDirectError, "Invalid image size");
        return NULL;        
    }
    
    Py_BEGIN_ALLOW_THREADS
    EPD_image_0(self, image);
    Py_END_ALLOW_THREADS
    
    Py_INCREF(Py_None);
    return Py_None;  
}

PyDoc_STRVAR(image_0_doc,
"image_0(image) -> None\n\
\n\
Output an image, assuming a clear (white) screen");

static PyObject *
epdirect_image(epdirect_t *self, PyObject *args) 
{
    int res;
    int imagelen;
    char *image;
    int old_imagelen;
    char *old_image;
    
#if PY_MAJOR_VERSION < 3
    if (!PyArg_ParseTuple(args,"s#s#:image", &old_image, &old_imagelen,
                          &image, &imagelen)) 
        return NULL;
#else
    Py_buffer buf;
    Py_buffer old_buf;

    if (!PyArg_ParseTuple(args,"y*y*:image", &old_buf, &buf)) 
        return NULL;
    
    old_image = old_buf.buf;
    old_imagelen = old_buf.len;
    image = buf.buf;
    imagelen = buf.len;
#endif

    if (!self->epd)
    {
        PyErr_SetString(EPDirectError, "EPD device is closed");
        return NULL;
    }
    
    if (imagelen != self->bytes_per_line * self->lines_per_display) {
        PyErr_SetString(EPDirectError, "Invalid image size");
        return NULL;        
    }

    if (old_imagelen != self->bytes_per_line * self->lines_per_display) {
        PyErr_SetString(EPDirectError, "Invalid image size (old)");
        return NULL;        
    }
    
    Py_BEGIN_ALLOW_THREADS
        EPD_image(self, old_image, image);
    Py_END_ALLOW_THREADS
    
    Py_INCREF(Py_None);
    return Py_None;
}

PyDoc_STRVAR(image_doc,
"image(old_image, image) -> None\n\
\n\
Output an image");

static PyObject *
epdirect_image_partial(epdirect_t *self, PyObject *args) 
{
    int res;
    int imagelen;
    char *image;
    int old_imagelen;
    char *old_image;
    
#if PY_MAJOR_VERSION < 3
    if (!PyArg_ParseTuple(args,"s#s#:image_partial", &old_image, &old_imagelen,
                          &image, &imagelen)) 
        return NULL;
#else
    Py_buffer buf;
    Py_buffer old_buf;

    if (!PyArg_ParseTuple(args,"y*y*:image", &old_buf, &buf)) 
        return NULL;
    
    old_image = old_buf.buf;
    old_imagelen = old_buf.len;
    image = buf.buf;
    imagelen = buf.len;
#endif

    if (!self->epd)
    {
        PyErr_SetString(EPDirectError, "EPD device is closed");
        return NULL;
    }
    
    if (imagelen != self->bytes_per_line * self->lines_per_display) {
        PyErr_SetString(EPDirectError, "Invalid image size");
        return NULL;        
    }

    if (old_imagelen != self->bytes_per_line * self->lines_per_display) {
        PyErr_SetString(EPDirectError, "Invalid image size (old)");
        return NULL;        
    }
    
    Py_BEGIN_ALLOW_THREADS
        EPD_image(self, old_image, image);
    Py_END_ALLOW_THREADS
    
    Py_INCREF(Py_None);
    return Py_None;
}

PyDoc_STRVAR(image_doc,
"image(old_image, image) -> None\n\
\n\
Output an image");

/* EPDirect Object Bureaucracy */

static PyMethodDef epdirect_methods[] = {
    { "set_temperature", (PyCFunction)epdirect_set_temperature,
      METH_VARARGS, set_temperature_doc},
    { "clear", (PyCFunction)epdirect_clear,
      METH_VARARGS, clear_doc},
    { "image_0", (PyCFunction)epdirect_image_0, METH_VARARGS, image_0_doc},
    { "image", (PyCFunction)epdirect_image, METH_VARARGS, image_doc},
    { "image_partial", (PyCFunction)epdirect_image_partial, METH_VARARGS,
        image_partial_doc},
    {NULL, NULL}
};

#if PY_VERSION_HEX < 0x02020000 
static PyObject *	 
epdirect_getattr(alsapcm_t *self, char *name) {	 
    return Py_FindMethod(epdirect_methods, (PyObject *)self, name);	 
}
#endif

static PyTypeObject EPDirectType = {
#if PY_MAJOR_VERSION < 3
    PyObject_HEAD_INIT(&PyType_Type)
    0,                              /* ob_size */
#else
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
#endif
    "epdirect.EPD",                 /* tp_name */
    sizeof(epdirect_t),             /* tp_basicsize */
    0,                              /* tp_itemsize */
    /* methods */    
    (destructor) epdirect_dealloc,  /* tp_dealloc */
    0,                              /* print */
#if PY_VERSION_HEX < 0x02020000
    (getattrfunc)epdirect_getattr,  /* tp_getattr */
#else
    0,                              /* tp_getattr */
#endif
    0,                              /* tp_setattr */
    0,                              /* tp_compare */ 
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
    0,                              /* tp_str */
#if PY_VERSION_HEX >= 0x02020000 
    PyObject_GenericGetAttr,        /* tp_getattro */
#else
    0,                              /* tp_getattro */
#endif
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,             /* tp_flags */
    "EPD device.",                  /* tp_doc */
    0,					            /* tp_traverse */
    0,					            /* tp_clear */
    0,					            /* tp_richcompare */
    0,					            /* tp_weaklistoffset */
    0,					            /* tp_iter */
    0,					            /* tp_iternext */
    epdirect_methods,		        /* tp_methods */
    0,			                    /* tp_members */
};

#if PY_MAJOR_VERSION >= 3

#define _EXPORT_INT(mod, name, value) \
  if (PyModule_AddIntConstant(mod, name, (long) value) == -1) return NULL;

static struct PyModuleDef epdirect_module = {
    PyModuleDef_HEAD_INIT,
    "epdirect",
    epdirect_module_doc,
    -1,
    epdirect_methods,
    0,  /* m_reload */
    0,  /* m_traverse */
    0,  /* m_clear */
    0,  /* m_free */
};

#else

#define _EXPORT_INT(mod, name, value) \
  if (PyModule_AddIntConstant(mod, name, (long) value) == -1) return;

#endif // 3.0

#if PY_MAJOR_VERSION < 3
void initepdirect(void) 
#else
PyObject *PyInit_epdirect(void)
#endif
{
    PyObject *m;
    EPDirectType.tp_new = epdirect_new;

    PyEval_InitThreads();

#if PY_MAJOR_VERSION < 3
    m = Py_InitModule3("epdirect", epdirect_methods, epdirect_module_doc);
    if (!m) 
        return;
#else

    m = PyModule_Create(&epdirect_module);
    if (!m) 
        return NULL;

#endif

    EPDirectError = PyErr_NewException("epdirect.EPDirectError", NULL, 
                                        NULL);
    if (!EPDirectError)
#if PY_MAJOR_VERSION < 3
        return;
#else
        return NULL;
#endif

    /* Each call to PyModule_AddObject decrefs it; compensate: */

    Py_INCREF(&EPDirectType);
    PyModule_AddObject(m, "EPD", (PyObject *)&EPDirectType);
  
    Py_INCREF(ALSAAudioError);
    PyModule_AddObject(m, "EPDirectError", EPDirectError);

    _EXPORT_INT(m, "EPD_1_44", EPD_1_44);
    _EXPORT_INT(m, "EPD_1_9", EPD_1_9);
    _EXPORT_INT(m, "EPD_2_0", EPD_2_0);
    _EXPORT_INT(m, "EPD_2_6", EPD_2_6);
    _EXPORT_INT(m, "EPD_2_7", EPD_2_7);

    _EXPORT_INT(m, "EPD_CHIP_VERSION", EPD_CHIP_VERSION);
    _EXPORT_INT(m, "EPD_FILM_VERSION", EPD_FILM_VERSION);
    _EXPORT_INT(m, "EPD_PWM_REQUIRED", EPD_PWM_REQUIRED);
    _EXPORT_INT(m, "EPD_IMAGE_ONE_ARG", EPD_IMAGE_ONE_ARG);
    _EXPORT_INT(m, "EPD_IMAGE_TWO_ARG", EPD_IMAGE_TWO_ARG);
    _EXPORT_INT(m, "EPD_PARTIAL_AVAILABLE", EPD_PARTIAL_AVAILABLE);
    
#if PY_MAJOR_VERSION >= 3
    return m;
#endif
}

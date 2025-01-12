#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <ctype.h>
#include <stdbool.h>

static void
_skip_sp(Py_ssize_t *i, PyObject *src, Py_ssize_t srclen)
{
    Py_ssize_t j = *i;

    while (j < srclen) {
        Py_UCS4 c = PyUnicode_READ_CHAR(src, j);
        if (!Py_UNICODE_ISSPACE(c)) {
            break;
        }
        j++;
    }

    *i = j;
}

static bool
_parse_ident(
    Py_ssize_t *i,
    PyObject *src,
    Py_ssize_t len,
    Py_UCS4 buf[],
    size_t buf_size,
    size_t *buf_len
) {
    Py_ssize_t j = *i;
    *buf_len = 0;

    for (; j < len; j++) {
        int c = PyUnicode_READ_CHAR(src, j);
        if (c == '\n') {
            break;
        }
        if (c == '_' || c == '-' || isalnum(c)) {
            if (*buf_len >= buf_size-1) {
                return false;
            }
            buf[*buf_len] = c;
            (*buf_len)++;
        } else {
            break;
        }
    }

    buf[*buf_len] = 0;
    *i = j;
    return true;
}

static bool
_parse_value(
    Py_ssize_t *i,
    PyObject *src,
    Py_ssize_t len,
    Py_UCS4 buf[],
    size_t buf_size,
    size_t *buf_len
) {
    bool ret = true;
    Py_ssize_t j = *i;
    int m = 0;
    *buf_len = 0;

    for (; j < len; j++) {
        int c = PyUnicode_READ_CHAR(src, j);

        switch (m) {
        case 0:
            if (c == '"') {
                m = 100;
            } else if (c == '\n') {
                goto done;
            } else if (Py_UNICODE_ISSPACE(c)) {
                // pass
            } else {
                if (*buf_len >= buf_size-1) {
                    ret = false;
                    goto done;
                }
                buf[*buf_len] = c;
                (*buf_len)++;
                m = 50;
            }
            break;
        case 50:
            if (c == '\n') {
                goto done;
            } else if (Py_UNICODE_ISSPACE(c)) {
                goto done;
            } else {
                if (*buf_len >= buf_size-1) {
                    ret = false;
                    goto done;
                }
                buf[*buf_len] = c;
                (*buf_len)++;
            }
            break;
        case 100:
            if (c == '\\') {
                j++;
                if (j >= len) {
                    ret = false;
                    goto done;
                }
                c = PyUnicode_READ_CHAR(src, j);

                if (*buf_len >= buf_size-1) {
                    ret = false;
                    goto done;
                }
                buf[*buf_len] = c;
                (*buf_len)++;
            } else if (c == '"') {
                j++;
                goto done;
            } else {
                if (*buf_len >= buf_size-1) {
                    ret = false;
                    goto done;
                }
                buf[*buf_len] = c;
                (*buf_len)++;                
            }
            break;
        }
    }

done:
    *i = j;
    buf[*buf_len] = 0;
    return ret;
}

static PyObject *parse_key_value(PyObject* self, PyObject* args) {
    Py_ssize_t i = 0;
    PyObject *src = NULL;
    Py_ssize_t len = 0;

    if (!PyArg_ParseTuple(args, "nOn", &i, &src, &len)) {
        return NULL;
    }

    #undef _BUF_SIZE
    #define _BUF_SIZE 1024
    Py_UCS4 key[_BUF_SIZE] = {0};
    Py_UCS4 val[_BUF_SIZE] = {0};
    size_t key_len = 0;
    size_t val_len = 0;

    for (; i < len; i++) {
        int c = PyUnicode_READ_CHAR(src, i);
        if (c == '\n') {
            break;
        }

        if (c == '_' || c == '-' || isalpha(c)) {
            if (!_parse_ident(&i, src, len, key, sizeof key, &key_len)) {
                break;
            }
            _skip_sp(&i, src, len);
            if (i >= len) {
                break;
            }

            c = PyUnicode_READ_CHAR(src, i);
            if (c == '=') {
                i++;
                _skip_sp(&i, src, len);
                if (!_parse_value(&i, src, len, val, sizeof val, &val_len)) {
                    break;
                }
                break;
            }
        }
    }

    PyObject* result = PyTuple_New(3);
    if (result == NULL) {
        return NULL;
    }

    PyTuple_SET_ITEM(result, 0, PyLong_FromSsize_t(i));
    PyTuple_SET_ITEM(result, 1, PyUnicode_FromKindAndData(PyUnicode_4BYTE_KIND, key, key_len));
    PyTuple_SET_ITEM(result, 2, PyUnicode_FromKindAndData(PyUnicode_4BYTE_KIND, val, val_len));

    return result;
}

static PyMethodDef MyMethods[] = {
    {"parse_key_value", parse_key_value, METH_VARARGS, "Parse keyword arguments."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef mymodule = {
    PyModuleDef_HEAD_INIT,
    "parseutils",
    "Utility for parse strings.",
    -1,
    MyMethods
};

PyMODINIT_FUNC PyInit_parseutils(void) {
    return PyModule_Create(&mymodule);
}

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <ctype.h>
#include <stdbool.h>

static inline bool
_is_ident(int c) {
    return (c == '_' || c == '-' || isalnum(c));
}

static inline bool
_is_ident_head(int c) {
    return (c == '_' || c == '-' || isalpha(c));
}

static inline bool
_is_css_ident(int c) {
    return (c == '.' || c == '#' || c == '_' || c == '-' || isalnum(c));
}

static inline bool
_is_css_ident_head(int c) {
    return (c == '.' || c == '#' || c == '_' || c == '-' || isalpha(c));
}

static void
_skip_sp(Py_ssize_t *index, PyObject *src, Py_ssize_t srclen)
{
    Py_ssize_t i = *index;

    while (i < srclen) {
        Py_UCS4 c = PyUnicode_READ_CHAR(src, i);
        if (!Py_UNICODE_ISSPACE(c)) {
            break;
        }
        i++;
    }

    *index = i;
}

static bool
_parse_ident(
    Py_ssize_t *index,
    PyObject *src,
    Py_ssize_t len,
    Py_UCS4 buf[],
    size_t buf_size,
    size_t *buf_len
) {
    Py_ssize_t i = *index;
    *buf_len = 0;

    for (; i < len; i++) {
        int c = PyUnicode_READ_CHAR(src, i);
        if (c == '\n') {
            break;
        }
        if (_is_ident(c)) {
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
    *index = i;
    return true;
}

static bool
_parse_css_ident(
    Py_ssize_t *index,
    PyObject *src,
    Py_ssize_t len,
    Py_UCS4 buf[],
    size_t buf_size,
    size_t *buf_len
) {
    Py_ssize_t i = *index;
    *buf_len = 0;

    for (; i < len; i++) {
        int c = PyUnicode_READ_CHAR(src, i);
        if (c == '\n') {
            break;
        }
        if (_is_css_ident(c)) {
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
    *index = i;
    return true;
}

static bool
_parse_value(
    Py_ssize_t *index,
    PyObject *src,
    Py_ssize_t len,
    Py_UCS4 buf[],
    size_t buf_size,
    size_t *buf_len
) {
    bool ret = true;
    Py_ssize_t i = *index;
    int m = 0;
    *buf_len = 0;

    for (; i < len; i++) {
        int c = PyUnicode_READ_CHAR(src, i);

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
                i++;
                if (i >= len) {
                    ret = false;
                    goto done;
                }
                c = PyUnicode_READ_CHAR(src, i);

                if (*buf_len >= buf_size-1) {
                    ret = false;
                    goto done;
                }
                buf[*buf_len] = c;
                (*buf_len)++;
            } else if (c == '"') {
                i++;
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
    *index = i;
    buf[*buf_len] = 0;
    return ret;
}

static bool
_parse_css_value(
    Py_ssize_t *index,
    PyObject *src,
    Py_ssize_t len,
    Py_UCS4 buf[],
    size_t buf_size,
    size_t *buf_len
) {
    bool ret = true;
    Py_ssize_t i = *index;
    int m = 0;
    *buf_len = 0;

    for (; i < len; i++) {
        int c = PyUnicode_READ_CHAR(src, i);

        switch (m) {
        case 0:
            if (c == ';' || c == '}') {
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
    *index = i;
    buf[*buf_len] = 0;
    return ret;
}

static bool
_parse_key_value(
    Py_ssize_t *index,
    PyObject *src,
    Py_ssize_t len,
    Py_UCS4 key[],
    size_t key_size,
    size_t *key_len,
    Py_UCS4 val[],
    size_t val_size,
    size_t *val_len,
    int sep,  // '=' or ':'
    int end  // 0 or ';'
) {
    Py_ssize_t i = *index;

    for (; i < len; i++) {
        int c = PyUnicode_READ_CHAR(src, i);
        if (c == end) {
            break;
        }

        if (_is_ident_head(c)) {
            if (!_parse_ident(&i, src, len, key, key_size, key_len)) {
                break;
            }
            _skip_sp(&i, src, len);
            if (i >= len) {
                break;
            }

            c = PyUnicode_READ_CHAR(src, i);
            if (c == sep) {
                i++;
                _skip_sp(&i, src, len);
                if (!_parse_value(&i, src, len, val, val_size, val_len)) {
                    break;
                }
                if (end) {
                    for (; i < len; i++) {
                        c = PyUnicode_READ_CHAR(src, i);
                        if (c == end) {
                            break;
                        }
                    }
                }
                break;
            }
        }
    }

    *index = i;
    return true;
}

static bool
_parse_css_key_value(
    Py_ssize_t *index,
    PyObject *src,
    Py_ssize_t len,
    Py_UCS4 key[],
    size_t key_size,
    size_t *key_len,
    Py_UCS4 val[],
    size_t val_size,
    size_t *val_len
) {
    Py_ssize_t i = *index;

    for (; i < len; i++) {
        int c = PyUnicode_READ_CHAR(src, i);
        if (_is_css_ident_head(c)) {
            if (!_parse_css_ident(&i, src, len, key, key_size, key_len)) {
                break;
            }
            _skip_sp(&i, src, len);
            if (i >= len) {
                break;
            }

            c = PyUnicode_READ_CHAR(src, i);
            if (c == ':') {
                i++;
                _skip_sp(&i, src, len);
                if (!_parse_css_value(&i, src, len, val, val_size, val_len)) {
                    break;
                }
                for (; i < len; i++) {
                    c = PyUnicode_READ_CHAR(src, i);
                    if (c == ';' || c == '}') {
                        break;
                    }
                }
                break;
            }
        }
    }

    *index = i;
    return true;
}

static PyObject *
parse_key_value(PyObject* self, PyObject* args) {
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

    if (!_parse_key_value(
        &i, src, len, 
        key, _BUF_SIZE, &key_len,
        val, _BUF_SIZE, &val_len,
        '=', 0
    )) {
        return NULL;
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

static bool
_parse_css_block_content(
    Py_ssize_t *index,
    PyObject *src,
    Py_ssize_t len,
    PyObject *dict
) {
    Py_ssize_t i = *index;
    i++;  // '{'

    for (; i < len; i++) {
        _skip_sp(&i, src, len);
        int c = PyUnicode_READ_CHAR(src, i);
        if (c == '}') {
            i++;
            break;
        }

        #undef _BUF_SIZE
        #define _BUF_SIZE 1024
        Py_UCS4 key[_BUF_SIZE] = {0};
        Py_UCS4 val[_BUF_SIZE] = {0};
        size_t key_len = 0;
        size_t val_len = 0;

        if (!_parse_css_key_value(
            &i, src, len,
            key, _BUF_SIZE, &key_len,
            val, _BUF_SIZE, &val_len
        )) {
            return false;
        }
        _skip_sp(&i, src, len);
        c = PyUnicode_READ_CHAR(src, i);
        if (c == '}') {
            i++;
            break;
        }

        PyObject *okey = PyUnicode_FromKindAndData(PyUnicode_4BYTE_KIND, key, key_len);
        PyObject *oval = PyUnicode_FromKindAndData(PyUnicode_4BYTE_KIND, val, val_len);

        if (PyDict_SetItem(dict, okey, oval) < 0) {
            Py_DECREF(okey);
            Py_DECREF(oval);
            return false;
        }
    }

    *index = i;
    return true;
}

static bool
_parse_css_block(
    Py_ssize_t *index,
    PyObject *src,
    Py_ssize_t len,
    Py_UCS4 ident[],
    size_t ident_size,
    size_t *ident_len,
    PyObject *dict
) {
    Py_ssize_t i = *index;

    for (; i < len; i++) {
        int c = PyUnicode_READ_CHAR(src, i);
        if (_is_css_ident_head(c)) {
            if (!_parse_css_ident(&i, src, len, ident, ident_size, ident_len)) {
                return false;
            }
            _skip_sp(&i, src, len);
            c = PyUnicode_READ_CHAR(src, i);
            if (c == '{') {
                if (!_parse_css_block_content(&i, src, len, dict)) {
                    return false;
                }
                break;
            } else {
                return false;
            }
        }
    }

    *index = i;
    return true;
}

static PyObject *
parse_css_block(PyObject* self, PyObject* args) {
    Py_ssize_t i = 0;
    PyObject *src = NULL;
    Py_ssize_t len = 0;

    if (!PyArg_ParseTuple(args, "nOn", &i, &src, &len)) {
        return NULL;
    }

    PyObject *dict = PyDict_New();
    if (!dict) {
        return NULL;
    }

    #undef _BUF_SIZE
    #define _BUF_SIZE 1024
    Py_UCS4 ident[_BUF_SIZE] = {0};
    size_t ident_len = 0;

    if (!_parse_css_block(
        &i, src, len,
        ident, _BUF_SIZE, &ident_len,
        dict
    )) {
        Py_DECREF(dict);
        return NULL;
    }

    PyObject* result = PyTuple_New(3);
    if (!result) {
        Py_DECREF(dict);
        return NULL;
    }

    PyTuple_SET_ITEM(result, 0, PyLong_FromSsize_t(i));
    PyTuple_SET_ITEM(result, 1, PyUnicode_FromKindAndData(PyUnicode_4BYTE_KIND, ident, ident_len));
    PyTuple_SET_ITEM(result, 2, dict); 

    return result;
}

static PyObject *
parse_css_blocks(PyObject* self, PyObject* args) {
    Py_ssize_t i = 0;
    PyObject *src = NULL;
    Py_ssize_t len = 0;

    if (!PyArg_ParseTuple(args, "nOn", &i, &src, &len)) {
        return NULL;
    }

    PyObject *blocks = PyDict_New();
    if (!blocks) {
        return NULL;
    }

    for (; i < len; i++) {
        #undef _BUF_SIZE
        #define _BUF_SIZE 1024
        Py_UCS4 ident[_BUF_SIZE] = {0};
        size_t ident_len = 0;

        PyObject *block = PyDict_New();
        if (!block) {
            Py_DECREF(blocks);
            return NULL;
        }

        if (!_parse_css_block(
            &i, src, len,
            ident, _BUF_SIZE, &ident_len, block
        )) {
            Py_DECREF(blocks);
            return NULL;
        }

        PyObject *oident = PyUnicode_FromKindAndData(PyUnicode_4BYTE_KIND, ident, ident_len);
        if (!oident) {
            Py_DECREF(blocks);
            return NULL;
        }

        if (PyDict_SetItem(blocks, oident, block) < 0) {
            Py_DECREF(blocks);
            return NULL;
        }
    }

    PyObject* result = PyTuple_New(2);
    if (!result) {
        Py_DECREF(blocks);
        return NULL;
    }

    PyTuple_SET_ITEM(result, 0, PyLong_FromSsize_t(i));
    PyTuple_SET_ITEM(result, 1, blocks);

    return result;
}

static PyMethodDef MyMethods[] = {
    {"parse_key_value", parse_key_value, METH_VARARGS, "Parse key and value."},
    {"parse_css_block", parse_css_block, METH_VARARGS, "Parse CSS block."},
    {"parse_css_blocks", parse_css_blocks, METH_VARARGS, "Parse CSS blocks."},
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

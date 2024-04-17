#include <Python.h>
#include <iostream>
#include "main.cpp"

char const * testGetMove = "\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
|18|85|0|3|X....|";


char const * testRateMove = "\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
|\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000000000\
0000111100\
|18|85|0|3|X....|";


static PyObject *method_rate_move(PyObject *self, PyObject *args) {
      char *test_input = NULL;
    //   const char *test_input = testRateMove;

      if(!PyArg_ParseTuple(args, "s", &test_input)){
        return NULL;
      }
      std::string result = mainProcess(test_input, RATE_MOVE);

      return Py_BuildValue("s", result.c_str());
};


static PyObject *method_get_move(PyObject *self, PyObject *args) {
      char *test_input = NULL;

      if(!PyArg_ParseTuple(args, "s", &test_input)){
        return NULL;
      }

      std::string result = mainProcess(test_input, GET_MOVE);

      return Py_BuildValue("s", result.c_str());
};

static PyMethodDef StackRabbitMethods[] = {
    {"get_move", method_get_move, METH_VARARGS, "Python interface for StackRabbit's GET_MOVE"},
    {"rate_move", method_rate_move, METH_VARARGS, "Python interface for StackRabbit's RATE_MOVE"},
    {NULL, NULL, 0, NULL}
};


static struct PyModuleDef stackrabbit_module = {
    PyModuleDef_HEAD_INIT,
    "stackrabbit",
    "Python interface for StackRabbit",
    -1,
    StackRabbitMethods
};

PyMODINIT_FUNC PyInit_stackrabbit(void) {
    return PyModule_Create(&stackrabbit_module);
}

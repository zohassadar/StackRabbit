#include <Python.h>
#include <iostream>
#include "main.cpp"

// example from entrypoint.cpp
char const * testInput = "\
0000000000\
0000000000\
0000000100\
0000000111\
0000000111\
0000001111\
0000011110\
0000011111\
0000011111\
0001011111\
0001111110\
1001111110\
1001111110\
1001111110\
1001111110\
1101111111\
1101111111\
1111111110\
1111111110\
1101111111\
|18|85|5|1|X....|";

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

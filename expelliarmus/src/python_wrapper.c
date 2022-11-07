#include <Python.h>
#include "expelliarmus.h"

PyMODINIT_FUNC PyInit_expelliarmus(void){
	PyObject* module; 
	static struct PyModuleDef moduledef = {
		PyModuleDef_HEAD_INIT, 
		"expelliarmus", 
		NULL, 
		-1, 
		NULL, 
		NULL, 
		NULL, 
		NULL, 
		NULL, 
	};
	module = PyModule_Create(&moduledef);
	if (!module) return NULL; 
	return module; 
}
   

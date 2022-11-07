#include <Python.h>
#include "expelliarmus.h"

PyMODINIT_FUNC PyInit_expelliarmus(void){
	PyObject* module; 
	static struct PyModuleDef moduledef = {
		PyModuleDef_HEAD_INIT, 
		"expelliarmus", 
		NULL, 
		-1, 
		{
			"read_dat", 
			"read_evt2", 
			"read_evt3", 
			"cut_dat", 
			"cut_evt2", 
			"cut_evt3"
		}, 
		NULL, 
		NULL, 
		NULL, 
		NULL, 
	};
	module = PyModule_Create(&moduledef);
	if (!module) return NULL; 
	return module; 
}
   

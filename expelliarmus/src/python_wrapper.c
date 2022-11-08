#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "expelliarmus.h"

PyMODINIT_FUNC PyInit_expelliarmus(void){
	PyObject* module; 
	static struct PyModuleDef moduledef = {
		PyModuleDef_HEAD_INIT, 
		"expelliarmus", 
		NULL, 
		-1, 
		(const PyMethodDef*) {
			{"read_dat", read_dat, 0, NULL}, 
			{"read_evt2", read_evt2, 0, NULL},
			{"read_evt3", read_evt3, 0, NULL}, 
			{"cut_dat", cut_dat, 0, NULL}, 
			{"cut_evt2", cut_evt2, 0, NULL}, 
			{"cut_evt3", cut_evt3, 0, NULL} 
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
   

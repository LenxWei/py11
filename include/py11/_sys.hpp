namespace py{

/* py system utils
*****************/

/**
 * @addtogroup utils
 * Utility functoins
 * @{
 */

/** pass the args to set the correct sys.path in python
 */
inline void set_arg(int argc, char *argv[])
{
	static details::py_initer_wrap __init;

	PySys_SetArgv(argc, argv);
}

/** py print err to stderr.
 * print python's error message and stack information.
 */
inline void print_err(int set_sys_last_vars=1)
{
	if(PyErr_Occurred())
		PyErr_PrintEx(set_sys_last_vars);
}

/** py import.
 * @throw val_err
 */
inline obj import(const char* module_name)
{
	static details::py_initer_wrap __init;

    PyObject* p = PyImport_ImportModule(module_name);
    if(p == NULL)
        throw val_err("py import () failed");
    return p;
}

/**
 * @}
 */

}; // ns py

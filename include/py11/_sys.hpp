namespace py{

/* py system utils
*****************/

/** set the main prog for pylib
 * @param program_name usually use argv[0]
 */
inline void set_prog(char* program_name = NULL)
{
    if(program_name){
        Py_SetProgramName(program_name);
    }
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

}; // ns py

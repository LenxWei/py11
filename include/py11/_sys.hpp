/* py system utils
*****************/

/** init the py lib
 * @param program_name usually use argv[0]
 */
inline void init(char* program_name = NULL)
{
    if(program_name){
        Py_SetProgramName(program_name);
    }
    Py_Initialize();
}

/** finalize the py lib
 */
inline void fini()
{
    Py_Finalize();
}

/** py import.
 * @throw val_err
 */
inline obj import(const char* module_name)
{
    PyObject* p = PyImport_ImportModule(module_name);
    if(p == NULL)
        throw val_err("py import () failed");
    return p;
}
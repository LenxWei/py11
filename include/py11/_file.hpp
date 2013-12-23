namespace py{

/** file object
 */

class file: public obj{
protected:
    void type_check(PyObject* p)noexcept(!PY11_ENFORCE)
    {
        if(PY11_ENFORCE && p){
            if(!PyFile_Check(p))
                throw type_err("file ctor failed");
        }
    }
    
    void type_check(const obj& o)noexcept(!PY11_ENFORCE)
    {
        type_check((PyObject*)o.p());
    }
    
public:
    /** ctor.
     */
    file()=default;
            
    file(const obj& o)
    {
        type_check(o);
        enter(o.p());
    }
    
    file& operator=(const obj& o)
    {
        if(o.p()!=_p){
            type_check(o);
            release();
            enter(o.p());
        }
        return *this;
    }

    file(obj&& o)noexcept(!PY11_ENFORCE)
    {
        type_check(o);
        _p = o.transfer();
    }
    
    file& operator=(obj&& o)noexcept(!PY11_ENFORCE)
    {
        if(o.p()!=_p){
            type_check(o);
            release();
            _p = o.transfer();
        }
        return *this;
    }

    /** create from Py functions.
     * in most of cases, need not to inc ref.
     * please read python doc carefully
     */
    file(PyObject* p, bool borrowed = false)
    {
        type_check(p);
        _p = p;
        if(borrowed)
            Py_XINCREF(_p);
    }
    
    file& operator=(PyObject* p)
    {
        if(p!=_p){
            type_check(p);
            release();
            _p = p;
        }
        return *this;
    }

    // own methods
    
    /** ctor.
     * @throw val_err
     */
    file(const char* fn, const char* mode = "r")
    {
        _p = PyFile_FromString((char*)fn, (char*)mode);
        if(!_p)
            throw val_err("file ctor failed");
    }

    /** readline
     * @param n if n == 0, read a whole line; if n > 0, read at most n chars.
     * @throw io_err
     * [FIXME] throw eof_err when n < 0
     */
    str readline(int n = 0)
    {
        PyObject* r = PyFile_GetLine(_p, n);
        if(!r)
            throw io_err("readline failed");
        return r;
    }
    
    /** get the filename
     */
    str filename()const
    {
        return str(PyFile_Name(_p), true);
    }
    
    /** write an object
     * @param use_raw  if true, use str(), otherwise, use repr()
     * @throw io_err
     */
    void write(const obj& o, bool use_raw = false)
    {
        int r = PyFile_WriteObject(o.p(), _p, use_raw ? Py_PRINT_RAW : 0);
        if(r == -1)
            throw io_err("write failed");
            
    }
    
    /** write a string
     * @throw io_err
     */
    void write(const char* s)
    {
        int r = PyFile_WriteString(s, _p);
        if(r == -1)
            throw io_err("write failed");
    }
};

}; // ns py


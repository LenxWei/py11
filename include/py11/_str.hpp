/** py str.
 */
class str: public seq{
protected:
    void type_check(PyObject* p)noexcept(!PY11_ENFORCE)
    {
        if(PY11_ENFORCE && p){
            if(!PyString_Check(p))
                throw type_err("creating str failed");
        }
    }
    
    void type_check(const obj& o)noexcept(!PY11_ENFORCE)
    {
        type_check((PyObject*)o.p());
    }
    
public:
    /** ctor.
     */
    str()=default;
    
    str(const obj& o)noexcept(!PY11_ENFORCE)
    {
        type_check(o);
        enter(o.p());
    }
    
    str& operator=(const obj& o)noexcept(!PY11_ENFORCE)
    {
        if(o.p()!=_p){
            type_check(o);
            release();
            enter(o.p());
        }
        return *this;
    }

    str(obj&& o)noexcept(!PY11_ENFORCE)
    {
        type_check(o);
        _p = o.transfer();
    }
    
    str& operator=(obj&& o)noexcept(!PY11_ENFORCE)
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
    str(PyObject* p, bool borrowed = false)
    {
        type_check(p);
        _p = p;
        if(borrowed)
            Py_XINCREF(_p);
    }
    
    str& operator=(PyObject* p)
    {
        if(p!=_p){
            type_check(p);
            release();
            _p = p;
        }
        return *this;
    }

    /** ctor.
     */
    str(const char* s)
    {
        if(s){
            _p = PyString_FromString(s);
            if(_p)
                return; 
        }
        throw val_err("str failed");
    }

    str(const char* s, Py_ssize_t len)
    {
        _p = PyString_FromStringAndSize(s, len);
        if(_p)
            return; 
        throw val_err("str failed");
    }
    
    /** =
     */
    str& operator = (const char* s)
    {
        release();
        if(s){
            _p = PyString_FromString(s);
            if(_p)
                return *this; 
        }
        throw val_err("str failed");
    }
    
    // container methods
    
    /** 'len' as 'size'.
     * @throw type_err
     */
    long size()const
    {
        long r = PyString_Size(_p);
        if(r != -1)
            return r;
        throw type_err("len failed");
    }

    /** c_str().
     * @throw type_err
     */
    const char* c_str()const
    {
        const char* p = PyString_AsString(_p);
        return p;
    }

    /** get item.
     * Warning, a new obj will be got! not a reference to the original one!
     * @throw index_err
     */
    const str operator [](Py_ssize_t i)const
    {
        PyObject* p = PySequence_GetItem(_p, i);
        if(!p){
            throw index_err("non-existing item");
        }
        return p;
    }
    
    /** slice, [i:j].
     * @throw type_err
     */
    str sub(int i, int j = std::numeric_limits<int>::max())const
    {
        PyObject* p = PySequence_GetSlice(_p, i, j);
        if(!p)
            throw type_err("sub failed");
        return p;
    }
    
    /** op +.
     * @throw type_err
     */
    str operator += (const obj& o)
    {
        PyString_Concat(&_p, o.p());
        if(_p)
            return *this;
        throw type_err("op + failed");        
    }

    /** op %.
     * @throw type_err
     */
    str operator % (const obj& o)const
    {
        PyObject* r = PyString_Format(_p, o.p());
        if(r)
            return r;
        throw type_err("op + failed");        
    }

    /** decode
     * @throw val_err
     */
    str decode(const str& s, const char* encoding, const char* errors)
    {
        PyObject* p = PyString_AsDecodedObject(s.p(), encoding, errors);
        if(p)
            return p;
        throw val_err("decode failed");
    }


    /** encode
     * @throw val_err
     */
    str encode(const str& s, const char* encoding, const char* errors)
    {
        PyObject* p = PyString_AsEncodedObject(s.p(), encoding, errors);
        if(p)
            return p;
        throw val_err("encode failed");
    }
};


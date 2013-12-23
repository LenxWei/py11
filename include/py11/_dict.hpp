namespace py{

/** py dict.
 */
class dict: public obj{
protected:
    void type_check(PyObject* p)noexcept(!PY11_ENFORCE)
    {
        if(PY11_ENFORCE && p){
            if(!PyDict_Check(p))
                throw type_err("creating dict failed");
        }
    }
    
    void type_check(const obj& o)noexcept(!PY11_ENFORCE)
    {
        type_check((PyObject*)o.p());
    }
    
public:
    /** ctor.
     */
    dict()=default;
    
    dict(const obj& o)noexcept(!PY11_ENFORCE)
    {
        type_check(o);
        enter(o.p());
    }
    
    dict& operator=(const obj& o)noexcept(!PY11_ENFORCE)
    {
        if(o.p()!=_p){
            type_check(o);
            release();
            enter(o.p());
        }
        return *this;
    }

    dict(obj&& o)noexcept(!PY11_ENFORCE)
    {
        type_check(o);
        _p = o.transfer();
    }
    
    dict& operator=(obj&& o)noexcept(!PY11_ENFORCE)
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
    dict(PyObject* p, bool borrowed = false)
    {
        type_check(p);
        _p = p;
        if(borrowed)
            Py_XINCREF(_p);
    }
    
    dict& operator=(PyObject* p)
    {
        if(p!=_p){
            type_check(p);
            release();
            _p = p;
        }
        return *this;
    }

    /** ctor.
     * @throw val_err
     */
    dict(std::initializer_list<tuple> l)
    {
        _p = PyDict_New();
        for(auto &x: l){
            if(PyDict_SetItem(_p, x[0].p(), x[1].p()) == -1)
                throw val_err("bad key/value");
        }
    }

    /** = {...}
     * @throw val_err
     */
    dict& operator = (std::initializer_list<tuple> l)
    {
        release();
        _p = PyDict_New();
        for(auto &x: l){
            if(PyDict_SetItem(_p, x[0].p(), x[1].p()) == -1)
                throw val_err("bad key/value");
        }
        return *this;
    }
    
    // container methods
    
    /** 'len' as 'size'.
     * @throw type_err
     */
    long size()const
    {
        long r = PyDict_Size(_p);
        if(r != -1)
            return r;
        throw type_err("len failed");
    }

    /** 'in' as 'has'.
     * @throw type_err
     */
    bool has(const obj& x)const
    {
        int r = PyDict_Contains(_p, x.p());
        if(r != -1)
            return r;
        throw type_err("has failed");
    }

    /** get item.
     * Warning, a new obj will be got! not a reference to the original one!
     * @throw index_err
     */
    const obj operator [](const obj& k)const
    {
        PyObject* p = PyDict_GetItem(_p, k.p());
        if(!p){
            throw index_err("non-existing item");
        }
        return p;
    }
    
    /** set_item.
     * @throw index_err
     */
    void set_item(const obj& k, const obj& value)
    {
        int r = PyDict_SetItem(_p, k.p(), value.p());
        if(r == -1)
            throw index_err("set_item failed");
    }
    
    /** del_item.
     * @throw index_err
     */
    void del_item(const obj& k)
    {
        int r = PyDict_DelItem(_p, k.p());
        if(r == -1)
            throw index_err("del_item failed");
    }
    
    /** items.
     * @throw val_err
     */
    list items()const
    {
        if(!_p)
            throw val_err("items failed");
        return PyDict_Items(_p);
    }
    
    /** keys.
     * @throw val_err
     */
    list keys()const
    {
        if(!_p)
            throw val_err("keys failed");
        return PyDict_Keys(_p);
    }
            
    /** values.
     * @throw val_err
     */
    list values()const
    {
        if(!_p)
            throw val_err("values failed");
        return PyDict_Values(_p);
    }
            
    /** clear.
     * @throw val_err
     */
    void clear()
    {
        if(!_p)
            throw val_err("clear failed");
        PyDict_Clear(_p);
    }
    
    /** copy.
     * @throw val_err
     */
    dict copy()
    {
        if(!_p)
            throw val_err("copy failed");
        return PyDict_Copy(_p);
    }
    
    /** update.
     * @throw val_err
     */
    void update(const dict& b)
    {
        if(_p && !b.is_null()){
            int r = PyDict_Update(_p, b.p());
            if(r != -1)
                return;
        }
        throw val_err("update failed");
    }
};


}; // py

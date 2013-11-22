/** py list.
 */
class list: public seq{
protected:
    void type_check(PyObject* p)noexcept(!PY11_ENFORCE)
    {
        if(PY11_ENFORCE && p){
            if(!PyList_Check(p))
                throw type_err("creating list failed");
        }
    }
    
    void type_check(const obj& o)noexcept(!PY11_ENFORCE)
    {
        type_check((PyObject*)o.p());
    }
    
public:
    /** ctor.
     */
    list()=default;
    
    list(const obj& o)noexcept(!PY11_ENFORCE)
    {
        type_check(o);
        enter(o.p());
    }
    
    list& operator=(const obj& o)noexcept(!PY11_ENFORCE)
    {
        if(o.p()!=_p){
            type_check(o);
            release();
            enter(o.p());
        }
        return *this;
    }

    list(obj&& o)noexcept(!PY11_ENFORCE)
    {
        type_check(o);
        _p = o.transfer();
    }
    
    list& operator=(obj&& o)noexcept(!PY11_ENFORCE)
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
    list(PyObject* p, bool borrowed = false)
    {
        type_check(p);
        _p = p;
        if(borrowed)
            Py_XINCREF(_p);
    }
    
    list& operator=(PyObject* p)
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
    list(std::initializer_list<obj> l)
    {
        _p = PyList_New(l.size());
        long i = 0;
        for(auto &x: l){
            PyList_SET_ITEM(_p, i++, x.p());
            x.__reset();
        }
    }

    // container methods
    
    /** 'len' as 'size'.
     * @throw type_err
     */
    long size()const
    {
        long r = PyList_Size(_p);
        if(r != -1)
            return r;
        throw type_err("len failed");
    }

    /** 'in' as 'has'.
     * @throw type_err
     */
    bool has(const obj& x)const
    {
        int r = PySequence_Contains(_p, x.p());
        if(r != -1)
            return r;
        throw type_err("has failed");
    }

    /** list find a item.
     * @return index if found, -1 otherwise
     * @throw type_err
     */
    long find(const obj& o)const
    {
        long r = PySequence_Index(_p, o.p());
        return r;
    }
    
    /** list index.
     * @return index if found
     * @throw index_err if not found
     * @throw type_err
     */
    long index(const obj& o)const
    {
        long r = PySequence_Index(_p, o.p());
        if(r == -1)
            throw index_err("index failed");
        return r;
    }

    /** get a tuple clone.
     * @throw type_err
     */    
    obj to_tuple()const
    {
        obj r = PyList_AsTuple(_p);
        if(!r.is_null())
            return r;
        throw type_err("to_tuple failed");
    }

    /** get item.
     * Warning, a new obj will be got! not a reference to the original one!
     * @throw index_err
     */
    const obj operator [](Py_ssize_t i)const
    {
        PyObject* p = PyList_GetItem(_p, i);
        if(!p){
            throw index_err("non-existing item");
        }
        return p;
    }
    
    /** set_item.
     * @throw index_err
     */
    void set_item(Py_ssize_t i, const obj& value)
    {
        int r = PyList_SetItem(_p, i, value.p());
        if(r == -1)
            throw index_err("set_item failed");
    }
        
    /** slice, [i:j].
     * @throw type_err
     */
    obj sub(int i, int j = std::numeric_limits<int>::max())const
    {
        PyObject* p = PyList_GetSlice(_p, i, j);
        if(!p)
            throw type_err("sub failed");
        return p;
    }
    
    /** sort in place.
     */
    void sort()
    {
        int r = PyList_Sort(_p);
        if(r == -1)
            throw err("sort failed");
    }

    /** reverse in place.
     */
    void reverse()
    {
        int r = PyList_Reverse(_p);
        if(r == -1)
            throw err("reverse failed");
    }
    
    /** append.
     */
    void append(const obj& o)
    {
        int r = PyList_Append(_p, o.p());
        if(r == -1)
            throw err("append failed");
    }
    
    /** insert.
     */
    void insert(Py_ssize_t index, const obj& o)
    {
        int r = PyList_Insert(_p, index, o.p());
        if(r == -1)
            throw err("insert failed");
    }
};


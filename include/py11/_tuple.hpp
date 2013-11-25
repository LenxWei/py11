/** py tuple.
 */
class tuple: public seq{
protected:
    void type_check(PyObject* p)noexcept(!PY11_ENFORCE)
    {
        if(PY11_ENFORCE && p){
            if(!PyTuple_Check(p))
                throw type_err("creating tuple failed");
        }
    }
    
    void type_check(const obj& o)noexcept(!PY11_ENFORCE)
    {
        type_check((PyObject*)o.p());
    }
    
public:
    /** ctor.
     */
    tuple()=default;
    
    tuple(const obj& o)noexcept(!PY11_ENFORCE)
    {
        type_check(o);
        enter(o.p());
    }
    
    tuple& operator=(const obj& o)noexcept(!PY11_ENFORCE)
    {
        if(o.p()!=_p){
            type_check(o);
            release();
            enter(o.p());
        }
        return *this;
    }

    tuple(obj&& o)noexcept(!PY11_ENFORCE)
    {
        type_check(o);
        _p = o.transfer();
    }
    
    tuple& operator=(obj&& o)noexcept(!PY11_ENFORCE)
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
    tuple(PyObject* p, bool borrowed = false)
    {
        type_check(p);
        _p = p;
        if(borrowed)
            Py_XINCREF(_p);
    }
    
    tuple& operator=(PyObject* p)
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
    tuple(std::initializer_list<obj> l)
    {
        _p = PyTuple_New(l.size());
        long i = 0;
        for(auto &x: l){
            PyTuple_SET_ITEM(_p, i++, x.p());
            x.__reset();
        }
    }

    /** = {...}
     */
    tuple& operator = (std::initializer_list<obj> l)
    {
        release();
        _p = PyTuple_New(l.size());
        long i = 0;
        for(auto &x: l){
            PyTuple_SET_ITEM(_p, i++, x.p());
            x.__reset();
        }
        return *this;
    }
    
    // container methods
    
    /** 'len' as 'size'.
     * @throw type_err
     */
    long size()const
    {
        long r = PyTuple_Size(_p);
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

    /** tuple find a item.
     * @return index if found, -1 otherwise
     * @throw type_err
     */
    long find(const obj& o)const
    {
        long r = PySequence_Index(_p, o.p());
        return r;
    }
    
    /** tuple index.
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

    /** get item.
     * Warning, a new obj will be got! not a reference to the original one!
     * @throw index_err
     */
    const obj operator [](Py_ssize_t i)const
    {
        PyObject* p = PyTuple_GetItem(_p, i);
        if(!p){
            throw index_err("non-existing item");
        }
        return obj(p, 1);
    }
    
    /** slice, [i:j].
     * @throw type_err
     */
    obj sub(int i, int j = std::numeric_limits<int>::max())const
    {
        PyObject* p = PyTuple_GetSlice(_p, i, j);
        if(!p)
            throw type_err("sub failed");
        return p;
    }
};


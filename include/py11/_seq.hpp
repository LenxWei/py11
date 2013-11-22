/** seq object
 */
class seq: public obj{
protected:
    void type_check(PyObject* p)
    {
        if(PY11_ENFORCE && p){
            if(!PySequence_Check(p))
                throw type_err("creating seq failed");
        }
    }
    
    void type_check(const obj& o)
    {
        type_check((PyObject*)o.p());
    }
    
public:
    /** ctor.
     */
    seq()=default;
            
    seq(const obj& o)
    {
        type_check(o);
        enter(o.p());
    }
    
    seq(obj&& o)
    {
        type_check(o);
        _p = o.transfer();
    }
    
    seq& operator=(const obj& o)
    {
        if(o.p()!=_p){
            type_check(o);
            release();
            enter(o.p());
        }
        return *this;
    }

    seq& operator=(obj&& o)
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
    seq(PyObject* p, bool borrowed = false)
    {
        type_check(p);
        _p = p;
        if(borrowed)
            Py_XINCREF(_p);
    }
    
    seq& operator=(PyObject* p)
    {
        if(p!=_p){
            type_check(p);
            release();
            _p = p;
        }
        return *this;
    }

    /** tuple.
     */
    seq(std::initializer_list<obj> l):obj(l)
    {}

    /** op +.
     * @throw type_err
     */
    seq operator + (const obj& o)const
    {
        obj r = PySequence_Concat(_p, o.p());
        if(!r.is_null())
            return r;
        throw type_err("op + failed");        
    }
    
    /** op +=.
     * @throw type_err
     */
    seq& operator +=(const obj& o)
    {
        obj r = PySequence_InPlaceConcat(_p, o.p());
        if(!r.is_null())
            return *this;
        throw type_err("op += failed");        
    }

    /** op *.
     * @throw type_err
     */
    seq operator * (Py_ssize_t t)const
    {
        seq r = PySequence_Repeat(_p, t);
        if(!r.is_null())
            return r;
        throw type_err("op + failed");        
    }
    
    /** op *=.
     * @throw type_err
     */
    seq& operator *=(Py_ssize_t t)
    {
        seq r = PySequence_InPlaceRepeat(_p, t);
        if(!r.is_null())
            return *this;
        throw type_err("op += failed");        
    }

    // container methods
    
    /** 'len' as 'size'.
     * @throw type_err
     */
    long size()const
    {
        long r = PySequence_Size(_p);
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

    /** seq find a item.
     * @return index if found, -1 otherwise
     * @throw type_err
     */
    long find(const obj& o)const
    {
        long r = PySequence_Index(_p, o.p());
        return r;
    }
    
    /** seq index.
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

    /** get a list clone.
     * @throw type_err
     */    
    obj to_list()const
    {
        obj r = PySequence_List(_p);
        if(!r.is_null())
            return r;
        throw type_err("to_list failed");
    }
    
    /** get a tuple clone.
     * @throw type_err
     */    
    obj to_tuple()const
    {
        obj r = PySequence_Tuple(_p);
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
        PyObject* p = PySequence_GetItem(_p, i);
        if(!p){
            throw index_err("non-existing item");
        }
        return p;
    }
    
    /** set_item.
     * @throw index_err
     */
    void set_item(Py_ssize_t i, const obj& value)const
    {
        int r = PySequence_SetItem(_p, i, value.p());
        if(r == -1)
            throw index_err("set_item failed");
    }
    
    /** del_item.
     * @throw index_err
     */
    void del_item(Py_ssize_t i)
    {
        int r = PySequence_DelItem(_p, i);
        if(r == -1)
            throw index_err("del_item failed");
    }
    
    /** slice, [i:j].
     * @throw type_err
     */
    obj sub(int i, int j = std::numeric_limits<int>::max())const
    {
        PyObject* p = PySequence_GetSlice(_p, i, j);
        if(!p)
            throw type_err("sub failed");
        return p;
    }    
};


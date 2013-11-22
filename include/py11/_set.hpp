/** set object
 */
class set: public num{
protected:
    void type_check(PyObject* p)noexcept(!PY11_ENFORCE)
    {
        if(PY11_ENFORCE && p){
            if(!PyAnySet_Check(p))
                throw type_err("creating set failed");
        }
    }
    
    void type_check(const obj& o)noexcept(!PY11_ENFORCE)
    {
        type_check((PyObject*)o.p());
    }
    
public:
    /** ctor.
     */
    set()=default;
            
    set(const obj& o)
    {
        type_check(o);
        enter(o.p());
    }
    
    set& operator=(const obj& o)
    {
        if(o.p()!=_p){
            type_check(o);
            release();
            enter(o.p());
        }
        return *this;
    }

    set(obj&& o)noexcept(!PY11_ENFORCE)
    {
        type_check(o);
        _p = o.transfer();
    }
    
    set& operator=(obj&& o)noexcept(!PY11_ENFORCE)
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
    set(PyObject* p, bool borrowed = false)
    {
        type_check(p);
        _p = p;
        if(borrowed)
            Py_XINCREF(_p);
    }
    
    set& operator=(PyObject* p)
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
    set(std::initializer_list<obj> l)
    {
        _p = PySet_New(NULL);
        for(auto &x: l){
            add(x);
        }
    }
    
    set& operator=(std::initializer_list<obj> l)
    {
        release();
        _p = PySet_New(NULL);
        for(auto &x: l){
            add(x);
        }
        return *this;
    }
    
    /** op &.
     * @throw type_err
     */
    set operator & (const obj& o)const
    {
        PyObject* r = PyNumber_And(_p, o.p());
        if(r)
            return r;
        throw type_err("op & failed");        
    }
    
    /** op &=.
     * @throw type_err
     */
    set& operator &=(const obj& o)
    {
        PyObject* r = PyNumber_InPlaceAnd(_p, o.p());
        if(r)
            return *this;
        throw type_err("op &= failed");        
    }
    
    /** op |.
     * @throw type_err
     */
    set operator | (const obj& o)const
    {
        PyObject* r = PyNumber_Or(_p, o.p());
        if(r)
            return r;
        throw type_err("op | failed");        
    }
    
    /** op |=.
     * @throw type_err
     */
    set& operator |=(const obj& o)
    {
        PyObject* r = PyNumber_InPlaceOr(_p, o.p());
        if(r)
            return *this;
        throw type_err("op |= failed");        
    }    
    
    /** op -.
     * @throw type_err
     */
    set operator - (const obj& o)const
    {
        PyObject* r = PyNumber_Subtract(_p, o.p());
        if(r)
            return r;
        throw type_err("op - failed");        
    }
    
    /** op -=.
     * @throw type_err
     */
    set& operator -=(const obj& o)
    {
        PyObject* r = PyNumber_InPlaceSubtract(_p, o.p());
        if(r)
            return *this;
        throw type_err("op -= failed");        
    }    

    /** add an element.
     * @throw val_err
     */    
    void add(const obj& o)
    {
        if(_p){
            int r = PySet_Add(_p, o.p());
            if(r != -1)
                return;
        }
        throw val_err("set add failed");
    }
    
    /** discard an element if present
     * @throw err
     */
    void discard(const obj& o)
    {
        int r = PySet_Discard(_p, o.p());
        if(r == -1)
            throw err("set discard failed");
    }
    
    /** remove an element if present
     * @throw index_err if not present
     * @throw err
     */
    void remove(const obj& o)
    {
        int r = PySet_Discard(_p, o.p());
        if(r == -1)
            throw err("set remove failed");
        else if(r == 0)
            throw index_err("set remove failed");
    }
    
    /** pop an element
     * @throw index_err if empty
     * @throw err
     */
    obj pop()
    {
        if(_p){
            PyObject* r = PySet_Pop(_p);
            if(r)
                return r;
        }
        throw index_err("set pop failed");
    }
    
    /** add elements from an iterable object.
     */
    void extend(const obj& o)
    {
        for(auto &x: o){
            add(x);
        }
    }

    /** clear.
     */
    void clear()
    {
        if(_p)
            PySet_Clear(_p);
    }

};


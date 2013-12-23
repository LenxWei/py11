namespace py{

/** num object
 */
class num: public obj{
protected:
    void type_check(PyObject* p)noexcept(!PY11_ENFORCE)
    {
        if(PY11_ENFORCE && p){
            if(!PyNumber_Check(p))
                throw type_err("creating num failed");
        }
    }
    
    void type_check(const obj& o)noexcept(!PY11_ENFORCE)
    {
        type_check((PyObject*)o.p());
    }
    
public:
    /** ctor.
     */
    num()=default;
            
    num(const obj& o)
    {
        type_check(o);
        enter(o.p());
    }
    
    num& operator=(const obj& o)
    {
        if(o.p()!=_p){
            type_check(o);
            release();
            enter(o.p());
        }
        return *this;
    }

    num(obj&& o)noexcept(!PY11_ENFORCE)
    {
        type_check(o);
        _p = o.transfer();
    }
    
    num& operator=(obj&& o)noexcept(!PY11_ENFORCE)
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
    num(PyObject* p, bool borrowed = false)
    {
        type_check(p);
        _p = p;
        if(borrowed)
            Py_XINCREF(_p);
    }
    
    num& operator=(PyObject* p)
    {
        if(p!=_p){
            type_check(p);
            release();
            _p = p;
        }
        return *this;
    }

    /** op +.
     * @throw type_err
     */
    num operator + (const obj& o)const
    {
        PyObject* r = PyNumber_Add(_p, o.p());
        if(r)
            return r;
        throw type_err("op + failed");        
    }
    
    /** op +=.
     * @throw type_err
     */
    num& operator +=(const obj& o)
    {
        PyObject* r = PyNumber_InPlaceAdd(_p, o.p());
        if(r)
            return *this;
        throw type_err("op += failed");        
    }

    /** op *.
     * @throw type_err
     */
    num operator * (const obj& o)const
    {
        PyObject* r = PyNumber_Multiply(_p, o.p());
        if(r)
            return r;
        throw type_err("op * failed");        
    }
    
    /** op *=.
     * @throw type_err
     */
    num& operator *=(const obj& o)
    {
        PyObject* r = PyNumber_InPlaceMultiply(_p, o.p());
        if(r)
            return *this;
        throw type_err("op *= failed");        
    }

    /** op &.
     * @throw type_err
     */
    num operator & (const obj& o)const
    {
        PyObject* r = PyNumber_And(_p, o.p());
        if(r)
            return r;
        throw type_err("op & failed");        
    }
    
    /** op &=.
     * @throw type_err
     */
    num& operator &=(const obj& o)
    {
        PyObject* r = PyNumber_InPlaceAnd(_p, o.p());
        if(r)
            return *this;
        throw type_err("op &= failed");        
    }
    
    /** op |.
     * @throw type_err
     */
    num operator | (const obj& o)const
    {
        PyObject* r = PyNumber_Or(_p, o.p());
        if(r)
            return r;
        throw type_err("op | failed");        
    }
    
    /** op |=.
     * @throw type_err
     */
    num& operator |=(const obj& o)
    {
        PyObject* r = PyNumber_InPlaceOr(_p, o.p());
        if(r)
            return *this;
        throw type_err("op |= failed");        
    }    
    
    /** op -.
     * @throw type_err
     */
    num operator - (const obj& o)const
    {
        PyObject* r = PyNumber_Subtract(_p, o.p());
        if(r)
            return r;
        throw type_err("op - failed");        
    }
    
    /** op -=.
     * @throw type_err
     */
    num& operator -=(const obj& o)
    {
        PyObject* r = PyNumber_InPlaceSubtract(_p, o.p());
        if(r)
            return *this;
        throw type_err("op -= failed");        
    }    
    
};

}; // ns py

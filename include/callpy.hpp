#include <Python.h>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <initializer_list>
#include <limits>

namespace py {

/** exception: err.
 */
class err: public std::exception{
protected:
    const char* _what;
public:
    err():_what(NULL)
    {}
    
    err(const char* what):_what(what)
    {}
    
    virtual const char* what()const noexcept
    {
        return _what;
    }
};


/** exception: index_err.
 */
class index_err: public err{
public:
    index_err(const char* what):err(what)
    {}
};

/** exception: type_err.
 */
class type_err: public err{
public:
    type_err(const char* what):err(what)
    {}
};

/** exception: val_err.
 */
class val_err: public err{
public:
    val_err(const char* what):err(what)
    {}
};


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

class iter;

/** wrapper of PyObject.
 */
class obj{
friend obj list(std::initializer_list<obj> l);
protected:
    PyObject* _p;

    void __reset()const
    {
        // only for stolen references
        const_cast<obj&>(*this)._p = NULL;
    }
    
    void enter(PyObject* p)
    {
        _p = p;
        __enter();
    }
    
    void __enter()
    {
        if(_p)
            Py_INCREF(_p);
    }
        
public:
    obj():_p(NULL)
    {}
    
    /** copy ctor.
     */
    obj(const obj& o):_p(o._p)
    {
        __enter();
    }
    
    obj(obj& o):_p(o._p)
    {
        __enter();
    }
    
    obj& operator=(const obj& o)
    {
        if(o._p!=_p){
            release();
            enter(o._p);
        }
        return *this;
    }

    /** create from Py functions, need not to inc ref
     */
    obj(PyObject* p):_p(p)
    {}
    
    obj& operator=(PyObject* p)
    {
        if(p!=_p){
            release();
            _p = p;
        }
        return *this;
    }
    
    /** move ctor.
     */
    obj(obj&& o)noexcept:_p(o._p)
    {
        o._p = NULL;
    }

    /** tuple.
     */
    obj(std::initializer_list<obj> l):_p(PyTuple_New(l.size()))
    {
        long i = 0;
        for(auto &x: l){
            PyTuple_SET_ITEM(_p, i++, x.p());
            x.__reset();
        }
    }
    
    /** dtor.
     */
    ~obj()
    {
        release();
    }

    /** release the contained object
     */
    void release()
    {
        if(_p){
            Py_DECREF(_p);
            _p = NULL;
        }
    }
    
    /** test null.
     */
    bool is_null()const
    {
        return _p == NULL;
    }
    
    // cast
    
    /** is false
     */    
    bool operator !()const
    {
        return (!_p) || PyObject_Not(_p);
    }
    
    /** int.
     */
    obj(int i):_p(PyInt_FromLong(i))
    {}

    /** long.
     */
    obj(long i):_p(PyInt_FromLong(i))
    {}

    /** is_int.
     */
    bool is_int()const
    {
        return _p && PyInt_Check(_p);
    }
    
    /** get long from obj
     * @throw type_err
     */
    long as_long()const
    {
        long r = PyInt_AsLong(_p);
        if(PyErr_Occurred() != NULL){
            throw type_err("as_long failed");
        }
        return r;
    }
    
    /** double ctor.
     */
    obj(double v):_p(PyFloat_FromDouble(v))
    {}

    /** is_float.
     */
    bool is_float()const
    {
        return _p && PyFloat_Check(_p);
    }
    
    /** as_double from obj.
     * @throw type_err
     */
    double as_double()const
    {
        double r = PyFloat_AsDouble(_p);
        if(PyErr_Occurred() != NULL){
            throw type_err("as_double failed");
        }
        return r;
    }
    
    /** str.
     */    
    obj(const char* s):_p(PyString_FromString(s))
    {}

    /** c_str().
     * @throw type_err
     */
    const char* c_str()const
    {
        const char* p = PyString_AsString(_p);
        if(!p)
            throw type_err("c_str failed");
        
        return p;
    }

    /** py object type.
     * @throw type_err
     */
    obj type()const
    {
        if(_p){
            PyObject* p = PyObject_Type(_p);
            if(!p)
                throw type_err("type failed");
            return p;
        }
        return obj();
    }
    
    /** type check
     * @throw type_err
     * @throw val_err
     */
    bool is_a(const obj& t)const
    {
        if(t.is_null())
            throw type_err("is_a with null type");
            
        int r = PyObject_TypeCheck(_p, t._p->ob_type);
        if(r == -1)
            throw val_err("is_a failed");
            
        return r;
    }

    // attr
            
    /** has attr.
     */
    bool has_attr(const obj& o)const
    {
        return PyObject_HasAttr(_p, o._p);
    }

    /** has attr.
     */
    bool has_attr(const char* s)const
    {
        return PyObject_HasAttrString(_p, s);
    }

    /** get attr.
     * @throw index_err
     */
    obj attr(const obj& o)const
    {
        PyObject* p = PyObject_GetAttr(_p, o._p);
        if(!p){
            throw index_err("non-existing attr");
        }
        return p;
    }

    /** get attr.
     * @throw index_err
     */
    obj attr(const char* s)const
    {
        PyObject* p = PyObject_GetAttrString(_p, s);
        if(!p){
            throw index_err("non-existing attr");
        }
        return p;
    }

    /** get attr, short form.
     * @throw index_err
     */
    obj a(const char* s)const
    {
        PyObject* p = PyObject_GetAttrString(_p, s);
        if(!p){
            throw index_err("non-existing attr");
        }
        return p;
    }

    /** set attr.
     * @throw index_err
     */
    void set_attr(const obj& a, const obj& v)const
    {
        int r = PyObject_SetAttr(_p, a._p, v._p);
        if(r == -1)
            throw index_err("set_attr failed");
    }

    /** set attr.
     * @throw index_err
     */
    void set_attr(const char* a, const obj& v)const
    {
        int r = PyObject_SetAttrString(_p, a, v._p);
        if(r == -1)
            throw index_err("set_attr failed");
    }

    /** del attr.
     * @throw index_err
     */
    void del_attr(const obj& a)const
    {
        int r = PyObject_DelAttr(_p, a._p);
        if(r == -1)
            throw index_err("del_attr failed");
    }

    /** del attr.
     * @throw index_err
     */
    void del_attr(const char* a)const
    {
        int r = PyObject_DelAttrString(_p, a);
        if(r == -1)
            throw index_err("del_attr failed");
    }

    // comparison
    
    /** <
     * @throw val_err
     */
    bool operator < (const obj& o)const
    {
        if(_p == o._p)
            return false;
        if(_p == NULL)
            return true;
        if(o._p == NULL)
            return false;
        int r = PyObject_RichCompareBool(_p, o._p, Py_LT);
        if(r == -1){
            throw val_err("op < failed");
        }
        return r;
    }
    
    bool operator <= (const obj& o)const
    {
        if(_p == o._p)
            return true;
        if(_p == NULL)
            return true;
        if(o._p == NULL)
            return false;
        int r = PyObject_RichCompareBool(_p, o._p, Py_LE);
        if(r == -1){
            throw val_err("op <= failed");
        }
        return r;
    }
    
    bool operator == (const obj& o)const
    {
        if(_p == o._p)
            return true;
        if(_p == NULL)
            return false;
        if(o._p == NULL)
            return false;
        int r = PyObject_RichCompareBool(_p, o._p, Py_EQ);
        if(r == -1){
            throw val_err("op == failed");
        }
        return r;
    }
    
    bool operator != (const obj& o)const
    {
        if(_p == o._p)
            return false;
        if(_p == NULL)
            return true;
        if(o._p == NULL)
            return true;
        int r = PyObject_RichCompareBool(_p, o._p, Py_NE);
        if(r == -1){
            throw val_err("op != failed");
        }
        return r;
    }
    
    bool operator > (const obj& o)const
    {
        if(_p == o._p)
            return false;
        if(_p == NULL)
            return false;
        if(o._p == NULL)
            return true;
        int r = PyObject_RichCompareBool(_p, o._p, Py_GT);
        if(r == -1){
            throw val_err("op > failed");
        }
        return r;
    }
    
    bool operator >= (const obj& o)const
    {
        if(_p == o._p)
            return true;
        if(_p == NULL)
            return false;
        if(o._p == NULL)
            return true;
        int r = PyObject_RichCompareBool(_p, o._p, Py_GE);
        if(r == -1){
            throw val_err("op >= failed");
        }
        return r;
    }
    
    /** op +.
     * @throw type_err
     */
    obj operator + (const obj& o)const
    {
        if(PySequence_Check(_p)){     
            obj r = PySequence_Concat(_p, o._p);
            if(!!r)
                return r;
        }
        throw type_err("op + failed");        
    }
    
    /** op +=.
     * @throw type_err
     */
    obj& operator +=(const obj& o)
    {
        if(PySequence_Check(_p)){     
            obj r = PySequence_InPlaceConcat(_p, o._p);
            if(!!r)
                return *this;
        }
        throw type_err("op += failed");        
    }
    
    /** op &.
     * @throw type_err
     */
    obj operator & (const obj& o)const
    {
        obj r = PyNumber_And(_p, o._p);
        if(!!r)
            return r;
        throw type_err("op & failed");        
    }
    
    /** op &=.
     * @throw type_err
     */
    obj& operator &=(const obj& o)
    {
        obj r = PyNumber_InPlaceAnd(_p, o._p);
        if(!!r)
            return *this;
        throw type_err("op &= failed");        
    }
    
    /** op |.
     * @throw type_err
     */
    obj operator | (const obj& o)const
    {
        obj r = PyNumber_Or(_p, o._p);
        if(!!r)
            return r;
        throw type_err("op | failed");        
    }
    
    /** op |=.
     * @throw type_err
     */
    obj& operator |=(const obj& o)
    {
        obj r = PyNumber_InPlaceOr(_p, o._p);
        if(!!r)
            return *this;
        throw type_err("op |= failed");        
    }    
    
    /** op -.
     * @throw type_err
     */
    obj operator - (const obj& o)const
    {
        obj r = PyNumber_Subtract(_p, o._p);
        if(!!r)
            return r;
        throw type_err("op - failed");        
    }
    
    /** op -=.
     * @throw type_err
     */
    obj& operator -=(const obj& o)
    {
        obj r = PyNumber_InPlaceSubtract(_p, o._p);
        if(!!r)
            return *this;
        throw type_err("op -= failed");        
    }    
    
    // output
    
    /** repr.
     * @throw val_err
     */
    obj repr()const
    {
        PyObject* p = PyObject_Repr(_p);
        if(p == NULL)
            throw val_err("repr failed");
        return p;
    }
    
    /** str.
     * @throw val_err
     */
    obj str()const
    {
        PyObject* p = PyObject_Str(_p);
        if(p == NULL)
            throw val_err("str failed");
        return p;
    }
    
    /** unicode.
     * @throw val_err
     */
    obj unicode()const
    {
        PyObject* p = PyObject_Unicode(_p);
        if(p == NULL)
            throw val_err("unicode failed");
        return p;
    }
    
    void output(std::ostream& s)const
    {
        if(!_p){
            s << "<NULL>";
        }
        else{
            const char* p = PyString_AsString(_p);
            if(p){
                s << p;
            }
            else{
                s << str().c_str();
            }
        }
    }

    // call
        
    /** test callable.
     */
    bool is_callable()const
    {
        return (_p) && (PyCallable_Check(_p));
    }
    
    /** call using operator.
     * @throw type_err
     */
    template<typename ...argT>obj operator ()(argT&& ...a)const
    {
        PyObject* r = PyObject_CallFunctionObjArgs(_p, obj(a).p()..., NULL);
        if(r == NULL)
            throw type_err("operator() failed");
        return r;
    }

    /** call with key/value pairs.
     * @throw type_err
     */
    obj call(const obj& args, const obj& kw)const
    {
        PyObject* r = PyObject_Call(_p, args._p, kw._p);
        if(r == NULL)
            throw type_err("call failed");
        return r;
    }
        
    // container methods
    
    /** 'len' as 'size'.
     * @throw type_err
     */
    long size()const
    {
        if(_p){
            if(PySequence_Check(_p)){        
                long r = PySequence_Size(_p);
                if(r != -1)
                    return r;
            }
            else if(PyMapping_Check(_p)){
                long r = PyMapping_Size(_p);
                if(r != -1)
                    return r;
            }
            else if(PyAnySet_Check(_p)){
                long r = PySet_Size(_p);
                if(r != -1)
                    return r;
            }
        }
        throw type_err("len failed");
    }

    /** 'in' as 'has'.
     * @throw type_err
     */
    bool has(const obj& x)const
    {
        if(_p){
            if(PySequence_Check(_p)){        
                int r = PySequence_Contains(p(), x._p);
                if(r != -1)
                    return r;
            }
            else if(PyMapping_Check(_p)){
                int r = PyMapping_HasKey(_p, x._p);
                if(r != -1)
                    return r;
            }
            else if(PyAnySet_Check(_p)){
                int r = PySet_Contains(_p, x._p);
                if(r != -1)
                    return r;
            }
        }
        throw type_err("has failed");
    }

    /** seq find a item.
     * @return index if found, -1 otherwise
     * @throw type_err
     */
    long find(const obj& o)const
    {
        if(PySequence_Check(_p)){        
            long r = PySequence_Index(_p, o._p);
            return r;
        }
        throw type_err("index failed");
    }
    
    /** seq index.
     * @return index if found
     * @throw index_err if not found
     * @throw type_err
     */
    long index(const obj& o)const
    {
        if(PySequence_Check(_p)){        
            long r = PySequence_Index(_p, o._p);
            if(r == -1)
                throw index_err("index failed");
            return r;
        }
        throw type_err("index failed");
    }

    /** get a list clone.
     * @throw type_err
     */    
    obj to_list()const
    {
        if(PySequence_Check(_p)){        
            obj r = PySequence_List(_p);
            if(!!r)
                return r;
        }
        throw type_err("to_list failed");
    }
    
    /** get a tuple clone.
     * @throw type_err
     */    
    obj to_tuple()const
    {
        if(PySequence_Check(_p)){        
            obj r = PySequence_Tuple(_p);
            if(!!r)
                return r;
        }
        throw type_err("to_tuple failed");
    }
        
    /** get item.
     * Warning, a new obj will be got! not a reference to the original one!
     * @throw index_err
     */
    const obj operator [](const obj& o)const
    {
        PyObject* p = PyObject_GetItem(_p, o._p);
        if(!p){
            throw index_err("non-existing item");
        }
        return p;
    }
    
    /** set_item.
     * @throw index_err
     */
    void set_item(const obj& key, const obj& value)const
    {
        int r = PyObject_SetItem(_p, key._p, value._p);
        if(r == -1)
            throw index_err("set_item failed");
    }
    
    /** del_item.
     * @throw index_err
     */
    void del_item(const obj& key)
    {
        int r = PyObject_DelItem(_p, key._p);
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
    
    /** get the begin iter.
     * @throw type_err
     */
    inline iter begin()const;
    
    /** get the end iter.
     */
    inline iter end()const;
    
    // introspection

    /** py object dir.
     * @throw val_err
     */
    obj dir()const
    {
        PyObject* p = PyObject_Dir(_p);
        if(!p)
            throw val_err("dir failed");
        return p;
    }
   
    /** refcnt.
     */
    Py_ssize_t refcnt()const
    {
        if(_p)
            return _p->ob_refcnt;
        else
            return 0;
    }
    
    /** get inner PyObject.
     */
    PyObject* p()const
    {
        return _p;
    }
        
};

/** iter for the c++11 range loop.
 */
class iter{
private:
    obj _it;
    obj _v;
    bool _fin;
public:
    iter():_fin(true)
    {}
    
    /** create iter from obj.
     * @throw type_err
     */
    iter(const obj& o):_it(PyObject_GetIter(o.p())), _fin(false)
    {
        if(_it.is_null())
            throw type_err("iter ctor failed");
        ++(*this);
    }
    
    /** operator ++.
     */
    iter& operator++()
    {
        _v = obj(PyIter_Next(_it.p()));
        if(_v.is_null())
            _fin = true;
        return *this;
    }
    
    /** operator *.
     */
    obj& operator*()
    {
        return _v;
    }
    
    bool operator==(const iter& i)const
    {
        return (i._it == _it) || (i._fin && _fin);
    }

    bool operator!=(const iter& i)const
    {
        return !(*this == i);
    }

};

/** ostream output
 */
inline std::ostream& operator <<(std::ostream& s, const obj& o)
{
    o.output(s);
    return s;
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

/** py list.
 */
inline obj list(std::initializer_list<obj> l)
{
    obj o = PyList_New(l.size());
    long i = 0;
    for(auto &x: l){
        PyList_SET_ITEM(o.p(), i++, x.p());
        x.__reset();
    }
    return o;
}

/** py set.
 * @throw type_err
 */
inline obj set(const obj& o = obj() )
{
    obj s = PySet_New(o.p());
    if(!s)
        throw type_err("creating set failed");
    return s;
}

/** py dict.
 */
inline obj dict()
{
    return PyDict_New();
}

// implementation

inline iter obj::end()const
{
    return iter();
}

inline iter obj::begin()const
{
    return iter(*this);
}

}; // ns py

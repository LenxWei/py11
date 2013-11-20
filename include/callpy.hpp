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

enum const_type {
    False = 0,
    True = 1,
    None = 2,
};

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
    
    obj(const_type i):_p(NULL)
    {
        static PyObject* c[] = {Py_False, Py_True, Py_None};
        _p = c[i];
        __enter();
    }
    
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
        return !PyObject_IsTrue(_p);
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
     */
    long as_long()const
    {
        long r = PyInt_AsLong(_p);
        if(PyErr_Occurred() != NULL){
            throw type_err("py as_long failed");
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
     */
    double as_double()const
    {
        double r = PyFloat_AsDouble(_p);
        if(PyErr_Occurred() != NULL){
            throw type_err("py as_double failed");
        }
        return r;
    }
    
    /** str.
     */    
    obj(const char* s):_p(PyString_FromString(s))
    {}

    /** c_str().
     */
    const char* c_str()const
    {
        const char* p = PyString_AsString(_p);
        if(!p)
            throw type_err("py c_str failed");
        
        return p;
    }

    /** py object type.
     */
    obj type()const
    {
        if(_p){
            PyObject* p = PyObject_Type(_p);
            if(!p)
                throw type_err("py type failed");
            return p;
        }
        return obj();
    }
    
    /** type check
     */
    bool is_a(const obj& t)const
    {
        if(t.is_null())
            throw type_err("py is_a with null type");
            
        int r = PyObject_TypeCheck(_p, t._p->ob_type);
        if(r == -1)
            throw val_err("py is_a failed");
            
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
     */
    obj attr(const obj& o)const
    {
        PyObject* p = PyObject_GetAttr(_p, o._p);
        if(!p){
            throw index_err("non-existing py attr");
        }
        return p;
    }

    /** get attr.
     */
    obj attr(const char* s)const
    {
        PyObject* p = PyObject_GetAttrString(_p, s);
        if(!p){
            throw index_err("non-existing py attr");
        }
        return p;
    }

    /** get attr, short form.
     */
    obj a(const char* s)const
    {
        PyObject* p = PyObject_GetAttrString(_p, s);
        if(!p){
            throw index_err("non-existing py attr");
        }
        return p;
    }

    /** set attr.
     */
    void set_attr(const obj& a, const obj& v)const
    {
        int r = PyObject_SetAttr(_p, a._p, v._p);
        if(r == -1)
            throw index_err("py set_attr failed");
    }

    /** set attr.
     */
    void set_attr(const char* a, const obj& v)const
    {
        int r = PyObject_SetAttrString(_p, a, v._p);
        if(r == -1)
            throw index_err("py set_attr failed");
    }

    /** del attr.
     */
    void del_attr(const obj& a)const
    {
        int r = PyObject_DelAttr(_p, a._p);
        if(r == -1)
            throw index_err("py del_attr failed");
    }

    /** del attr.
     */
    void del_attr(const char* a)const
    {
        int r = PyObject_DelAttrString(_p, a);
        if(r == -1)
            throw index_err("py del_attr failed");
    }

    // comparison
    
    /** <
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
            throw val_err("py < failed");
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
            throw val_err("py <= failed");
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
            throw val_err("py == failed");
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
            throw val_err("py != failed");
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
            throw val_err("py > failed");
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
            throw val_err("py >= failed");
        }
        return r;
    }
    
    // output
    
    /** repr.
     */
    obj repr()const
    {
        PyObject* p = PyObject_Repr(_p);
        if(p == NULL)
            throw val_err("py repr failed");
        return p;
    }
    
    /** str.
     */
    obj str()const
    {
        PyObject* p = PyObject_Str(_p);
        if(p == NULL)
            throw val_err("py str failed");
        return p;
    }
    
    /** unicode.
     */
    obj unicode()const
    {
        PyObject* p = PyObject_Unicode(_p);
        if(p == NULL)
            throw val_err("py unicode failed");
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
    
    template<typename ...argT>obj operator ()(argT&& ...a)const
    {
        PyObject* r = PyObject_CallFunctionObjArgs(_p, obj(a).p()..., NULL);
        if(r == NULL)
            throw type_err("py operator() failed");
        return r;
    }

    obj call(const obj& args, const obj& kw)const
    {
        PyObject* r = PyObject_Call(_p, args._p, kw._p);
        if(r == NULL)
            throw type_err("py call failed");
        return r;
    }
        
    // container methods
    
    /** len.
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
            else if(PySet_Check(_p) || PyFrozenSet_Check(_p)){
                long r = PySet_Size(_p);
                if(r != -1)
                    return r;
            }
        }
        throw type_err("len failed");
    }

    /** 'in' as 'has'.
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
            else if(PySet_Check(_p) || PyFrozenSet_Check(_p)){
                int r = PySet_Contains(_p, x._p);
                if(r != -1)
                    return r;
            }
        }
        throw type_err("has failed");
    }

    /** get item.
     * Warning, a new obj will be got! not a reference to the original one!
     */
    const obj operator [](const obj& o)const
    {
        PyObject* p = PyObject_GetItem(_p, o._p);
        if(!p){
            throw index_err("non-existing py attr");
        }
        return p;
    }
    
    /** set_item.
     */
    void set_item(const obj& key, const obj& value)const
    {
        int r = PyObject_SetItem(_p, key._p, value._p);
        if(r == -1)
            throw index_err("py set_item");
    }
    
    /** del_item.
     */
    void del_item(const obj& key)
    {
        int r = PyObject_DelItem(_p, key._p);
        if(r == -1)
            throw index_err("py del_item");
    }
    
    /** slice, [i:j].
     */
    obj sub(int i, int j = std::numeric_limits<int>::max())const
    {
        PyObject* p = PySequence_GetSlice(_p, i, j);
        if(!p)
            throw type_err("py sub");
        return p;
    }
    
    /** get iter.
     */
    inline iter begin()const;
    inline iter end()const;
    
    // introspection

    /** py object dir.
     */
    obj dir()const
    {
        PyObject* p = PyObject_Dir(_p);
        if(!p)
            throw val_err("py dir failed");
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

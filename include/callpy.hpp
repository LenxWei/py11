#include <Python.h>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <initializer_list>

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

    /** str.
     */    
    obj(const char* s):_p(PyString_FromString(s))
    {}

    /** long. please use 0L to avoid silly type confusing of compilers
     */
    obj(long i):_p(PyInt_FromLong(i))
    {}
    
    long as_long()const
    {
        long r = PyInt_AsLong(_p);
        if(PyErr_Occurred() != NULL){
            throw type_err("py as_long failed");
        }
        return r;
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
    
    /** get inner PyObject.
     */
    PyObject* p()const
    {
        return _p;
    }
        
    /** test callable.
     */
    bool is_callable()const
    {
        return (_p) && (PyCallable_Check(_p));
    }
    
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

    /** comparison.
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
    
    bool is_a(const obj& cls)const
    {
        int r = PyObject_IsInstance(_p, cls._p);
        if(r == -1)
            throw val_err("py is_a failed");
        return r;
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
    
    /** get item.
     */
    obj operator [](const obj& o)const
    {
        PyObject* p = PyObject_GetItem(_p, o._p);
        if(!p){
            throw index_err("non-existing py attr");
        }
        return p;    
    }
    
    /** get iter.
     */
    inline iter begin()const;
    inline iter end()const;
    
    /** c_str().
     */
    const char* c_str()const
    {
        const char* p = PyString_AsString(_p);
        if(!p)
            throw type_err("py c_str failed");
        
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
    
    /** py object dir.
     */
    obj dir()const
    {
        PyObject* p = PyObject_Dir(_p);
        if(!p)
            throw val_err("py dir failed");
        return p;
    }
   
    /** len.
     */
    long size()const
    {
        long r = PySequence_Size(p());
        if(r == -1)
            throw type_err("len failed");
        return r;
    }

    bool has(const obj& x)const
    {
        int r = PySequence_Contains(p(), x.p());
        if(r == -1)
            throw type_err("has failed");
        return r;
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

inline obj list_cast(const obj& o)
{
    
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

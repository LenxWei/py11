#include <Python.h>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <initializer_list>

namespace callpy {

/** init the py lib
 * @param program_name usually use argv[0]
 */
inline void py_init(char* program_name = NULL)
{
    if(program_name){
        Py_SetProgramName(program_name);
    }
    Py_Initialize();
}

/** finalize the py lib
 */
inline void py_fini()
{
    Py_Finalize();
}

class py_iter;

/** wrapper of PyObject.
 */
class pyo{
protected:
    PyObject* _p;

    void __reset()const
    {
        // only for stolen references
        const_cast<pyo&>(*this)._p = NULL;
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
        
    void release()
    {
        if(_p){
            Py_DECREF(_p);
            _p = NULL;
        }
    }
    
public:
    pyo():_p(NULL)
    {}
    
    /** copy ctor.
     */
    pyo(const pyo& o):_p(o._p)
    {
        __enter();
    }
    
    pyo(pyo& o):_p(o._p)
    {
        __enter();
    }
    
    pyo& operator=(const pyo& o)
    {
        if(o._p!=_p){
            release();
            enter(o._p);
        }
        return *this;
    }

    /** create from Py functions, need not to inc ref
     */
    pyo(PyObject* p):_p(p)
    {}
    
    pyo& operator=(PyObject* p)
    {
        if(p!=_p){
            release();
            _p = p;
        }
        return *this;
    }
    
    /** move ctor.
     */
    pyo(pyo&& o)noexcept:_p(o._p)
    {
        o._p = NULL;
    }

    /** str.
     */    
    pyo(const char* s):_p(PyString_FromString(s))
    {}

    /** int.
     */
    pyo(int i):_p(PyInt_FromLong(i))
    {}
    
    /** tuple.
     */
    pyo(std::initializer_list<pyo> l):_p(PyTuple_New(l.size()))
    {
        long i = 0;
        for(auto &x: l){
            PyTuple_SET_ITEM(_p, i++, x.p());
            x.__reset();
        }
    }
    
    /** dtor.
     */
    ~pyo()
    {
        release();
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
    bool has_attr(const pyo& o)const
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
    pyo attr(const pyo& o)const
    {
        PyObject* p = PyObject_GetAttr(_p, o._p);
        if(!p){
            throw std::out_of_range("non-existing py attr");
        }
        return p;
    }

    /** get attr.
     */
    pyo attr(const char* s)const
    {
        PyObject* p = PyObject_GetAttrString(_p, s);
        if(!p){
            throw std::out_of_range("non-existing py attr");
        }
        return p;
    }

    /** set attr.
     */
    void set_attr(const pyo& a, const pyo& v)const
    {
        int r = PyObject_SetAttr(_p, a._p, v._p);
        if(r == -1)
            throw std::invalid_argument("py set_attr failed");
    }

    /** set attr.
     */
    void set_attr(const char* a, const pyo& v)const
    {
        int r = PyObject_SetAttrString(_p, a, v._p);
        if(r == -1)
            throw std::invalid_argument("py set_attr failed");
    }

    /** del attr.
     */
    void del_attr(const pyo& a)const
    {
        int r = PyObject_DelAttr(_p, a._p);
        if(r == -1)
            throw std::invalid_argument("py del_attr failed");
    }

    /** del attr.
     */
    void del_attr(const char* a)const
    {
        int r = PyObject_DelAttrString(_p, a);
        if(r == -1)
            throw std::invalid_argument("py del_attr failed");
    }

    /** comparison.
     */
    bool operator < (const pyo& o)const
    {
        if(_p == o._p)
            return false;
        if(_p == NULL)
            return true;
        if(o._p == NULL)
            return false;
        int r = PyObject_RichCompareBool(_p, o._p, Py_LT);
        if(r == -1){
            throw std::invalid_argument("py < failed");
        }
        return r;
    }
    
    bool operator <= (const pyo& o)const
    {
        if(_p == o._p)
            return true;
        if(_p == NULL)
            return true;
        if(o._p == NULL)
            return false;
        int r = PyObject_RichCompareBool(_p, o._p, Py_LE);
        if(r == -1){
            throw std::invalid_argument("py <= failed");
        }
        return r;
    }
    
    bool operator == (const pyo& o)const
    {
        if(_p == o._p)
            return true;
        if(_p == NULL)
            return false;
        if(o._p == NULL)
            return false;
        int r = PyObject_RichCompareBool(_p, o._p, Py_EQ);
        if(r == -1){
            throw std::invalid_argument("py == failed");
        }
        return r;
    }
    
    bool operator != (const pyo& o)const
    {
        if(_p == o._p)
            return false;
        if(_p == NULL)
            return true;
        if(o._p == NULL)
            return true;
        int r = PyObject_RichCompareBool(_p, o._p, Py_NE);
        if(r == -1){
            throw std::invalid_argument("py != failed");
        }
        return r;
    }
    
    bool operator > (const pyo& o)const
    {
        if(_p == o._p)
            return false;
        if(_p == NULL)
            return false;
        if(o._p == NULL)
            return true;
        int r = PyObject_RichCompareBool(_p, o._p, Py_GT);
        if(r == -1){
            throw std::invalid_argument("py > failed");
        }
        return r;
    }
    
    bool operator >= (const pyo& o)const
    {
        if(_p == o._p)
            return true;
        if(_p == NULL)
            return false;
        if(o._p == NULL)
            return true;
        int r = PyObject_RichCompareBool(_p, o._p, Py_GE);
        if(r == -1){
            throw std::invalid_argument("py >= failed");
        }
        return r;
    }
    
    /** repr.
     */
    pyo repr()const
    {
        PyObject* p = PyObject_Repr(_p);
        if(p == NULL)
            throw std::logic_error("py repr failed");
        return p;
    }
    
    /** str.
     */
    pyo str()const
    {
        PyObject* p = PyObject_Str(_p);
        if(p == NULL)
            throw std::logic_error("py str failed");
        return p;
    }
    
    /** unicode.
     */
    pyo unicode()const
    {
        PyObject* p = PyObject_Unicode(_p);
        if(p == NULL)
            throw std::logic_error("py unicode failed");
        return p;
    }
    
    bool is_a(const pyo& cls)const
    {
        int r = PyObject_IsInstance(_p, cls._p);
        if(r == -1)
            throw std::invalid_argument("py is_a failed");
        return r;
    }
    
    template<typename ...argT>pyo operator ()(argT&& ...a)const
    {
        PyObject* r = PyObject_CallFunctionObjArgs(_p, pyo(a).p()..., NULL);
        if(r == NULL)
            throw std::invalid_argument("py operator() failed");
        return r;
    }

    pyo call(const pyo& args, const pyo& kw)const
    {
        PyObject* r = PyObject_Call(_p, args._p, kw._p);
        if(r == NULL)
            throw std::invalid_argument("py call failed");
        return r;
    }
    
    /** get item.
     */
    pyo operator [](const pyo& o)const
    {
        PyObject* p = PyObject_GetItem(_p, o._p);
        if(!p){
            throw std::out_of_range("non-existing py attr");
        }
        return p;    
    }
    
    /** get iter.
     */
    inline py_iter begin()const;
    inline py_iter end()const;
    
    /** c_str().
     */
    const char* c_str()const
    {
        const char* p = PyString_AsString(_p);
        if(!p)
            throw std::invalid_argument("py c_str failed");
        
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
    pyo dir()const
    {
        PyObject* p = PyObject_Dir(_p);
        if(!p)
            throw std::logic_error("py dir failed");
        return p;
    }
   
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
class py_iter{
private:
    pyo _it;
    pyo _v;
    bool _fin;
public:
    py_iter():_fin(true)
    {}
    
    py_iter(const pyo& o):_it(PyObject_GetIter(o.p())), _fin(false)
    {
        if(_it.is_null())
            throw std::invalid_argument("py_iter ctor failed");
        ++(*this);
    }
    
    /** operator ++.
     */
    py_iter& operator++()
    {
        _v = pyo(PyIter_Next(_it.p()));
        if(_v.is_null())
            _fin = true;
        return *this;
    }
    
    /** operator *.
     */
    pyo& operator*()
    {
        return _v;
    }
    
    bool operator==(const py_iter& i)const
    {
        return (i._it == _it) || (i._fin && _fin);
    }

    bool operator!=(const py_iter& i)const
    {
        return !(*this == i);
    }

};

/** ostream output
 */
inline std::ostream& operator <<(std::ostream& s, const pyo& o)
{
    o.output(s);
    return s;
}

/** py import.
 */
inline pyo py_import(const char* module_name)
{
    PyObject* p = PyImport_ImportModule(module_name);
    if(p == NULL)
        throw std::invalid_argument("py import () failed");
    return p;
}

// implementation

inline py_iter pyo::end()const
{
    return py_iter();
}

inline py_iter pyo::begin()const
{
    return py_iter(*this);
}

}; // ns callpy

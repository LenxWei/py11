#ifndef PY11_PY_HPP
#define PY11_PY_HPP

#include <Python.h>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <initializer_list>
#include <limits>

#ifndef PY11_ENFORCE
#define PY11_ENFORCE 1
#endif

namespace py {

#include "_err.hpp"

/* assistant classes
*******************/

class iter;

class PPyObject {
private:
    PyObject* _p;
public:
    PPyObject(PyObject* p):_p(p)
    {}
    
    operator PyObject*()const
    {
        return _p;
    }

    template<typename T> explicit operator T()const
    {
        return (T)_p;
    }
};

namespace details{
	class py_initer{
	public:
		py_initer()
		{
			Py_Initialize();
		}

		~py_initer()
		{
			Py_Finalize();
		}
	};
};

/* main class
************/

/** wrapper of PyObject.
 */
class obj{
friend class tuple;
friend class list;
friend class set;
private:
	static details::py_initer __init;

protected:

    PyObject* _p;

    void __reset()const
    {
        // only for stolen references
        const_cast<obj&>(*this)._p = NULL;
    }
    
    void enter(PyObject* p)noexcept
    {
        _p = p;
        __enter();
    }
    
    void __enter()noexcept
    {
        if(_p)
            Py_INCREF(_p);
    }

private:        
    /** disabled.
     * to prohibit getting a PyObject* from p() without correct ref counting
     */
    obj(PPyObject p);

public:
    obj()noexcept:_p(NULL)
    {}
    
    
    /** copy ctor.
     */
    obj(const obj& o)noexcept:_p(o._p)
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

    /** create from Py functions.
     * in most of cases, need not to inc ref.
     * please read python doc carefully
     */
    obj(PyObject* p, bool borrowed = false)noexcept:_p(p)
    {
        if(borrowed)
            Py_XINCREF(_p);
    }
    
    obj& operator=(PyObject* p)noexcept
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

    obj& operator=(obj&& o)noexcept
    {
        if(o._p!=_p){
            release();
            _p = o.transfer();
        }
        return *this;
    }


    /** tuple.
     */
    obj(std::initializer_list<obj> l):_p(PyTuple_New(l.size()))
    {
        long i = 0;
        for(auto &x: l){
            PyTuple_SET_ITEM(_p, i++, x._p);
            x.__reset();
        }
    }
    
    obj& operator = (std::initializer_list<obj> l)
    {
        _p = PyTuple_New(l.size());
        long i = 0;
        for(auto &x: l){
            PyTuple_SET_ITEM(_p, i++, x._p);
            x.__reset();
        }
        return *this;
    }
    
    /** dtor.
     */
    ~obj()noexcept
    {
        release();
    }

    /** release the contained object
     */
    void release()noexcept
    {
        if(_p){
            Py_DECREF(_p);
            _p = NULL;
        }
    }
    
    /** transfer the ownership of inner object.
     * @return the current PyObject*
     */
    PyObject* transfer()noexcept
    {
        PyObject* r = _p;
        if(_p){
            _p = NULL;
        }
        return r;
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
        if(r== -1 && PyErr_Occurred() != NULL){
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
        if(r== -1.0 && PyErr_Occurred() != NULL){
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
        const char* p;
        if(PyString_Check(_p)){
            p = PyString_AsString(_p);
        }
        else if(PyUnicode_Check(_p)){
            p = PyUnicode_AS_DATA(_p);
        }
        else if(PyByteArray_Check(_p)){
            p = PyByteArray_AsString(_p);
        }
        else
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
    void set_attr(const obj& a, const obj& v)
    {
        int r = PyObject_SetAttr(_p, a._p, v._p);
        if(r == -1)
            throw index_err("set_attr failed");
    }

    /** set attr.
     * @throw index_err
     */
    void set_attr(const char* a, const obj& v)
    {
        int r = PyObject_SetAttrString(_p, a, v._p);
        if(r == -1)
            throw index_err("set_attr failed");
    }

    /** del attr.
     * @throw index_err
     */
    void del_attr(const obj& a)
    {
        int r = PyObject_DelAttr(_p, a._p);
        if(r == -1)
            throw index_err("del_attr failed");
    }

    /** del attr.
     * @throw index_err
     */
    void del_attr(const char* a)
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
    obj to_str()const
    {
        PyObject* p = PyObject_Str(_p);
        if(p == NULL){
            throw val_err("str failed");
        }
        return p;
    }
    
    /** unicode.
     * @throw val_err
     */
    obj to_unicode()const
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
            return;
        }
        if(PyString_Check(_p)){
            const char* p = PyString_AsString(_p);
            if(!p)
                throw val_err("bad internal string");
            s << p;
        }
        else{
            s << to_str().c_str();
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
        PyObject* r = PyObject_CallFunctionObjArgs(_p, obj(a)._p..., NULL);
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
                int r = PySequence_Contains(_p, x._p);
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
    void set_item(const obj& key, const obj& value)
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
    PPyObject p()const
    {
        return _p;
    }
        
};


/** ostream output
 */
inline std::ostream& operator <<(std::ostream& s, const obj& o)
{
    o.output(s);
    return s;
}

/* sub types
***********/

#include "_iter.hpp"

#include "_seq.hpp"
#include "_tuple.hpp"
#include "_list.hpp"
#include "_str.hpp"

#include "_file.hpp"

#include "_num.hpp"
#include "_set.hpp"

#include "_dict.hpp"

/* implementation
****************/

#include "_sys.hpp"

inline iter obj::end()const
{
    return iter();
}

inline iter obj::begin()const
{
    return iter(*this);
}

inline list seq::to_list()const
{
    PyObject* r = PySequence_List(_p);
    if(r)
        return r;
    throw type_err("to_list failed");
}

inline tuple seq::to_tuple()const
{
    PyObject* r = PySequence_Tuple(_p);
    if(r)
        return r;
    throw type_err("to_tuple failed");
}

}; // ns py

#endif // PY11_PY_HPP

namespace py {

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


}; // ns py


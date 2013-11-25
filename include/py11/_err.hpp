/* exceptions
************/

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
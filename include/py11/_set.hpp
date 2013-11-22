/** py set.
 * @throw type_err
 */
inline obj set(const obj& o = obj() )
{
    obj s = PySet_New(o.p());
    if(s.is_null())
        throw type_err("creating set failed");
    return s;
}

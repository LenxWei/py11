#include <callpy.hpp>
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
    py::init(argv[0]);
    
    // import
    auto t = py::import("time");
    auto time = t.attr("time");

    // call/output
    auto s = time();
    cout << s.as_double() << ":" << s << endl;
    
    // consts
    cout << "True: " << py::obj(py::True) << endl;
        
    // tuple
    py::obj y = {"abc", 2, 0L, 1, 3};
    cout << y << ":" << y.size() << endl;
    
    cout << y.type() << endl;
    cout << py::obj().type() << endl;
    try{
        cout << y[1].size() << endl;
    }
    catch(const py::type_err& e){
        cout << "caught: " << e.what() << endl;
    }
    
    
    // list
    {
        py::obj l = py::list({{0, "abc"}, 22});
        cout << l << endl;
        for(auto &x: l){
            cout << x.refcnt() << endl;
        }
        l.a("sort")();
        cout << "sorted: " << l << endl;
        
        cout << "22 in l: " << l.has(22) << endl;
    }
    
    // ref count
    py::obj z = y[1];
    int refcnt = z.refcnt();
    py::obj z1 = y[1];
    
    assert( z1.refcnt() == refcnt + 1);
    y.release();
    assert( z1.refcnt() == refcnt);
    z.release();
    assert( z1.refcnt() == refcnt - 1);
        
    py::fini();
}

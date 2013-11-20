#include <callpy.hpp>
#include <iostream>

using namespace std;
using namespace callpy;

int main(int argc, char** argv)
{
    py_init(argv[0]);
    
    // import
    auto t = py_import("time");
    auto time = t.attr("ctime");

    // call/output
    auto s = time();
    cout << s << ":" << s.dir() << endl;
    
    // tuple
    pyo y = {"abc", 2, 0L, 1, 3};
    cout << y << ":" << len(y) << endl;
    
    for(auto &x: y){
        cout << x.refcnt() << endl;
    }
    
    // list
    {
        pyo l = py_list({{0L, "abc"}, 22});
        cout << l << endl;
        for(auto &x: l){
            cout << x.refcnt() << endl;
        }
        l.attr("sort")();
        cout << l << endl;
    }
    
    // ref count
    pyo z = y[1];
    
    for(auto &x: y){
        cout << x.refcnt() << endl;
    }
    y=nullptr;
    cout << z.refcnt() << endl;
    
    
    py_fini();
}

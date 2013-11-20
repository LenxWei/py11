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
    cout << s << endl;
    
    // for iter
    for(auto &x: s.dir()){
        cout << x << "\n";
    }
    
    // tuple
    pyo y = {"abc",221};
    cout << y << endl;
    
    for(auto &x: y){
        cout << x.refcnt() << endl;
    }
    
    pyo z = y[1];
    
    for(auto &x: y){
        cout << x.refcnt() << endl;
    }
    y=nullptr;
    cout << z.refcnt() << endl;
    py_fini();
}

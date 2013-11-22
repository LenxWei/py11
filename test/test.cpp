#include <py11/py.hpp>
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
    cout << "True: " << py::obj(Py_True) << endl;
        
    cout << ">> tuple" << endl;
    
    py::seq y = {"abc", 2, 0L, 1, 3};
    cout << y << ":" << y.size() << endl;
    
    cout << y.type() << endl;
    cout << py::obj().type() << endl;
    try{
        cout << y[1].size() << endl;
    }
    catch(const py::type_err& e){
        cout << "caught: " << e.what() << endl;
    }
    
    {
        cout << ">> dict" << endl;
        py::obj z = py::dict();
        
        z.set_item(1, "abc");
        z.set_item("def", 2);
        cout << z << endl;
        cout << "items: " << z.a("items")() << endl;
    }

    {
        cout << ">> set" << endl;
        cout << "y: " << y << endl;
        
        py::set s({});
        cout << "s= " << s << endl;
        s.extend(y);
        cout << "s= " << s << endl;

        py::set s1 = {1 ,4, 0};
        cout << "s1 = " << s1 << endl;

        cout << "s | s1 = " << (s | s1) << endl;

        s -= s1;
        cout << "s -= s1 : " << s << endl;
    }
    
    {
        cout << ">> list" << endl;
        py::list l({{0, "abc"}, 22});
        cout << l << endl;
        for(auto &x: l){
            cout << x.refcnt() << endl;
        }
        l.sort();
        cout << "sorted: " << l << endl;
        l.append(33);
        l.insert(1,44);
        cout << "modified: " << l << endl;
        l.reverse();
        cout << "reverse: " << l << endl;
        cout << "[2:4]: " <<l.sub(2,4) << endl;
        cout << "22 in l: " << l.has(22) << endl;
        cout << "l * 2: " << l*2 << endl;
        l *=3;
        cout << "l*=3: " << l << endl;
    }
    
    {
        cout << ">> ref count tests" << endl;
        py::obj z = y[1];
        int refcnt = z.refcnt();
        py::obj z1 = y[1];
    
        assert( z1.refcnt() == refcnt + 1);
        y.release();
        assert( z1.refcnt() == refcnt);
        z.release();
        assert( z1.refcnt() == refcnt - 1);
    }
    py::fini();
}

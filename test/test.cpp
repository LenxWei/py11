#include <py11/py.hpp>
#include <iostream>

using namespace std;

//py::list a;

int main(int argc, char** argv)
{
    //py::set_prog(argv[0]);

//	cout << a << endl;

    // import
    auto t = py::import("time");
    auto time = t.attr("time");

    // call/output
    auto s = time();
    cout << s.as_double() << ":" << s << endl;
    
    // consts
    cout << "True: " << py::obj(Py_True) << endl;
    cout << "False: " << py::obj(Py_False) << endl;
    cout << "None: " << py::obj(Py_None) << endl;
        
    cout << ">> tuple" << endl;
    
    py::tuple y = {"abc", 2, 0L, 1, 3};
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
        py::dict z({});
        
        z.set_item(1, "abc");
        z.set_item("def", 2);
        cout << z << endl;
        cout << "items: " << z.a("items")() << endl;
        cout << "items == a(\"items\"): " << (z.a("items")() == z.items()) << endl;       
        cout << "keys == a(\"keys\"): " << (z.a("keys")() == z.keys()) << endl;       
        cout << "values == a(\"values\"): " << (z.a("values")() == z.values()) << endl;       
        py::dict z1({{1,"abc"}, {"def", 2}});
        cout << z1 << endl;
        py::dict z2;
        z2={{3,"a"}, {4,"b"}};
        cout << z2 << endl;
        z1.update(z2);
        cout << "update: " << z1 << endl;
        for(auto& x : z1){
            cout << x << endl;
        }
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
        cout << ">> str" << endl;
        py::str a="adfaf";
        cout << a << endl;
        cout << a.a("replace")("a","_") << endl;
        for(auto& x: a){
            cout << x << ", ";
        }
        cout << endl;
        cout << a[1] << " " << a.sub(1,4) << endl;
        a = "%d";
        cout << (a % py::obj({1}) ) << endl;
    }
    {
        cout << ">> file" << endl;
        py::file f("test/test.cpp","rb");
        int cnt = 0;
        while(1){
            py::str line = f.readline();
            cnt ++;
            if(line.size() == 0)
                break;
        }
        cout << "line count: " << cnt << endl;
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
}

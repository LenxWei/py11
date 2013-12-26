#include <py11/py.hpp>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    py::obj module, func, value;
    py::list args;

    try {
        if (argc < 3) {
            cerr << "Usage: call pythonfile funcname [args]\n";
            return 1;
        }

        // for setting sys.path
        py::set_arg(argc, argv);

        module = py::import(argv[1]); // throw exception if failed

        func = module.attr(argv[2]); // throw exception if not found

        args = {};
        for (int i = 0; i < argc - 3; ++i) {
            args.append(atoi(argv[i + 3]));
        }

        value = func.call(args.to_tuple()); // throw exception if failed

        printf("Result of call: %ld\n", value.as_long());
        return 0;
    }
    catch (const py::err& e) {
        cerr << e.what() << endl;
        py::print_err();
        return 1;
    }
}

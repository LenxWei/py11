#include <py11/py.hpp>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
	py::obj name, module, func, value;
	py::list args;
    int i;

    try{
		if (argc < 3) {
			cerr << "Usage: call pythonfile funcname [args]\n";
			return 1;
		}

		// for setting sys.path
		py::set_arg(argc, argv);

		name = argv[1];

		module = py::import(name); // throw exception if failed

		func = module.attr(argv[2]);
		if (!func.is_callable())
			throw py::index_err(py::str("Cannot find callable function '%s'", argv[2]).c_str());

		args = {};
		for (i = 0; i < argc - 3; ++i) {
			value = atoi(argv[i + 3]);
			if (!value) {
				cerr << "Cannot convert argument\n";
				return 1;
			}
			args.append(value);
		}

		value = func.call(args.to_tuple()); // throw exception if failed

		printf("Result of call: %ld\n", value.as_long());
		return 0;
    }
    catch(py::err& e){
    	cerr << e.what() << endl;
    	py::print_err();
    	return 1;
    }
}

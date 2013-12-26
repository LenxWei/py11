/** @mainpage Py11
*
* @authors <a href="http://lenx.100871.net">Lenx Tao Wei</a> (lenx.wei at gmail.com)
*
* @section intro Introduction
* Py11 is a minimalist Python wrapper for C++11, and makes it easier to call Python libraries from C++11.
*
* The git repository is at https://github.com/LenxWei/py11.
*
* @section install Install
*
* Py11 is a pure header library. To install it, just copy the directory include/py11 to system include directory.
*
* @section compile Compile
*
* Need a compiler supporting C++11, such as gcc 4.7, clang 3.2 or higher.<br>
* Make sure to include correct Python header files and link correct Python libraries.
* For example:<br>
* <pre>
* export CFLAGS="-I/usr/include/python2.7 -std=c++11"
* export LDFLAGS=-lpython2.7
* </pre>
*
* @section doc Documents
* You can find documents about wrapper classes <a href="hierarchy.html">here</a>,
* and Python utils <a href="modules.html">here</a>.
*
* @section ex1 Examples
* Here is the py11 implementation of <a href="http://docs.python.org/2/extending/embedding.html#pure-embedding">the original call example</a>.
* \include call_py11.cpp
*
* <hr>
* @todo add tutorial
* @todo add exception handling
*
* @defgroup utils Python utils
* Utility functoins.
*
*/

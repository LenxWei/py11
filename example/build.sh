export CXX=g++-4.9
export CFLAGS="-I/usr/include/python2.7 -O3 -g -std=c++11"
export PY11FLAGS="-I../include"
export LDFLAGS=-lpython2.7

$CXX -o call_orig call_orig.cpp $CFLAGS $LDFLAGS
$CXX -o call_py11 call_py11.cpp $CFLAGS $LDFLAGS $PY11FLAGS

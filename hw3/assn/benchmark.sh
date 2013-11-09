echo "amptjp-bal.rep"
./mdriver -v -f ../traces/amptjp-bal.rep |grep 'Perf index'
echo "binary2-bal.rep"
./mdriver -v -f ../traces/binary2-bal.rep |grep 'Perf index'
echo "binary-bal.rep"
./mdriver -v -f ../traces/binary-bal.rep |grep 'Perf index'
echo "cccp-bal.rep"
./mdriver -v -f ../traces/cccp-bal.rep |grep 'Perf index'
echo "coalescing-bal.rep"
./mdriver -v -f ../traces/coalescing-bal.rep |grep 'Perf index'
echo "cp-decl-bal.rep"
./mdriver -v -f ../traces/cp-decl-bal.rep |grep 'Perf index'
echo "expr-bal.rep"
./mdriver -v -f ../traces/expr-bal.rep |grep 'Perf index'
echo "random2-bal.rep"
./mdriver -v -f ../traces/random2-bal.rep |grep 'Perf index'
echo "random-bal.rep"
./mdriver -v -f ../traces/random-bal.rep |grep 'Perf index'
echo "realloc2-bal.rep"
./mdriver -v -f ../traces/realloc2-bal.rep |grep 'Perf index'
echo "realloc-bal.rep"
./mdriver -v -f ../traces/realloc-bal.rep |grep 'Perf index'

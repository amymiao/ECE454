echo "Homework 4: Tester"

echo "Clean Build commencing"
make clean; make

#compute randtrack with 1 thread, 50 results
/usr/bin/time ./randtrack 1 50 > rt1.out; sort -n rt1.out > rt1.outs;
rm rt1.out

#compute randtrack_global_lock with varying threads, 50 results
/usr/bin/time ./randtrack_global_lock 1 50 > rtgl1.out; sort -n rtgl1.out > rtgl1.outs;
echo ""
rm rtgl1.out

/usr/bin/time ./randtrack_global_lock 2 50 > rtgl2.out; sort -n rtgl2.out > rtgl2.outs;
echo ""
rm rtgl2.out

/usr/bin/time ./randtrack_global_lock 4 50 > rtgl4.out; sort -n rtgl4.out > rtgl4.outs;
echo ""
rm rtgl4.out

#compute randtrack_tm with varying threads, 50 results
/usr/bin/time ./randtrack_tm 1 50 > rttm1.out; sort -n rttm1.out > rttm1.outs;
echo ""
rm rttm1.out

/usr/bin/time ./randtrack_tm 2 50 > rttm2.out; sort -n rttm2.out > rttm2.outs;
echo ""
rm rttm2.out

/usr/bin/time ./randtrack_tm 4 50 > rttm4.out; sort -n rttm4.out > rttm4.outs;
echo ""
rm rttm4.out

#compute randtrack_list_lock with varying threads, 50 results
/usr/bin/time ./randtrack_list_lock 1 50 > rtll1.out; sort -n rtll1.out > rtll1.outs;
echo ""
rm rtll1.out

/usr/bin/time ./randtrack_list_lock 2 50 > rtll2.out; sort -n rtll2.out > rtll2.outs;
echo ""
rm rtll2.out

/usr/bin/time ./randtrack_list_lock 4 50 > rtll4.out; sort -n rtll4.out > rtll4.outs;
echo ""
rm rtll4.out

#compute randtrack_element_lock with varying threads, 50 results
/usr/bin/time ./randtrack_element_lock 1 50 > rtel1.out; sort -n rtel1.out > rtel1.outs;
echo ""
rm rtel1.out

/usr/bin/time ./randtrack_element_lock 2 50 > rtel2.out; sort -n rtel2.out > rtel2.outs;
echo ""
rm rtel2.out

/usr/bin/time ./randtrack_element_lock 4 50 > rtel4.out; sort -n rtel4.out > rtel4.outs;
echo ""
rm rtel4.out

#compute randtrack_reduction with varying threads, 50 results
/usr/bin/time ./randtrack_reduction 1 50 > rtr1.out; sort -n rtr1.out > rtr1.outs;
echo ""
rm rtr1.out

/usr/bin/time ./randtrack_reduction 2 50 > rtr2.out; sort -n rtr2.out > rtr2.outs;
echo ""
rm rtr2.out

/usr/bin/time ./randtrack_reduction 4 50 > rtr4.out; sort -n rtr4.out > rtr4.outs;
echo ""
rm rtr4.out

sleep 1

echo "Enumerating all differences with respect to the original randtrack"

echo "Global lock enumeration:"
diff rt1.outs rtgl1.outs
diff rt1.outs rtgl2.outs
diff rt1.outs rtgl4.outs

echo "Transactional memory enumeration:"
diff rt1.outs rttm1.outs
diff rt1.outs rttm2.outs
diff rt1.outs rttm4.outs

echo "List lock enumeration:"
diff rt1.outs rtll1.outs
diff rt1.outs rtll2.outs
diff rt1.outs rtll4.outs

echo "Element lock enumeration:"
diff rt1.outs rtel1.outs
diff rt1.outs rtel2.outs
diff rt1.outs rtel4.outs

echo "Reduction enumeration:"
diff rt1.outs rtr1.outs
diff rt1.outs rtr2.outs
diff rt1.outs rtr4.outs

echo "Script complete"

#cleanup
rm rt*.outs

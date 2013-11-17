
#Message
echo "Generating randtrack results for thread variations: 1,2,4"

#compute randtrack with 1 thread, 50 results
./randtrack 1 50 > rt1.out; sort -n rt1.out > rt1.outs;
rm rt1.out

#compute randtrack with 2 threads, 50 results
./randtrack 2 50 > rt2.out; sort -n rt2.out > rt2.outs;
rm rt2.out

#compute randtrack with 4 threads, 50 results
./randtrack 4 50 > rt4.out; sort -n rt4.out > rt4.outs;
rm rt4.out

echo "Diffing rt*.outs files"

echo "Diffing rt1 vs rt2"
diff rt1.outs rt2.outs
sleep 1

echo "Diffing rt1 vs rt4"
diff rt1.outs rt4.outs
sleep 1


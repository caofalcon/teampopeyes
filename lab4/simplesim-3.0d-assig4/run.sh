make clean
make sim-cache >/dev/null

#./sim-cache -config cache-config/cache-lru-nextline.cfg /cad2/ece552f/benchmarks/compress.eio
#./sim-cache -config cache-config/cache-lru-nextline.cfg /cad2/ece552f/benchmarks/gcc.eio
#./sim-cache -config cache-config/cache-lru-nextline.cfg /cad2/ece552f/benchmarks/go.eio

# ./sim-cache -config cache-config/cache-lru-stride.cfg /cad2/ece552f/benchmarks/compress.eio
# ./sim-cache -config cache-config/cache-lru-stride.cfg /cad2/ece552f/benchmarks/gcc.eio
# ./sim-cache -config cache-config/cache-lru-stride.cfg /cad2/ece552f/benchmarks/go.eio

 ./sim-cache -config cache-config/cache-lru-open.cfg /cad2/ece552f/benchmarks/compress.eio
 ./sim-cache -config cache-config/cache-lru-open.cfg /cad2/ece552f/benchmarks/gcc.eio
 ./sim-cache -config cache-config/cache-lru-open.cfg /cad2/ece552f/benchmarks/go.eio

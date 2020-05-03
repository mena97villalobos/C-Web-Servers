cd serverS
make clean
make
cd ..

cd cliente
make clean
make
cd ..


cd detener
make clean
make
cd ..


THREADS=5
CICLOS=5



cd serverS/build
./server-secuencial 8080 &
cd ../..

sleep 1

cd cliente/build
./cliente 127.0.0.1 8080 video.mp4 $THREADS $CICLOS
cd ../..

cd detener/build
./detener 127.0.0.1 8080 
cd ../..





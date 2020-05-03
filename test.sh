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



cd serverS/build
./server-secuencial 8080 &
cd ../..

sleep 1

cd cliente/build
./cliente 127.0.0.1 8080 video.mp4 1 1
cd ../..

cd detener/build
./detener 127.0.0.1 8080 
cd ../..





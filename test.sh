cd serverS
make
cd build
./server-secuencial 8080 &
cd ../..
cd cliente
make 
cd build
./cliente 127.0.0.1 8080 video.mp4 10 10
cd ../..
cd detener
make
cd build
./detener 127.0.0.1 8080 
cd ../..





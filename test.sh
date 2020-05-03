cd serverS
make clean
make
cd ..

cd serverF
make clean
make
cd ..


cd serverT
make clean
make
cd ..



cd serverPT
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


THREADS=10
CICLOS=10



cd serverS/build
./server-secuencial 8080 &
cd ../..

sleep 1

cd cliente/build
./cliente 127.0.0.1 8080 video.mp4 $THREADS $CICLOS
cd ../..
echo

cd detener/build
./detener 127.0.0.1 8080 
cd ../..





cd serverF/build
./serverForks 8081 &
cd ../..

sleep 1

cd cliente/build
./cliente 127.0.0.1 8081 video.mp4 $THREADS $CICLOS
cd ../..
echo

cd detener/build
./detener 127.0.0.1 8081
cd ../..





cd serverT/build
./serverT 8082 &
cd ../..

sleep 1

cd cliente/build
./cliente 127.0.0.1 8082 video.mp4 $THREADS $CICLOS
cd ../..
echo

cd detener/build
./detener 127.0.0.1 8082
cd ../..



cd serverPT/build
./server-predefine-threads 8083 10&
cd ../..

sleep 1

cd cliente/build
./cliente 127.0.0.1 8083 video.mp4 $THREADS $CICLOS
cd ../..
echo

cd detener/build
./detener 127.0.0.1 8083 
cd ../..




sleep 3

echo "Checking server alive"

ps aux|grep server-secuencial
ps aux|grep serverForks
ps aux|grep serverT
ps aux|grep server-predefine-threads




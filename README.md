Instituto Tecnológico de Costa Rica\
Maestría en Computación\
Sistemas Operativos Avanzados\
Segundo Semestre - 2020

Profesor: PhD Francisco Torres

## Proyecto 2
Estudiantes

Ignacio Murillo - 2016082016\
Bryan Mena Villalobos - 2016112933\
Jordan Alexander Gomez Tapasco - 2016074382\
Luis Carlos Lara Lopez - 200315796\
Luis José Castillo Valverde - 2016094804

Contenidos del proyecto
=======================

```
.
├── clientSRC                     # Carpeta con archivos fuente del cliente
│   ├── client.c                  # C'odigo fuente del cliente
│   └── makefile                  # makefile para el cliente
│
├── commonSrc                     # Carpeta con los archivos fuente comunes para todos los proyectos
│   ├── argValidator.c            # C'odigo fuente para validar argumentos en los sevidores y el cliente
│   ├── commonHttpFunctions.c     # C'odigo fuente para manejar peticiones y respuestas mediante el protocolo HTTP
│   ├── mime.c                    # C'odigo fuente para procesar el tipo de dato de los archivos que se envian
│   └── net.c                     # C'odigo fuente con funciones de networking aqu'i se inicia el socket para el servidor
│
├── headers                       # Carpeta con los headers en com'un para los servidores
│   ├── argValidator.h
│   ├── commonHttpFunctions.h
│   ├── mime.h
│   └── net.h
│
├── serverF                       # Carpeta con archivos fuente para el servidor versi'on forked
│   ├── main.c                    # C'odigo fuente del servidor web versi'on forked
│   └── makefile                  # makefile para el servidor versi'on forked
│
├── serverPF                      # Carpeta con archivos fuente para el servidor versi'on pre-forked
│   ├── main.c                    # C'odigo fuente del servidor web versi'on pre-forked
│   └── makefile                  # makefile para el servidor versi'on pre-forked
│
├── serverPT                      # Carpeta con archivos fuente para el servidor versi'on pre-threaded
│   ├── main.c                    # C'odigo fuente del servidor web versi'on pre-threaded
│   └── makefile                  # makefile para el servidor versi'on pre-threaded
│
├── serverroot                    # Carpeta con los contenidos disponibles en los servidores, es una carpeta en com'un para todos los servidores
│   ├── a.txt
│   ├── b.mp4
│   └── video.mp4
│
├── serverS                       # Carpeta con archivos fuente para el servidor versi'on secuencial
│   ├── main.c                    # C'odigo fuente del servidor web versi'on secuencial
│   └── makefile                  # makefile para el servidor versi'on secuencial
│
└── serverT                       # Carpeta con archivos fuente para el servidor versi'on threaded
    ├── main.c                    # C'odigo fuente del servidor web versi'on threaded
    └── makefile                  # makefile para el servidor versi'on threaded
```

Cliente
=======

Compilar el Cliente
-------------------

1. Dirigirse a la capeta ./clientSRC
2. Utilizar el comando make para compilar el cliente utilizando el make file incluido
3. Los objetos resultantes se encuentran en la carpeta ./clientSRC/build

Utilizar el cliente
-------------------

El cliente tiene como agumentos la m'aquina donde se est'a ejecutando el servidor, el puerto del servidor, 
el archivo que se solicita, la cantidad de hilos que el cliente utilizar'a y la cantidad de ciclos que el cliente har'a

Ejemplo de uso del cliente
```
./client 127.0.0.1 8080 video.mp4 5 1
```

Versi'on Secuencial
==================

La versi'on secuencial del servidor recibe como par'ametro el puerto en el que correr'a el servidor, la implementaci'on
es est'andar, es un 'unico proceso en el cual acepta peticiones mediante el protocolo HTTP y responde con los archivos
solicitados igualmente con una respuesta HTTP

Compilar el Servidor Secuencial
-------------------------------

1. Dirigirse a la capeta ./serverS
2. Utilizar el comando make para compilar el cliente utilizando el make file incluido
3. Los objetos resultantes se encuentran en la carpeta ./serverS/build

Utilizar el Servidor Secuencial
-------------------------------

Esta versi'on tiene como par'ametro 'unicamente el puerto en el que el servidor correr'a
Ejemplo de uso
```
./server-secuencial 8080
``` 

Versi'on Threaded
==================

La versi'on threaded del servidor recibe como par'ametro el puerto en el que correr'a el servidor, la implementaci'on
utiliza el proceso principal para escuchar peticiones al servidor, cada petici'on que llega crear'a un nuevo thread para 
atender dicha solicitud, una vez atendida la solicitud el thread muere.

Compilar el Servidor Threaded
-------------------------------

1. Dirigirse a la capeta ./serverT
2. Utilizar el comando make para compilar el cliente utilizando el make file incluido
3. Los objetos resultantes se encuentran en la carpeta ./serverT/build

Utilizar el Servidor Threaded
-------------------------------

Esta versi'on tiene como par'ametro 'unicamente el puerto en el que el servidor correr'a
Ejemplo de uso
```
./serverT 8080
``` 

Versi'on Forked
==================

La versi'on forked del servidor recibe como par'ametro el puerto en el que correr'a el servidor, la implementaci'on
utiliza el proceso principal para escuchar peticiones al servidor, cada petici'on que llega crear'a un nuevo fork para 
atender dicha solicitud, una vez atendida la solicitud el fork muere.

Compilar el Servidor Forked
-------------------------------

1. Dirigirse a la capeta ./serverT
2. Utilizar el comando make para compilar el cliente utilizando el make file incluido
3. Los objetos resultantes se encuentran en la carpeta ./serverT/build

Utilizar el Servidor Forked
-------------------------------

Esta versi'on tiene como par'ametro 'unicamente el puerto en el que el servidor correr'a
Ejemplo de uso
```
./serverForks 8080
``` 

Versi'on Pre-Threaded
==================

La versi'on pre-threaded del servidor recibe como par'ametro el puerto en el que correr'a el servidor y la cantidad de
hilos que se crear'an para manejar las solicitudes, la implementaci'on inicia creando la cantidad de threads solicitada
posteriormente se utiliza el proceso principal para escuchar peticiones al servidor, cada petici'on que llega al 
servidor se asigna a uno de los threads pre creados, estos threads manejan la petici'on y responden al cliente pero no
mueren, sino que permanecen a la espera de nuevas solicitudes. Para sincronizar los procesos se utiliza la librer'ia
Pthread, m'as espec'ificamente los ```pthread_mutex_t``` y los ```pthread_cond_t``` para tener a los threads creados
esperando por la sennal del proceso principal indicando al thread que maneje la petici'on

Compilar el Servidor Pre-Threaded
-------------------------------

1. Dirigirse a la capeta ./serverPT
2. Utilizar el comando make para compilar el cliente utilizando el make file incluido
3. Los objetos resultantes se encuentran en la carpeta ./serverPT/build

Utilizar el Servidor Pre-Threaded
-------------------------------

Esta versi'on tiene como par'ametro el puerto en el que el servidor correr'a y la cantidad de hilos que se crear'an
Ejemplo de uso
```
./server-predefine-threads 8080 5
``` 

Versi'on Pre-Forked
==================

La versi'on pre-Forked del servidor recibe como par'ametro el puerto en el que correr'a el servidor y la cantidad de
forks que se crear'an para manejar las solicitudes, la implementaci'on inicia creando la cantidad de forks solicitada
posteriormente se utiliza el proceso principal para escuchar peticiones al servidor, cada petici'on que llega al 
servidor se asigna a uno de los forks pre creados, estos forks manejan la petici'on y responden al cliente pero no
mueren, sino que permanecen a la espera de nuevas solicitudes. Para sincronizar los procesos se utiliza la librer'ia
Pthread, m'as espec'ificamente los ```pthread_mutex_t``` y los ```pthread_cond_t``` para tener a los forks creados
esperando por la sennal del proceso principal indicando al thread que maneje la petici'on, se hizo neceario el uso de
memoria compartida para los sem'aforos antes mencionados adem'as de la configuraci'on de los mismos para permitir que
estos funcionacen entre procesos.

Compilar el Servidor Pre-Forked
-------------------------------

1. Dirigirse a la capeta ./serverPF
2. Utilizar el comando make para compilar el cliente utilizando el make file incluido
3. Los objetos resultantes se encuentran en la carpeta ./serverPF/build

Utilizar el Servidor Pre-Forked
-------------------------------

Esta versi'on tiene como par'ametro el puerto en el que el servidor correr'a y la cantidad de forks que se crear'an
Ejemplo de uso
```
./serverPredefineForks 8080 5
``` 

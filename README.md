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
├── cliente                     # Carpeta con archivos fuente del cliente
│   ├── cliente.c                  # Código fuente del cliente
│   └── makefile                  # makefile para el cliente
│
├── commonSrc                     # Carpeta con los archivos fuente comunes para todos los proyectos
│   ├── argValidator.c            # Código fuente para validar argumentos en los sevidores y el cliente
│   ├── commonHttpFunctions.c     # Código fuente para manejar peticiones y respuestas mediante el protocolo HTTP
│   ├── mime.c                    # Código fuente para procesar el tipo de dato de los archivos que se envian
│   └── net.c                     # Código fuente con funciones de networking aquí se inicia el socket para el servidor
│
├── headers                       # Carpeta con los headers en común para los servidores
│   ├── argValidator.h
│   ├── commonHttpFunctions.h
│   ├── mime.h
│   └── net.h
│
├── serverF                       # Carpeta con archivos fuente para el servidor versión forked
│   ├── main.c                    # Código fuente del servidor web versión forked
│   └── makefile                  # makefile para el servidor versión forked
│
├── serverPF                      # Carpeta con archivos fuente para el servidor versión pre-forked
│   ├── main.c                    # Código fuente del servidor web versión pre-forked
│   └── makefile                  # makefile para el servidor versión pre-forked
│
├── serverPT                      # Carpeta con archivos fuente para el servidor versión pre-threaded
│   ├── main.c                    # Código fuente del servidor web versión pre-threaded
│   └── makefile                  # makefile para el servidor versión pre-threaded
│
├── serverroot                    # Carpeta con los contenidos disponibles en los servidores, es una carpeta en común para todos los servidores
│   ├── a.txt
│   ├── b.mp4
│   └── video.mp4
│
├── serverS                       # Carpeta con archivos fuente para el servidor versión secuencial
│   ├── main.c                    # Código fuente del servidor web versión secuencial
│   └── makefile                  # makefile para el servidor versión secuencial
│
└── serverT                       # Carpeta con archivos fuente para el servidor versión threaded
    ├── main.c                    # Código fuente del servidor web versión threaded
    └── makefile                  # makefile para el servidor versión threaded
```

Cliente
=======

Compilar el Cliente
-------------------

1. Dirigirse a la capeta ./client
2. Utilizar el comando make para compilar el cliente utilizando el make file incluido
3. Los objetos resultantes se encuentran en la carpeta ./client/build

Utilizar el cliente
-------------------

El cliente tiene como agumentos la máquina donde se está ejecutando el servidor, el puerto del servidor, 
el archivo que se solicita, la cantidad de hilos que el cliente utilizará y la cantidad de ciclos que el cliente hará

Ejemplo de uso del cliente
```
./cliente 127.0.0.1 8080 video.mp4 5 1
```

Opcionalmente el cliente tiene el argumento ```v```, este argumento muestra los request realizados (estilo verbose) 
por el cliente

```
./cliente 127.0.0.1 8080 video.mp4 5 1 v
```

Versión Secuencial
==================

La versión secuencial del servidor recibe como parámetro el puerto en el que correrá el servidor, la implementación
es estándar, es un único proceso en el cual acepta peticiones mediante el protocolo HTTP y responde con los archivos
solicitados igualmente con una respuesta HTTP

Compilar el Servidor Secuencial
-------------------------------

1. Dirigirse a la capeta ./serverS
2. Utilizar el comando make para compilar el cliente utilizando el make file incluido
3. Los objetos resultantes se encuentran en la carpeta ./serverS/build

Utilizar el Servidor Secuencial
-------------------------------

Esta versión tiene como parámetro únicamente el puerto en el que el servidor correrá
Ejemplo de uso
```
./server-secuencial 8080
``` 

Versión Threaded
==================

La versión threaded del servidor recibe como parámetro el puerto en el que correrá el servidor, la implementación
utiliza el proceso principal para escuchar peticiones al servidor, cada petición que llega creará un nuevo thread para 
atender dicha solicitud, una vez atendida la solicitud el thread muere.

Compilar el Servidor Threaded
-------------------------------

1. Dirigirse a la capeta ./serverT
2. Utilizar el comando make para compilar el cliente utilizando el make file incluido
3. Los objetos resultantes se encuentran en la carpeta ./serverT/build

Utilizar el Servidor Threaded
-------------------------------

Esta versión tiene como parámetro únicamente el puerto en el que el servidor correrá
Ejemplo de uso
```
./serverT 8080
``` 

Versión Forked
==================

La versión forked del servidor recibe como parámetro el puerto en el que correrá el servidor, la implementación
utiliza el proceso principal para escuchar peticiones al servidor, cada petición que llega creará un nuevo fork para 
atender dicha solicitud, una vez atendida la solicitud el fork muere.

Compilar el Servidor Forked
-------------------------------

1. Dirigirse a la capeta ./serverT
2. Utilizar el comando make para compilar el cliente utilizando el make file incluido
3. Los objetos resultantes se encuentran en la carpeta ./serverT/build

Utilizar el Servidor Forked
-------------------------------

Esta versión tiene como parámetro únicamente el puerto en el que el servidor correrá
Ejemplo de uso
```
./serverForks 8080
``` 

Versión Pre-Threaded
==================

La versión pre-threaded del servidor recibe como parámetro el puerto en el que correrá el servidor y la cantidad de
hilos que se crearán para manejar las solicitudes, la implementación inicia creando la cantidad de threads solicitada
posteriormente se utiliza el proceso principal para escuchar peticiones al servidor, cada petición que llega al 
servidor se asigna a uno de los threads pre creados, estos threads manejan la petición y responden al cliente pero no
mueren, sino que permanecen a la espera de nuevas solicitudes. Para sincronizar los procesos se utiliza la librería
Pthread, más específicamente los ```pthread_mutex_t```para tener a los threads creados
esperando por la señal del proceso principal indicando al thread que maneje la petición

Se utiliza un mecanismo elegante para detener los hilos, cuando un hilo encuentra una llamada de terminacion 
poner una bandera global de terminacion en true y apaga el server socket.

El hilo principal se da cuenta y despierta a los hilos que al ver la bandera global de terminacion, tambien terminan

El hilo principal espera que todos los hilos terminen antes de terminar el mismo.


Compilar el Servidor Pre-Threaded
-------------------------------

1. Dirigirse a la capeta ./serverPT
2. Utilizar el comando make para compilar el cliente utilizando el make file incluido
3. Los objetos resultantes se encuentran en la carpeta ./serverPT/build

Utilizar el Servidor Pre-Threaded
-------------------------------

Esta versión tiene como parámetro el puerto en el que el servidor correrá y la cantidad de hilos que se crearán
Ejemplo de uso
```
./server-predefine-threads 8080 5
``` 

Versión Pre-Forked
==================

La versión pre-Forked del servidor recibe como parámetro el puerto en el que correrá el servidor y la cantidad de
forks que se crearán para manejar las solicitudes, la implementación inicia creando la cantidad de forks solicitada
posteriormente se utiliza el proceso principal para escuchar peticiones al servidor, cada petición que llega al 
servidor se asigna a uno de los forks pre creados, estos forks manejan la petición y responden al cliente pero no
mueren, sino que permanecen a la espera de nuevas solicitudes. Para sincronizar los procesos se utiliza la librería
Pthread, más específicamente los ```pthread_mutex_t``` y los ```pthread_cond_t``` para tener a los forks creados
esperando por la señal del proceso principal indicando al thread que maneje la petición, se hizo neceario el uso de
memoria compartida para los semáforos antes mencionados además de la configuración de los mismos para permitir que
estos funcionacen entre procesos.

Compilar el Servidor Pre-Forked
-------------------------------

1. Dirigirse a la capeta ./serverPF
2. Utilizar el comando make para compilar el cliente utilizando el make file incluido
3. Los objetos resultantes se encuentran en la carpeta ./serverPF/build

Utilizar el Servidor Pre-Forked
-------------------------------

Esta versión tiene como parámetro el puerto en el que el servidor correrá y la cantidad de forks que se crearán
Ejemplo de uso
```
./serverPredefineForks 8080 5
``` 

Detener el servidor
=======
Para detener el servidor ser hace una llamada como un cliente pero con una sintasix especial y un clave.
http://localhost:8080/DETENER?PK=12345
Hay un cliente especializado para denener el servido


Compilar el cliente especializados para detener el servidor
-------------------

1. Dirigirse a la capeta ./detener
2. Utilizar el comando make para compilar el cliente utilizando el make file incluido
3. Los objetos resultantes se encuentran en la carpeta ./detener/build

Utilizar el cliente especializados para detener el servidor
-------------------

El cliente  especializados para detener el servidor tiene como agumentos la máquina donde se está ejecutando el servidor
y el puerto del servidor, 

Ejemplo de uso del cliente especializados para detener el servidor
```
./detener 127.0.0.1 8080
```
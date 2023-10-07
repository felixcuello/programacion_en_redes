# Programación en Redes - Trabajo Práctico 2023

## Introduccion

```
    TCP Server                                         TCP Client

1   socket()

2   bind()

3   listen()

4   accept()

5                                                      socket()

6            <---------------------------------------  connect()

7            <---------------------------------------  write()

8   read()

9   write()  --------------------------------------->  read()

10  close()  --------------------------------------->  read()

11                                                     close()

```

## Iteracion 0
Se desarrollara la iteracion 1, sin multithread.


## Iteracion 1

Se desarrollará la primera versión del webserver. Este proceso será un programa que se levantará
por consola con la posibilidad de recibir ciertos parámetros como el puerto de escucha. El servidor
será unproceso multithread, por lo tanto cada request recibido deberá atenderse en un nuevo thread.
El mismodeberá finalizar al terminar de atender el pedido, liberando todos los recursos asociados.
Se utilizará elsoporte de la librería pthreads. Los threads serán creados en modo detached (ver
man pthread_create). Enparalelo, también se desarrollará un pequeño proceso cliente cuya única
función será la de establecer unaconexión TCP al webserver. Enviará un mensaje con el string
“PING” y el server responderá con el string “PONG”. Al recibir la respuesta la imprimirá por
consola y finalizará adecuadamente.


## Iteracion 2

Para esta iteración se modificará el webserver desarrollado en la iteración anterior y se lo
adaptará pararealizar non-blocking I/O sobre un single-thread. Utilizará la función select()
ó poll() para atendertodas las conexiones TCP entrantes. El resto de la funcionalidad quedará
igual que antes.


## Iteracion 3

El proceso webserver levantará un thread el cual creará un socket UDP cuyo único propósito será
el deresponder al mensaje de heartbeat. Al mismo tiempo se programará un pequeño proceso que,
periódicamente y de forma indefinida, envíe el mensaje al server para indicar la normal operación
del mismo. Si durante cierto período de tiempo o tras varios intentos no se recibió respuesta,
entonces sedeberá indicar por consola sobre dicho evento.


## Iteracion 4

El webserver implementará una versión muy reducida del protocolo http. El cliente programado
paratestear el servidor será dado de baja y a partir de ahora los clientes serán los browsers
del mercado. Elúnico endpoint que expone el servidor es
HTTP GET http://direcciónIp:puerto/imagen.jpg.Un request a dicha url deberá traer el
archivo de imagen completo.


## Iteracion 5

Como último paso, la implementación multithread del webserver deberá incluir un pool de threads
cuyo tamaño de worker threads será variable por línea de comando. Cuando se acepte una nueva
conexión, sedeberá seleccionar el primer thread del pool para atender la tarea. Al finalizar el
thread deberá volver alpool sin finalizar. En el caso de no haber thread disponible, la tarea
deberá quedar en espera hasta quehaya uno disponible nuevamente. La opción de timeout queda
a criterio del alumno.


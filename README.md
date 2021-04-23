# Tarea Corta 1 - María Jesús Vargas Medrano - B98243

Pequeña aplicación Cliente-Servidor para la transferencia de archivos por medio de sockets.


## Compilación

Para la compilación del programa del servidor ("servidor.cpp") lo que se tiene que hacer es escribir en la terminal 
#### g++ servidor.cpp -o ejecutable
Igualmente para la compilación del programa del cliente, solo es escribir lo siguiente en la terminal
#### g++ cliente.cpp -o ejec

## Ejecución de los programas

Para ejecutarlos solo se escriben los nombre de los ejecutables que se les asigno en las respectivas terminales y los programas se ejecutaran
#### ./ejecutable
#### ./ejec

## Funciones de los programas
Al ejecutar al servidor lo primero que este hace es solicitarle el numero de puerto que se utilizara, luego este se queda esperando a escuchar al cliente desde ese puerto. Cuando hay una conexión el servidor ya esta listo para recibir el archivo que el cliente va a enviar. Cuando se copia el archivo por completo, el programa va a mover dicho archivo hacia una carpeta, que el mismo servidor crea. Al terminar la conexión con ese cliente vuelve a preguntar por el numero de puerto por el cual va a escuchar y se queda esperando la siguiente conexión.

Respecto al cliente, este realiza la conexión con el servidor por medio del puerto que tenga definido, luego se le pregunta el nombre del archivo que desea copiar, luego que este nombre es aceptado se pregunta por el nombre que desea ponerle a la copia que el servidor va a realizar. Luego de esto el cliente envía el archivo al servidor, cuando se termina de copiar se le informa al cliente y se cierra la conexión,

En el .zip se adjuntan dos archivos con los cuales se puede realizar las pruebas con los programas.

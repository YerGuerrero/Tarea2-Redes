### Yerlyn Guerrero León 
### 2018175922

# Tarea2-Redes

Para correr la Tarea corta #2 se corren los siguientes comandos:

 * `docker-compose up`
 
Una vez corrido el comando anterior se ejecuta el siguiente: 
  
 * `sudo docker run -it --rm --name servidor -p 9666:9666 servidor`

En el cmd aparecerá el mensaje de que el servidor está corriendo

Por último solo queda abrir una nueva terminal y conectarse al servidor por medio de  `telnet [ip] [puerto]`

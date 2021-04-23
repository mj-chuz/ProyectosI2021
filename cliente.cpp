//Maria Jesus Vargas Medrano B98243
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#define PORT 8080


class Cliente
{
public:

	int socket_cliente = 0;
	struct sockaddr_in direccion;
	FILE * archivoEnviar;
	int bit;
	char bitEnviar[1];
	char nombreArchivo[1024];
	char archivoDestino[1024];
	long int tamanoArchivo=0;



	Cliente()
	{
		socket_cliente = socket(AF_INET, SOCK_STREAM, 0);
  
	    direccion.sin_family = AF_INET;
	    direccion.sin_port = htons(PORT);
	}

	void AsignarDireccion()
	{

	    if(inet_pton(AF_INET, "127.0.0.1", &direccion.sin_addr)<=0) 
	    {
	        std::cout<<"Direccion invalida"<<std::endl;
	        exit(EXIT_FAILURE);
	    }
	}


	void Conectar()
	{
		if (connect(socket_cliente, (struct sockaddr *)&direccion, sizeof(direccion)) < 0)
	    {	
	    	std::cout<<"No se pudo conectar"<<std::endl;
	        exit(EXIT_FAILURE);
	        
	    }else
	    {
	    	std::cout<<"Se conecto con el servidor"<<std::endl;
	    }
	}


	void SolicitarNombresArchivos()
	{
		std::cout<<"Bienvenido!"<<std::endl<<"Escriba el nombre del archivo que desea copiar"<<std::endl;
		std::cin>>nombreArchivo;
		std::cout<<"Dijite el nombre que le desea poner a la copia del archivo"<<std::endl;
		std::cin>>archivoDestino;

	}

	bool AbrirArchivo()
	{
		archivoEnviar = fopen (nombreArchivo, "r");
		if(!archivoEnviar) {
	        std::cout<<"No se pudo abrir el archivo"<<std::endl;
	        close(socket_cliente);
	        return false;
	    }
	    return true;
	}

	void EnviarArchivo()
	{
		
		fseek (archivoEnviar, 0, SEEK_END);
		tamanoArchivo = ftell(archivoEnviar);
		rewind(archivoEnviar);
		send(socket_cliente, archivoDestino, sizeof(archivoDestino), 0);
		while((bit=getc(archivoEnviar)) != EOF)
		{
			bitEnviar[0] = bit;
			send(socket_cliente, bitEnviar,1, 0);
			
		}
		
	}

	void EscribirMensaje()
	{
		std::cout<<"El archivo se envio con exito al servidor"<<std::endl;
		std::cout<<"Nombre del archivo: "<<nombreArchivo<<std::endl;
		std::cout<<"Tamano del archivo: "<<tamanoArchivo<<" bytes"<<std::endl;
		fclose(archivoEnviar);
		close(socket_cliente);
	}

};

int main()
{

	Cliente cliente;
	cliente.AsignarDireccion();
	cliente.Conectar();
	cliente.SolicitarNombresArchivos();
	if(cliente.AbrirArchivo())
	{
		cliente.EnviarArchivo();
		cliente.EscribirMensaje();
	}
	
	return 0;
}
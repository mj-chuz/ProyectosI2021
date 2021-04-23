//Maria Jesus Vargas Medrano - B98243
#include <unistd.h> 
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <sys/stat.h>


class Servidor
{
public:

	int numero_puerto;
	int socket_servidor;
	int conector;
	struct sockaddr_in direccion;
	int direccion_longitud = sizeof(direccion);
	char nombreDirectorio[1024] = "Copia";
	FILE * recibirArchivo;
	char nombreArchivo[1024]={0};
	long int tamanoArchivo=0;
	char recibirBuffer[1024]={0};


	Servidor()
	{
	}

	void CrearSocket()
	{
		if((socket_servidor = socket(AF_INET, SOCK_STREAM, 0)) ==0)
		{
			perror("Fallo del socket");
			exit(EXIT_FAILURE);
		}
		direccion.sin_family = AF_INET;
	    direccion.sin_addr.s_addr = INADDR_ANY;
	    direccion.sin_port = htons( numero_puerto );
	}

	void Unir()
	{
	    if(bind(socket_servidor, (struct sockaddr *)&direccion, sizeof(direccion))<0)
	    {
	    	perror("Fallo al unir");
			exit(EXIT_FAILURE);
	    }
	}

	void Escuchar()
	{

	    if(listen(socket_servidor, 2)<0)
	    {
	    	perror("Fallo al escuchar al cliente");
			exit(EXIT_FAILURE);
	    }else{
	    	std::cout<<"Esperando a escuchar solicitudes del cliente"<<std::endl;
	    }

	    
	}

	void Conectar()
	{

	    if ((conector = accept(socket_servidor, (struct sockaddr *)&direccion, (socklen_t*)&direccion_longitud))<0)
    	{
	        perror("No se acepto la conexion");
	        exit(EXIT_FAILURE);
    	}else{
    		std::cout<<"Se realizo la conexion con el cliente"<<std::endl;
    	}
	}

	void CrearCarpeta()
	{
		mkdir(nombreDirectorio, 0777);
	}


	void setNumeroPuerto(int nuevoPuerto)
	{
		this->numero_puerto=nuevoPuerto;
	}


	void RecibirArchivo()
	{
		if (recv(conector, nombreArchivo, sizeof(nombreArchivo), 0))
		{

			recibirArchivo = fopen(nombreArchivo, "w");

			while(recv(conector, recibirBuffer, 1, 0) != 0)
			{
					
				fwrite(recibirBuffer, sizeof(recibirBuffer[0]), 1, recibirArchivo);
				recibirBuffer[0] = 0;

			}
			tamanoArchivo = ftell(recibirArchivo);
			std::cout<<"El archivo se recibio con exito"<<std::endl;
			std::cout<<"Nombre del archivo: "<<nombreArchivo<<std::endl;
			std::cout<<"Tamano del archivo: "<<tamanoArchivo<< " bytes"<<std::endl;
			close(socket_servidor);
			close(conector);
			fclose(recibirArchivo);
			
		}else{
			std::cout<<"Se cerro la conexion"<<std::endl;
		}
	}

	void MoverArchivo()
	{
		std::string nomArch(nombreArchivo);

	   	char *nombreViejo = get_current_dir_name();

	   	std::string nombreViejoCompl(nombreViejo);

	   	nombreViejoCompl= nombreViejoCompl + "/" + nomArch;

	   	char *nombreNuevo = get_current_dir_name(); ;


	   	std::string nombreNuevoCompleto(nombreNuevo);

	   	nombreNuevoCompleto = nombreNuevoCompleto + "/Copia/" + nomArch;  


	   	char* nom1 = &nombreViejoCompl[0];

	   	char* nom2 = &nombreNuevoCompleto[0];


	   	rename(nom1, nom2);	

	   	std::cout<<"Se copio a la carpeta llamada Copia "<<std::endl;

	}

};


int main()
{

	int  port_number;
	Servidor servidor;
	servidor.CrearCarpeta();
	while(1){
		
		std::cout<<"Dijite el numero de puerto al que desea conectarse"<<std::endl;
		std::cin>>port_number;
		servidor.setNumeroPuerto(port_number);
		servidor.CrearSocket();
		servidor.Unir();
		servidor.Escuchar();
		servidor.Conectar();
		if (port_number>1024 && port_number<65535)
		{
			
			servidor.RecibirArchivo();
			servidor.MoverArchivo();

		}else{
			std::cout<<"Numero de puerto invalido"<<std::endl;
		}
	}
	return 0;
}


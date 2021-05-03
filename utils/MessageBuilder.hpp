#ifndef HTTP_HANDLER
#define HTTP_HANDLER

#include <string>
#include <time.h>

/**
 * 	Clase encargada de crear un std::string que se enviará como respuesta/petición. Cuenta con
 * 	constantes de los códigos de respuesta, metódos y campos del header más comunes.
 * 
 * 	Last Modified: Tyron Fonseca (24/04/2021)
 * 
 **/

namespace HttpMessageBuilder{
	typedef std::string string;
	typedef std::string httpMethod;
	typedef std::string httpCode;
	typedef std::string httpHeader;

	//Peticiones
	const httpMethod GET = "GET";		//Conseguir un recurso
	const httpMethod POST = "POST";		//Crear un recurso
	const httpMethod HEAD = "HEAD";		//Verificar si exite un recurso (igual que GET pero sin devolver el recurso)

	//Respuestas
	const httpCode OK = "200 OK"; 							//Se procesó correctamente la petición
	const httpCode BAD_REQUEST = "400 Bad Request"; 		//Error del cliente en la syntaxis o en algo de la request 
	const httpCode NOT_FOUND = "404 Not Found"; 		 	//El recurso no se encuentra en el servidor
	const httpCode NOT_ACCEPTABLE = "406 Not Acceptable";	//No hay recursos del tipo que el cliente necesita (Header Accept)
	const httpCode NOT_IMPLEMENTED = "501 Not Implemented"; //El método no está implementado en el server

	//Campos del header
	const httpHeader ACCEPT = "Accept: ";				  //Tipo de archivo aceptado en el request
	const httpHeader CONTENT_TYPE = "Content-type: ";	  //Tipo de archivo que se envía o enviará
	const httpHeader CONTENT_LENGTH = "Content-length: "; //Tamaño en bytes del body
	const httpHeader DATE = "Date: ";					  //Fecha y hora en el que el request fue originado (Formatos: rfc1123-date | rfc850-date | asctime-date)
	const httpHeader HOST = "Host: ";					  //Host donde se encuentra el recurso que se necesita (Dominio y puerto)
	const httpHeader REFERER = "Referer: ";				  //URI del recurso como se vería en el navegador
	const httpHeader SERVER = "Server: ";				  //Información del software del server para manejar request
	
	class MessageBuilder
	{	
		private:
			string initialLine; //Primera linea del header
			string headers;		//Donde se agregaran los headers
			string dateHeader;	//Fecha
			string body;		//Contenido que se enviará
			string serverName;  //software del server

			const string HTTP_VERSION = "HTTP/1.1";
			const string CRLF = "\r\n";

			/**
		 	* @brief De uso obligatorio para HTTP/1.1. Conseguir la fecha y hora actual en formato RFC 1123. Esta función se ve afectada por el locale.
		 	* @return String con la fecha y hora en formato RFC 1123
		 	**/
			string getRFC1123Date()
			{
				//setlocale(LC_ALL, ""); forzar default locale
				char buffer[30];
				time_t rawTime = time(nullptr);
				struct tm *m_tm = gmtime(&rawTime);
				strftime(buffer, 30, "%a, %d %b %Y %H:%M:%S GMT", m_tm);
				return buffer;
			}

		public: 

			MessageBuilder()
			{
				this->serverName = "Mapache/0.0.1 (Linux)";
			}

			MessageBuilder(const string& serverName){
				this->serverName = serverName;
			}

			//================REQUESTS================================
			
			/**
			 * @brief Se encarga de modificar la primera linea del header. Este método solo debe ser usado por el cliente
			 * cuando se realizar un request.
			 * @param method Método que se usará: GET, HEAD o POST (Usar constantes de esta clase)
			 * @param uri Dirección del recurso
			 * @return Puntero a este objeto para poder enlazar funciones
			 **/
			MessageBuilder &setMethod(const httpMethod &method, const string &uri)
			{
				this->initialLine = method + " " + uri + " " + HTTP_VERSION + CRLF;
				return *this;
			}

			/**
			* @brief Se encarga de agregar headers. 
			* @param field Campo del header que se agragará. (Usar constantes de esta clase)
			* @param value Valor del field que se agregó
			* @return Puntero a este objeto para poder enlazar funciones
			**/
			MessageBuilder &addHeader(const httpHeader &field, const string &value)
			{
				this->headers += field + value + CRLF;
				return *this;
			}

			/**
			* @brief Se encarga de agregar el header Accept 
			* @param value Valor del header
			* @return Puntero a este objeto para poder enlazar funciones
			**/
			MessageBuilder& accept(const string& value)
			{
				return this->addHeader(ACCEPT, value);
			}

			/**
			* @brief Se encarga de agregar el header Content-Type
			* @param value Valor del header
			* @return Puntero a este objeto para poder enlazar funciones
			**/
			MessageBuilder &contentType(const string &value)
			{
				return this->addHeader(CONTENT_TYPE, value);
			}

			/**
			* @brief Se encarga de agregar el header Content-length 
			* @param value Valor del header
			* @return Puntero a este objeto para poder enlazar funciones
			**/
			MessageBuilder &contentLength(const string &value)
			{
				return this->addHeader(CONTENT_LENGTH, value);
			}

			/**
			* @brief Se encarga de agregar el header Date 
			* @param value Valor del header
			* @return Puntero a este objeto para poder enlazar funciones
			**/
			MessageBuilder &date(const string &value)
			{
				this->dateHeader = value;
				this->headers += DATE + value + CRLF;				
				return *this;
			}

			/**
			* @brief Se encarga de agregar el header Date con la fecha en formato RFC 1123
			* @return Puntero a este objeto para poder enlazar funciones
			**/
			MessageBuilder &date()
			{
				return this->date(getRFC1123Date());
			}

			/**
			* @brief Se encarga de agregar el header Host
			* @param value Valor del header
			* @return Puntero a este objeto para poder enlazar funciones
			**/
			MessageBuilder &host(const string &value)
			{
				return this->addHeader(HOST, value);
			}

			/**
			* @brief Se encarga de agregar el header Referer
			* @param value Valor del header
			* @return Puntero a este objeto para poder enlazar funciones
			**/
			MessageBuilder &referer(const string &value)
			{
				return this->addHeader(REFERER, value);
			}

			/**
			* @brief Se encarga de agregar el header Server
			* @param value Valor del header
			* @return Puntero a este objeto para poder enlazar funciones
			**/
			MessageBuilder &server(const string &value)
			{
				return this->addHeader(SERVER, value);
			}

			/**
			* @brief Se encarga de agregar el contenido o body del request o respuesta.
			* @param body Contenido que se enviará al cliente/servidor
			* @return Puntero a este objeto para poder enlazar funciones
			**/
			MessageBuilder &addBody(const string &body)
			{
				this->body += body;
				return *this;
			}

			/**
			* @brief Limpiar los strings para crear un nuevo request
			* @return void
			**/
			void clearHttpMessage()
			{
				this->initialLine = "";
				this->headers = "";
				this->dateHeader = "";
				this->body = "";
			}

			/**
			* @brief Retorna tamaño del body
			* @return Tamaño en bytes
			**/
			string getContentLength(){				
				return std::to_string(this->body.size());
			}

			/**
			* @brief Retorna el request listo para enviarse.		
			* @return std::string Request listo
			**/
			string getHttpMessage()
			{
				if(dateHeader.size() == 0){
					this->date();
				}

				if (body.size() > 0)
				{
					return initialLine + headers + CRLF + body;
				}
				else
				{
					return initialLine + headers + CRLF;
				}
			}


			//==================RESPONSES===============================
			/**
			* @brief Se encarga crear un response generico con los headers básicos
			* @param response Tipo de respuesta: 400, 404, 200, .... (Usar constantes de esta clase)
			**/
			void buildResponse(const httpCode &response)
			{
				this->clearHttpMessage();
				this->initialLine = HTTP_VERSION + " " + response + CRLF;
				this->addHeader(SERVER, serverName);
				this->date();
			}

			/**
			* @brief Se encarga de crear un response en el que se enviar contenido al cliente
			* @param response Tipo de respuesta: 400, 404, 200, .... (Usar constantes de esta clase)
			* @param type Mime-Type del body
			* @param body Contenido que se le enviará al cliente
			**/
			void buildResponse(const httpCode &response, const string &type, const string &body)
			{
				buildResponse(response);
				if(type.size() > 0 && body.size() > 0){
					this->addHeader(CONTENT_TYPE, type);
					this->addBody(body);
					this->contentLength(this->getContentLength());
				}	
			}
			/**
			* @brief Se encarga de crear un response en el que se enviar contenido al cliente sin enviar el body
			* @param response Tipo de respuesta: 400, 404, 200, .... (Usar constantes de esta clase)
			* @param type Mime-Type del body
			* @param body Contenido que se le enviará al cliente
			**/
			void buildResponseWithoutBody(const httpCode &response, const string &type, int contentLength)
			{
				buildResponse(response);
				if (type.size() > 0)
				{
					printf("Entre\n");
					this->addHeader(CONTENT_TYPE, type);
					this->contentLength(std::to_string(contentLength));
				}
			}
	};
}
#endif
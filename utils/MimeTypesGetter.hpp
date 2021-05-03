#ifndef MIMETYPESGET
#define MIMETTYPEGET

#include <string>
#include <regex>
#include <map>
#include <fstream>

/**
 * 	Clase encargada leer un archivo xml con los mime types y transformalo en un diccionario (std::map)
 * 
 * 	Last Modified: Tyron Fonseca (30/04/2021)
 * 
 **/
class MimeTypesGetter
{
	typedef std::string string;
private:
	std::map<std::string, std::string> dicMIME;

	/**
	* @brief Abre un archivo .csv con las extensiones y los MIME-Types, luego los guarda en un 
	* diccionario (map) para su uso posterior
	* @return void
	* */
	void loadMIMETypes(const string& filePath)
	{
		std::string linea, ext, type;
		std::ifstream in(filePath);

		getline(in, linea);//Headers

		while (in.good())
		{	
			getline(in, ext, '\t');
			getline(in, type);
			dicMIME.insert(std::pair<std::string, std::string>(ext, type)); //Agregar al diccionario
		}
		in.close();
	}

public:

	MimeTypesGetter(const string& filePath)
	{
		this->loadMIMETypes(filePath);
	}

	/**
	* @brief Permite conseguir el MIME-type de un archivo dada su extensión.
	* @param extension Extensión del archivo sin el punto (.), ejemplo: ".js" debe de ser enviado como "js".
	* @return MIME-Type correspondiente
	* */
	std::string getTypeContentByExt(const std::string &extension)
	{
		std::string result = "application/octet-stream"; //https://www.rfc-editor.org/rfc/rfc2046.html#section-4.5.1
		std::map<std::string, std::string>::iterator it = dicMIME.find(extension);
		if (it != dicMIME.end())
		{
			result = it->second;
		}

		return result;
	}
	/**
	* @brief Verifica si un mime type tiene el formato válido
	* @param type Mime type a verificar
	* @return False: no es válido - True: es válido.
	* */
	bool isMimeTypeValid(const string& type){
		return std::regex_search(type, std::regex(R"((\w+|\*)/([-+.\w|(\*)]+))"));
	}
			
};

#endif
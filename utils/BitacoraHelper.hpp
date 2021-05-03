#ifndef BITACORA_H
#define BITACORA_H
#include <string>
#include <fstream>
/**
 * 	Clase encargada de crear y escribir en un archivo de bitacora.
 * 
 * 	Last Modified: Tyron Fonseca (30/04/2021)
 * 
 **/
class BitacoraHelper{   
typedef std::string string;
private:
    std::ofstream* result_file;
    const string SEP = "\t"; //Separator

public:
    typedef struct{
        string method;
        string time;
        string server;
        string referer;
        string uri;
        string data;
    } BitacoraData;

    /**
     *  @brief Constructor
     *  @param pathFile path and file name 
     **/
    BitacoraHelper(string pathFile){
        result_file = new std::ofstream(pathFile, std::ofstream::out);
        //Header
        *result_file << "Method" << SEP << "Time" << SEP << "Server" << SEP << "Referer";
        *result_file << SEP << "URL" << SEP << "Data" << "\n";
    }

    /**
     *  @brief Destroyer
     **/
    ~BitacoraHelper(){        
        delete result_file;
    }

    /**
     *  @brief Writes a new line in the file
     *  @pram dataBitacora Struct with the data to write
     **/
    void writeBitacoraLine(BitacoraHelper::BitacoraData& dataBitacora){
        *result_file << dataBitacora.method << SEP << dataBitacora.time << SEP;
        *result_file << dataBitacora.server << SEP << dataBitacora.referer << SEP;  
        *result_file << dataBitacora.uri << SEP << dataBitacora.data << "\n"; 
        result_file->flush();
    }

    /**
    * @brief Close the file
    **/
    void close(){
        result_file->close();
    }

    /**
     * @brief Get the current size of the file.
     * @return The size of the file in bytes.
     **/
    int getCurrentSize(){
        return result_file->tellp();
    }

};
#endif
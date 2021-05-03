#include "server.hpp"

const int max_requests = 100;
int debug = FALSE;
MimeTypesGetter* mimeGetter;
BitacoraHelper* bitacora;
pthread_mutex_t lock;


/**
 * @brief Displays error message and exits.
 * @param message Name of function called before error.
 */
void fatal(const char *message)
{
    if(errno > 0)
    {
        char error_message[ERROR_MESSAGE_SIZE];
        strcpy(error_message, "[ERROR] ");
        strncat(error_message, message, ERROR_MESSAGE_SIZE - 9);
        perror(error_message);
        exit(-1);
    }
}

/**
 * @brief Sets a file descriptor in non blocking mode to avoid waiting on sockets operations.
 * @param File descriptor to set in non blocking mode.
 */
void set_non_blocking(int fd)
{
    fcntl(fd, F_SETFL, (fcntl(fd, F_GETFL) | O_NONBLOCK));
}

/**
 * @brief Arguments parsing. Displays helps message and exits in case of wrong option.
 * @param argc Number of arguments
 * @param argv Vector of arguments
 * @param port Port reference to modify if -p option as argument.
 */

void arguments(int argc, char** argv, u_short *port)
{

    for(int i = 1; i < argc; i++)
    {
        // port 
        if((!strcmp(argv[i],"-p") || !strcmp(argv[i],"--port")) && (i < (argc-1)))
        {
            if(sscanf(argv[++i],"%hu", port) <= 0)
                help(argv[0]);
        }
        // help message
        else if(!strcmp(argv[i],"-h") || !strcmp(argv[i],"--help"))
            help(argv[0]);

        // Debug mode
        else if(!strcmp(argv[i],"-D") || !strcmp(argv[i],"--debug"))
            debug = TRUE;

        else
            help(argv[0]);
    }
}

/**
 * @brief handles a signal.
 * @param signal
 */
void signal_memory_handler(int sig)
{
    free_memory();
    exit(0);
}

/**
 * @brief Displays help message.
 * @param name Name used to execute this program.
 */
void help(char* name)
{
    printf( "Usage: %s [-D|--debug] [OPTION]\n\n"
            "Mandatory arguments for options.\n"
            "\t-p\t--port\t\tport number.\n"
            "\nNo arguments options.\n"
            "\t-D\t--debug\t\tshow debug output.\n"
            "\t-h\t--help\t\tdisplays this message.\n", name);

    exit(1);
}


/**
 * @brief parses headers of a request.
 * @param request Struct of a request to save the parsed data.
 */
void parser(request_t* request)
{
    request->header_buffer[strstr(request->header_buffer, "\r\n\r\n") - request->header_buffer] = '\0';

    if(debug){
        printf("Parsing %s\n", request->header_buffer);
    }
        
    std::string s(request->header_buffer);
    std::istringstream ss(s);
    std::string temp;

    ss >> request->header.method >> request->header.uri >> request->header.version;

    
    if(!request->header.method.compare("GET"))
        request->method_f = method_get;

    else if(!request->header.method.compare("HEAD"))
        request->method_f = method_head;

    else if(!request->header.method.compare("POST"))
        request->method_f = method_post;

    else
        request->method_f = method_not_implemented;

    // parse each line after the first one
    while(!ss.eof())
    {
        ss >> temp;
        std::getline(ss,s);

        std::for_each(temp.begin(), temp.end(), [](char & c) {
            c = ::tolower(c);
        });

        if(!temp.compare("accept:"))
            request->header.accept = s.erase(0,1);
        else if(!temp.compare("content-type:"))
            request->header.content_type = s.erase(s.size()-1);
        else if(!temp.compare("content-length:"))
            request->header.content_length = s.erase(s.size()-1);
        else if(!temp.compare("referer:"))
            request->header.referer = s.erase(s.size()-1);
        else if(!temp.compare("date:"))
            request->header.date = s.erase(s.size()-1);
                 
    }
    
}

void saveInBitacora(request_t* request){
     //Bitacora
    BitacoraHelper::BitacoraData bitacoraData;
    bitacoraData.method = request->header.method;
    bitacoraData.server = "localhost";
    bitacoraData.referer = request->header.referer;
    bitacoraData.time = std::to_string(std::time(0));
    bitacoraData.uri = request->header.uri.substr(0, request->header.uri.find("?"));
    if(!request->header.method.compare("POST")){
        bitacoraData.data = request->entity;
    }else{
        bitacoraData.data = request->header.uri.substr(request->header.uri.find("?")+1); 
    }       

    pthread_mutex_lock(&lock);
    bitacora->writeBitacoraLine(bitacoraData);
    pthread_mutex_unlock(&lock);
}

/*
 *  @brief 
 *  @param acceptHeader File descriptor of epoll.
 *  @param mimeTypeFile Reference to a request structure
 *  @return 
*/
bool handleAccept(const std::string& acceptHeader, const std::string mimeTypeFile)
{
    bool result= false;   
    int position= acceptHeader.find(mimeTypeFile);

    if(position != std::string::npos)
        result= true;
    else {
        position= acceptHeader.find("*/*");
        if(position != std::string::npos)
            result= true;
    }
        return result;
}

/**
 *  @brief The GET method retrieve whatever information (in the form of an entity) is identified by the Request-URI
 *  @param requestParsed A structure with the client's request parsed
 *  @return The headers with the result of the GET
 */
std::string method_get(req_header_t* requestParsed)
{
    MessageBuilder builder;  
    bool accept= (requestParsed->accept != "*/*")?true:false;

    int position;
    std::string extension;

    if(requestParsed->uri == "/")
        requestParsed->uri= "/index.html";


    requestParsed->uri= "./www/html" + requestParsed->uri;

    position= requestParsed->uri.find("?"); 
    if(position != std::string::npos)
    {
        requestParsed->body= requestParsed->uri.substr(position+1);
        requestParsed->uri= requestParsed->uri.substr(0, position);
    }
    else
    {
        requestParsed->body= requestParsed->uri;
    }

    position= requestParsed->uri.find_last_of(".");
    extension= requestParsed->uri.substr(position+1);

    requestParsed->content_type= mimeGetter->getTypeContentByExt(extension);
   
    if(mimeGetter->isMimeTypeValid(requestParsed->content_type))
    {
        if(accept)
        {
            if(!handleAccept(requestParsed->accept, requestParsed->content_type)){
                builder.buildResponse(NOT_ACCEPTABLE);
                return builder.getHttpMessage();
            }
        }

        FILE* file = fopen(&requestParsed->uri[0], "rb");

        unsigned long filesize = ftell(file);
        char buffer[filesize];

        rewind(file);
        fread(buffer, sizeof(char), filesize, file);
        requestParsed->body = buffer;
        if(file)
        {
            if(handleAccept(requestParsed->accept, requestParsed->content_type) || !accept){
                builder.buildResponse(OK, requestParsed->content_type, requestParsed->body);
            }
            else
            {
                builder.buildResponse(BAD_REQUEST);
            }

        }
        else
        {
            builder.buildResponse(NOT_FOUND);
        }
    }
    else
    {
        builder.buildResponse(BAD_REQUEST);
    }


    return builder.getHttpMessage();
}

/**
 *  @brief The HEAD method is identical to GET except that the server MUST NOT return a message-body in the response
 *  @param requestParsed A structure with the client's request parsed
 *  @return The headers with the result of the HEAD
 */
std::string method_head(req_header_t* requestParsed)
{
    MessageBuilder builder; 
    bool accept= (requestParsed->accept != "*/*")?true:false;

    int position;
    std::string extension;

    if(requestParsed->uri == "/")
        requestParsed->uri= "/index.html";

    requestParsed->uri= "./www/html" + requestParsed->uri;

    position= requestParsed->uri.find("?"); 
    if(position != std::string::npos)
    {
        requestParsed->body= requestParsed->uri.substr(position+1);
        requestParsed->uri= requestParsed->uri.substr(0, position);
    }
    else
    {
        requestParsed->body= requestParsed->uri;
    }

    position= requestParsed->uri.find_last_of(".");
    extension= requestParsed->uri.substr(position+1);

    requestParsed->content_type= mimeGetter->getTypeContentByExt(extension);
   
    if(mimeGetter->isMimeTypeValid(requestParsed->content_type))
    {
        if(accept)
        {
            if(!handleAccept(requestParsed->accept, requestParsed->content_type)){
                builder.buildResponse(NOT_ACCEPTABLE);
                return builder.getHttpMessage();
            }
        }
        
        FILE* file = fopen(&requestParsed->uri[0], "rb");

        unsigned long filesize = ftell(file);
        char buffer[filesize];

        rewind(file);
        fread(buffer, sizeof(char), filesize, file);
        requestParsed->body = buffer;
        if(file)
        {
            if(handleAccept(requestParsed->accept, requestParsed->content_type) || !accept){
                int len = 0;
                sscanf(&requestParsed->content_length[0], "%d", &len);
                builder.buildResponseWithoutBody(OK, requestParsed->content_type, len);
            }
            else
            {
                builder.buildResponse(BAD_REQUEST);
            }

        }
        else
        {
            builder.buildResponse(NOT_FOUND);
        }
    }
    else
    {
        builder.buildResponse(BAD_REQUEST);
    }


    return builder.getHttpMessage();
}

/**
 *  @brief is used to request that the origin server accept the entity enclosed in the request 
 *  @param requestParsed Reference to a request header structure
 *  @return the headers with the result of the post
 */
std::string method_post(req_header_t* requestParsed)
{

    MessageBuilder builder; 

    if (requestParsed->uri=="/")
    {
        requestParsed->uri="www/html/index.html";
    }

    if(mimeGetter->isMimeTypeValid(requestParsed->content_type))
    {
        bool accept=false;
        if(requestParsed->accept[0]!='*')
        {
            accept=true;
        }
        requestParsed->uri="./www/html" + requestParsed->uri;

        requestParsed->body=requestParsed->uri;

        char* name= &requestParsed->uri[0];

        FILE* file = fopen(name, "rb");

        if(file)
        {
            if (accept)
            {
                if (requestParsed->accept==requestParsed->content_type)
                {
                    builder.buildResponse(OK, requestParsed->content_type, requestParsed->body);
                }else{

                    builder.buildResponse(BAD_REQUEST);
                }
            }else{
                builder.buildResponse(OK, requestParsed->content_type, requestParsed->body);
            }
        }else{
            builder.buildResponse(NOT_FOUND);
        }

    }else{
        builder.buildResponse(BAD_REQUEST);
    }

    return builder.getHttpMessage();
}

/**
 *  @brief it gets call when is asked for a method not implented
 *  @param requestParsed Reference to a request header structure
 *  @return the headers with the result of the method not implemented
 */
std::string method_not_implemented(req_header_t* requestParsed)
{
    MessageBuilder builder;
    builder.buildResponse(NOT_IMPLEMENTED, mimeGetter->getTypeContentByExt("json"), "{\"error\":\"Not Implemented\"}");
    return builder.getHttpMessage();
}

/**
 *  @brief add event to poll in reading awareness. Uses EPOLLET & EPOLLONESHOT to avoid
 *  spurious wake ups from other threads.
 *  @param epoll_fd File descriptor of epoll.
 *  @param request Reference to a request structure
 *  @return 1 if succeeded 0 otherwise.
 */
int poll_status_reading(int epoll_fd, request_t* request)
{
    struct epoll_event event;
  
    event.events   = EPOLLIN | EPOLLET | EPOLLONESHOT;
    event.data.ptr = (void*)request;

    if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, request->socket, &event) < 0)
    {
        perror("in epoll_ctl()");
        return 0;
    }
    
    return 1;
}


/**
 *  @brief add event to poll in writing awareness. Uses EPOLLET & EPOLLONESHOT to avoid
 *  spurious wake ups from other threads.
 *  @param epoll_fd File descriptor of epoll.
 *  @param request Reference to a request structure
 *  @return 1 if succeeded 0 otherwise.
 */
int poll_status_writing(int epoll_fd, request_t* request)
{
   struct epoll_event event;
  
   event.events   = EPOLLOUT | EPOLLET | EPOLLONESHOT;
   event.data.ptr = (void*)request;

    if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, request->socket, &event) < 0)
    {
        perror("in epoll_ctl()");
        return 0;
    }
    
    return 1;
}

void free_memory()
{    
    bitacora->close();

    delete mimeGetter;
    delete bitacora;

    printf("\nServer shutdown\n");
}


void close_file(request_t* request)
{
    return;
}


/**
 * @brief Accepts a connection in master_socket 
 * @param master_socket File descriptor of the listening socket
 * @param address_ptr Address struct to save request address
 * @returns File descriptor of new socket
 **/

int new_connection(int master_socket, struct sockaddr_in* address_ptr)
{
    int addrlen = sizeof(*address_ptr);
    int new_socket = accept(master_socket, (struct sockaddr*)address_ptr, (socklen_t*)&addrlen);
    if (new_socket < 0)
    {
        if(errno != EAGAIN && errno != EWOULDBLOCK)
            fatal("in accept()");
        else
            return new_socket;
    }
    printf("New connection, socket fd: %d\nip: %s,port: %hu\n", new_socket, inet_ntoa(address_ptr->sin_addr), ntohs(address_ptr->sin_port));
    return new_socket;
}

/**
 * @brief Memory and epoll management of new requests
 * @param master_socket File descriptor of the listening socket
 * @param epoll_fd File descriptor of a epoll instance
 */

void handle_connection(int master_socket, int epoll_fd)
{
    struct sockaddr_in address;
    struct epoll_event event;

    int sd;

    while(TRUE)
    {
        sd = new_connection(master_socket, &address);

        if(sd < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            break;

        set_non_blocking(sd); // all sockets file descriptors should not block the thread.

        // memory assignation
        request_t* request = new request_t();

        request->socket = sd;
        request->status = READING;
        
        memset(request->header_buffer, 0, HEADER_SIZE+1);
        memset(request->entity, 0, ENTITY_SIZE+1);

        request->header_bytes_read  = 0;
        request->header_read        = FALSE;

        request->entity_bytes_read  = 0;
        request->entity_length      = 0;

        request->response.header_sent = 0;
        request->response.header_length = 0;

        // epoll event
        event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
        event.data.ptr = (void*)request;

        if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sd, &event) < 0)
            perror("in epoll_ctl()");
    }
}

/**
 * @brief gets connections and monitor socket activity through epoll_wait
 * @param args Void pointer for thread data.
 */
void* thread(void* args)
{
    char buffer[BUFFER_SIZE+1];

    struct epoll_event events_list[max_requests];
    struct epoll_event event;


    int epoll_fd        = ((thread_data_t*) args)->epoll_fd;
    int master_socket   = ((thread_data_t*) args)->master_socket;

    int sd;
    int event_count;

    while(TRUE)
    {

        event_count = epoll_wait(epoll_fd, events_list, max_requests, -1);

        if(event_count <= 0)
        {
            perror("in epoll_wait()");
            continue;
        }

        // An event ocurred
       
        for(int i = 0; i < event_count; i++)
        {
            sd = ((request_t*) events_list[i].data.ptr)->socket;
        
            // master socket event
            if(sd == master_socket)
                handle_connection(sd, epoll_fd);  

            else    // request socket
            {
                request_t* request = (request_t*) events_list[i].data.ptr;

                if(request->status == READING)
                {
                    while(TRUE) // if there are bytes left after recv
                    {
                        int recv_bytes = (recv(sd, buffer, BUFFER_SIZE-1, 0));

                        if(recv_bytes < 0)
                        {
                            if(errno != EAGAIN && errno != EWOULDBLOCK)
                                request->status = ENDED;
                            else
                            {
                                request->status = READING;
                                poll_status_reading(epoll_fd, request);
                            }
                            break;
                        }

                        if(recv_bytes == 0)
                        {
                            request->status = ENDED;
                            break;
                        }

                        else    // bytes received 
                        {
                            buffer[recv_bytes] = '\0';
                            if(!request->header_read)
                            {

                                char* header_end = strstr(buffer, "\r\n\r\n");
                                int crlf_size = 0;
                               
                                if(header_end)
                                    crlf_size = 4;

                                else if(!header_end && request->header_bytes_read > 1 && recv_bytes > 1 &&
                                        request->header_buffer[request->header_bytes_read-2]   == '\r' &&
                                        request->header_buffer[request->header_bytes_read-1]   == '\n' &&
                                        buffer[0]                                       == '\r' &&
                                        buffer[1]                                       == '\n')
                                        {
                                            header_end = buffer;
                                            crlf_size = 2;
                                        }


                                else if(!header_end && request->header_bytes_read > 0 && recv_bytes > 2 &&
                                        request->header_buffer[request->header_bytes_read-1]   == '\r' &&
                                        buffer[0]                                       == '\n' &&
                                        buffer[1]                                       == '\r' &&
                                        buffer[2]                                       == '\n')
                                        {
                                            header_end = buffer;
                                            crlf_size = 1;
                                        }



                                if(!header_end) // End of header not found
                                {
                                    request->header_bytes_read += recv_bytes;
                                    
                                    printf("Header bytes read %d bytes\n", request->header_bytes_read);

                                    if(request->header_bytes_read < HEADER_SIZE)
                                        strncat(request->header_buffer, buffer, recv_bytes);
                                    else
                                    {
                                        request->status = WRITING;
                                        break;
                                        // 431 Header Too Large
                                    }
                                }
                                else
                                {
                                    request->header_bytes_read += header_end - buffer;

                                    if(request->header_bytes_read < HEADER_SIZE)
                                    {
                                        request->header_read = TRUE;
                                        strncat(request->header_buffer,buffer, header_end - buffer + crlf_size);
                                        parser(request);

                                        if(request->header.content_length.compare("") != 0){
                                           request->entity_length = stoi(request->header.content_length);
                                        }

                                        if(request->entity_length > 0)
                                        {
                                            request->entity_bytes_read = recv_bytes - (header_end - buffer + crlf_size);

                                            if(request->entity_bytes_read < ENTITY_SIZE)
                                            {
                                                strncat(request->entity, header_end + crlf_size, request->entity_length);
                                                request->entity_bytes_read = strlen(request->entity);

                                                if(request->entity_bytes_read >= request->entity_length){
                                                    request->status = WRITING;
                                                    break;
                                                }                             

                                                continue;
                                            }
                                            else
                                            {
                                                request->status = WRITING;                                                
                                                break;
                                                // 413 Entity Too Large
                                            }

                                        } 
                                        
                                        else    // No entity
                                        {
                                            request->status = WRITING;
                                            break;
                                        }

                                        
                                    }
                                    else
                                    {
                                        request->status = WRITING;
                                        break;
                                        // 431 Header Too Large
                                    }

                                }
                            }


                            if(request->header_read)
                            {
                                if(request->entity_length > request->entity_bytes_read) // Content-length > 0
                                {
                                    request->entity_bytes_read += recv_bytes;
                                    if(request->entity_bytes_read < ENTITY_SIZE)
                                    {
                                        strncat(request->entity, buffer, recv_bytes);
                                    }
                                    else
                                    {
                                        request->status = WRITING;
                                        break;
                                        // 413 Entity Too Large
                                    }
                                }
                                else
                                {
                                    request->status = WRITING;
                                    break;
                                }
                            }
                        }
                    }
                }

                if(request->status == WRITING)
                {
                   saveInBitacora(request);

                    //Send response to the client
                   if(request->method_f)
                   {
                        request->header.body += request->entity;
                       request->response.header = request->method_f(&request->header);
                       request->response.header_length = request->response.header.size();
                   }                    

                    int sent;

                    while(request->response.header_sent < request->response.header_length)
                    {
                    sent = send(sd, request->response.header.data() + request->response.header_sent, request->response.header_length - request->response.header_sent, 0);
                        request->response.header_sent = sent;
                        if(sent < 0)
                            if(errno != EAGAIN && errno != EWOULDBLOCK)
                                close(sd);
                            else
                            {
                                poll_status_writing(epoll_fd, request);
                                break;
                            }
                    }

                    if(request->response.header_sent >= request->response.header_length)
                        request->status = ENDED;

                }

                if(request->status == ENDED)
                {
                    close(sd);
                    delete request;
                }
            }
        }
    }
}


int main(int argc, char* argv[])
{
    struct epoll_event event;

    int master_socket;
    int THREAD_NUM = 5;
    int opt = TRUE;
    int epoll_fd;
    int addrlen;

    //Create log folder
    struct stat sb;
    const char* logsFolder = "./logs";
    if(stat(logsFolder, &sb) < 0 && !S_ISDIR(sb.st_mode)){
        mkdir("./logs", 0777);
    } 

    //Initialize helpers
    mimeGetter = new MimeTypesGetter("./resources/mimeTypes.csv");

    time_t t = time(0);
    struct tm * now = localtime(&t);
    char timeStr[22];
    strftime(timeStr, 22, "_%d_%m_%Y_%H_%M_%S", now);
    bitacora = new BitacoraHelper("./logs/bitacora"+std::string(timeStr)+".csv");

    struct sockaddr_in address;

    u_short port = 80;

    socklen_t sin_size;
    
    signal(SIGINT, signal_memory_handler);
    
    arguments(argc, argv, &port);
  
    if(debug)
        printf("[DEBUG] port: %hu\n",port);

    pthread_t threads[THREAD_NUM];


    // Socket initiation & binding
    master_socket = socket(AF_INET, SOCK_STREAM, 0); fatal("in socket()");

    setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); fatal("in setsockopt()");


    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = 0;              // localhost
    memset(&(address.sin_zero), '\0', 8);     // struct sockaddr_in padding

    addrlen = sizeof(address);

    bind(master_socket, (struct sockaddr*)&address, addrlen); fatal("in bind()");
    listen(master_socket, 5); 

    set_non_blocking(master_socket);


    // Epoll initiation
    epoll_fd = epoll_create1(0); fatal("in epoll_fd()");
   
    // using the request_t structure for compatibility on coming events
    request_t* master = new request_t();
    master->socket = master_socket;

    event.events  = EPOLLIN | EPOLLET;
    event.data.ptr = (void*) master;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, master_socket, &event); fatal("in epoll_ctl()");

    
    // Threads
    thread_data_t thread_data;
    thread_data.epoll_fd = epoll_fd;
    thread_data.master_socket = master_socket;

    for(int i = 0; i < THREAD_NUM; ++i)
    {
        if(pthread_create(&threads[i], NULL, thread, &thread_data) < 0)
        {
            fprintf(stderr, "pthread_create() failed.\n");
            exit(1);
        }
    }

    std::cout << "Server running. Waiting for connetions..." << std::endl;

    thread(&thread_data);

    return 0;
}

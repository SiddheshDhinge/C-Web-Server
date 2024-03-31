/*
    @Author: Siddhesh
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

const int PORT = 3005;
const int sizeOfRequest = 1024;

char* getPath(unsigned char* buff)
{
    int startIndex = -1, endIndex = -1;
    for(int i=0;i<sizeOfRequest;i++)
    {
        if(buff[i] == ' ')
        {
            if(startIndex == -1)
            {
                startIndex = i + 1;
            }
            else
            {
                endIndex = i;
                break;
            }
        }
    }

    int sizeOfPath = endIndex - startIndex;

    char* requestedPath = (char*) malloc(sizeof(char) * sizeOfPath + 1);

    for(int i=startIndex; i<endIndex; i++)
    {
        requestedPath[i - startIndex] = buff[i];
    }

    requestedPath[sizeOfPath] = '\0';

    return requestedPath;
}

char* getFileForPath(char *path, int pathSize)
{
    if (strcmp(path, "/a") == 0)
    {
        return "/home/saddy/webserver/public/hello.txt";
    }
    else if (strcmp(path, "/b") == 0)
    {
        return "/home/saddy/webserver/public/bye.txt";
    }

    return "";
}


int size(char *ptr)
{
    int offset = 0;
    
    while (*(ptr + offset) != '\0')
    {
        ++offset;
    }
    return offset;
}

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;
    // create a server socket FD 
    if(sockfd < 0)
    {
        printf("Cant create socket3\n");
        return -1;
    }

    // create socket configs
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    int enable = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        printf("Cant set SO_REUSEADDR option.\n");
        return -1;
    }

    // bind to socket
    int retVal = bind(sockfd, (struct sockaddr *) &address, sizeof(address));
    if(retVal < 0)
    {
        printf("Cant bind to socket\n");
        return -1;
    }

    // listen on socket
    retVal = listen(sockfd, 4096);

    if(retVal < 0)
    {
        printf("Cant listen to socket\n");
        return -1;
    }

    printf("Server is listening at http://localhost:%d 🔥🔥🔥\n\n", PORT);

    while(1)
    {
        // client connects
        struct sockaddr_in clientaddress;
        int sizeOfClientAddress = 0;
        int clientfd = accept(sockfd, (struct sockaddr *) &clientaddress, &sizeOfClientAddress);
        
        if(clientfd < 0)
        {
            printf("Cant accept on socket\n");
            return -1;
        }

        // read what client says
        unsigned char *buff = (unsigned char*) malloc(sizeof(unsigned char) * sizeOfRequest);
        memset(buff, '\0', sizeOfRequest); 
        
        int bytesRead = read(clientfd, buff, sizeOfRequest);

        printf("============= START REQ =============\n");
        fflush(stdout);
        
        printf("%s\n", buff);

        printf("============= END REQ ==============\n");

        int pathSize = 0;
        char* path = getPath(buff); 
        pathSize = size(path);

        printf("Path Size: %d\n", pathSize);
        printf("PATH: %s\n", path);
        
        int fileNameSize = 0;
        char* fileName = getFileForPath(path, pathSize);
        fileNameSize = size(fileName);
        
        printf("File Name Size: %d\n", fileNameSize);
        printf("File Name: %s\n", fileName);
        
        fflush(stdout);

        if(fileNameSize == 0)
        {
            printf("Invlid Path\n");
            // invalid path
            char* response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n 404 NOT FOUND";
            write(clientfd, response, size(response));
            goto CLOSE;
        }
        
        int resFileFD = open(fileName, O_RDONLY, 0);
        if(resFileFD < 0)
        {
            printf("Cant Open File\n");
            char* response = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\n 500 Internal Server Error";
            write(clientfd, response, size(response));
            goto CLOSE;
        }

        printf("Sending Reponse...\n");
        char* responseBuffer = (char*) malloc(sizeof(char) * 1024);
        read(resFileFD, responseBuffer, 1024);

        char responseWithHeader[2048] = "HTTP/1.1 200 OK Error\r\nContent-Type: text/plain\r\n\r\n ";
        strcat(responseWithHeader, responseBuffer);

        int actualResSize = size(responseWithHeader);

        write(clientfd, responseWithHeader, actualResSize);

        free((void *) responseBuffer);

    CLOSE:
        free((void *) buff);
        free((void *) path);
        
        close(clientfd);

        printf("Response sent 👍\n\n\n");
        fflush(stdout);
    }

    close(sockfd);

    return 0;
}
#define _GNU_SOURCE
#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

//function to check file type
char file_type(char *name){
   //will cut off name until it only leaves file type
   char *dot = strrchr(name, '.');
   if (dot != NULL){
       if (strcmp(dot, ".html") == 0)
            return "text/html";
   }
   return "Uknown file type";

}



void handle_request(int nfd)
{
   FILE *network = fdopen(nfd, "r");
   char *line = NULL;
   size_t size;
   ssize_t num;

   //error messages
   char *error404 = "HTTP/1.0 400 Bad Request";
   char *error500 = "HTTP/1.0 500 Internal Error";

   if (network == NULL)
   {
      perror("fdopen");
      close(nfd);
      return;
   }
   char request[100]; //input
   char type[10]; //GET command
   char *fname[50]; //file name
   char protocol[50];
   printf("Enter command\n");
   if (fgets(request, sizeof(request), stdin) == NULL){
      write(nfd, error404, strlen(error404));
   }
   //assign values by parsing. If not exactly 3, then it is a bad request
   if (sscanf(request, "%s %s %s", type, fname, protocol) != 3){
      write(nfd, error404, strlen(error404));
      fclose(network);
   }
   //remove / from file name
   if (fname[0] == '/') {
        memmove(fname, fname + 1, strlen(fname));
   }

   //check if type is GET
   if (strcmp(type, "GET") != 0){
      write(nfd, error404, strlen(error404));
      fclose(network);
   }

   //open file
   FILE *file = fopen(fname, "r");
   //checks if file exists
   if (file == NULL){
      write(nfd, error404, strlen(error404));
      fclose(network);
   }
   //build structure for file
   struct stat finfo;
   if (stat(fname, &finfo) == -1){
      perror("Error getting info\n");
      return 1;
   }
   //begin building header
   char header[300];
   const char *content_type = get_content_type(fname);
   snprintf(header, sizeof(header), "HTTP/1.0 200 OK\r\n Content-Type: %s\r\n Content-Length: %ld\r\n \r\n", content_type, finfo.st_size );
   //write header to client
   write(nfd, header, strlen(header));

   //get contents of file and then write to client
   int read;
   size_t fsize = 0;
   while ((read = getline(&line, &fsize, fname)) != -1){
      if (write(nfd, line, fsize) == -1){
         perror("write");
      }

   }
   free(line);
   fclose(file);






   fclose(network);
}

void run_service(int fd)
{
   while (1)
   {
      int nfd = accept_connection(fd);
      if (nfd != -1)
      {
         printf("Connection established\n");
         handle_request(nfd);
         printf("Connection closed\n");
      }
   }
}



int main(int argc, char* argv[]){

    

    //checks if at least one command is given
    if (argc != 2){
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    //convert the number in command from string to number
    int port = atoi(argv[1]);

    //make sure port number is between 1024 and 65535
    if (port < 1024 || port > 65535){
        printf("Please enter number between 1024 and 65535\n");
        return 1;
    }

    //create file descriptor
    int fd = create_service(port);
    //check if create service is successful
    if (fd == -1){
        perror(0);
        exit(1);
   }
   run_service(fd);
   

    return 0;
}
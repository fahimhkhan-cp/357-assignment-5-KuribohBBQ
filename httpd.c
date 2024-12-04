#define _GNU_SOURCE
#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
void handle_request(int nfd)
{
   FILE *network = fdopen(nfd, "r");
   char *line = NULL;
   size_t size;
   ssize_t num;

   if (network == NULL)
   {
      perror("fdopen");
      close(nfd);
      return;
   }

   while ((num = getline(&line, &size, network)) >= 0)
   {
      printf("Message received from client: %s", line);
      // MODIFICATION: Use write() to send the data back to the client
      if (write(nfd, line, num) == -1)
      // MODIFICATION: (Optional) Standard error check
      {
         perror("write");
         break;
      }
   }

   free(line);
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



void limit_fork(rlim_t max_procs)
{
    struct rlimit rl;
    if (getrlimit(RLIMIT_NPROC, &rl))
    {
        perror("getrlimit");
        exit(-1);
    }
    rl.rlim_cur = max_procs;
    if (setrlimit(RLIMIT_NPROC, &rl))
    {
        perror("setrlimit");
        exit(-1);
    }
}


int main(int argc, char* argv[]){

    //limit forks
    limit_fork(300);
    

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
   //will be used for forks
   pid_t p;
   

    return 0;
}
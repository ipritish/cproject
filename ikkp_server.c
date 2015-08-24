#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<errno.h>
#include<signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

int listener_d;
int *client_number;

typedef struct connection_handler
{
    int id;
    int connect_id;
}con_handle;

con_handle *handler;
con_handle *head;

void handle_shutdown(int sig)
{
    if(listener_d)
        close(listener_d);
    
    fprintf(stderr,"Bye!\n");
    munmap(client_number, sizeof *client_number);
    exit(0);
}

int catch_signal(int sig, void (*handler)(int))
{
    struct sigaction action;
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    return sigaction (sig, &action, NULL);
}

void error (char *msg)
{
    fprintf(stderr,"%s: %s\n",msg,strerror(errno));
    exit(1);
}

int read_in (int socket, char *buf, int len)
{
    char *s = buf;
    int slen = len;
    int c = recv(socket,s,slen,0);
    while ((c>0) && (s[c-1] != '\n'))
    {
        s += c;
        slen -= c;
        c = recv(socket,s,slen,c);
    }

    if (c < 0)
        return c;
    else if (c == 0)
        buf[0] = '\0';
    else
        s[c-1] = '\0';

    return len - slen;
}

int open_listener_socket()
{
    int s = socket(PF_INET,SOCK_STREAM,0);
    if (s == -1)
        error("Can't open socket");
    return s;
}

void bind_to_port (int socket, int port)
{
    struct sockaddr_in name;
    name.sin_family = PF_INET;
    name.sin_port = (in_port_t) htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    int reuse = 1;
    if (setsockopt(socket,SOL_SOCKET,SO_REUSEADDR, (char *)&reuse,sizeof(int)) == -1)
        error("Can't reuse the port");
    int c = bind(socket,(struct sockaddr *)&name, sizeof(name));
    if(c == -1)
        error("Can't bind to socket");
}

int say (int socket , char *s)
{
    int result = send(socket,s,strlen(s),0);
    if (result == -1)
        fprintf(stderr,"%s: %s\n","Error talking to client",strerror(errno));
    return result;
}

int main(int argc, char *argv[])
{
    //init();
    client_number = (int *)mmap(NULL, sizeof *client_number,PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0);
    head = (struct connection_handler *)mmap(NULL, (sizeof *handler)*10,PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0); 
    handler = head;
    *client_number = 0;
    if (catch_signal(SIGINT,handle_shutdown) == -1)
        error("Can't set interrupt handler");
    listener_d = open_listener_socket();
    bind_to_port(listener_d,30000);
    if (listen(listener_d,10) == -1)
        error("can't listen");
    struct sockaddr_storage client_addr;
    unsigned int address_size = sizeof(client_addr);
    puts("Waiting for connection...");
    char buf[255];
    while(1)
    {
	
        int connect_d = accept(listener_d,(struct sockaddr *)&client_addr,&address_size);
	printf("connect_id %d\n",connect_d);
	printf("listener_id %d\n",listener_d);
	fflush(stdout);
        if (connect_d == -1)
            error("Can't open secondary socket");
        if (!fork())
        {
            close(listener_d);
	    //int temp = *client_number;
	    //temp++;
            //*client_number = temp;
            if (*client_number < 10)
	    {
		handler->id = *client_number;
		handler->connect_id = connect_d;
	    }
	    int temp = *client_number;
            temp++;
            *client_number = temp;
	    handler = handler++;
            //client_number++;
	    //printf("Client_Id %d",*client_number);
	    fflush(stdout);
            if (say(connect_d,"Hello to Chat World!\r\n>") != -1)
            {
		while (strncmp(buf,"end",3))
		{
			int i = 0;
			
			read_in(connect_d,buf,sizeof(buf));
			//flush();
			con_handle *temp_head;
			for (i=0;i<*client_number;i++)
			{
			    //if (!(con_handler[i].connect_id == -1))
			    temp_head = head;
			    //printf("\ntalking to %d",i);
			    //fflush(stdout);
			    say(handler->connect_id,strcat(buf,"\r\n>"));
			    //printf("\nConnectionId is %d\n",head->connect_id);
			    head++;
			    //if (head->id == -1)
			//	break;
			}
			head = temp_head;
			//fflush();
		}
                
            }
            close(connect_d);
            exit(0);
        }
	//munmap(client_number, sizeof *client_number);
        //close(connect_d);
    }

    munmap(client_number, sizeof *client_number);
    
    return 0;
}


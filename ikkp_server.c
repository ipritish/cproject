#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<errno.h>
#include<signal.h>

int listener_d;
int client_number = 0;

struct connection_handler
{
    int id;
    int connect_id;
}con_handler[10];

int init (void)
{	
    int counter;
    for (counter = 0 ; counter<10 ; counter ++)
    {
	con_handler[counter].id = -1;
	con_handler[counter].connect_id = -1;
    }
    return 0;
}
void handle_shutdown(int sig)
{
    if(listener_d)
        close(listener_d);
    
    fprintf(stderr,"Bye!\n");
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
    init();
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
        if (connect_d == -1)
            error("Can't open secondary socket");
        if (!fork())
        {
            close(listener_d);
            client_number++;
            if (client_number < 10)
	    {
		con_handler[client_number].id = client_number;
		con_handler[client_number].connect_id = connect_d;
	    }
            //client_number++;
	    printf("Client_Id %d",client_number);
	    fflush(stdout);
            if (say(connect_d,"Internet Knock-Knock Protocol Server\r\nVersion 1.0\r\nKnock!Knock!\r\n>") != -1)
            {
		while (strncmp(buf,"end",3))
		{
			int i = 0;
			
			read_in(connect_d,buf,sizeof(buf));
			//flush();
			for (i=0;i<10;i++)
			{
			    if (!(con_handler[i].connect_id == -1))
			    say(con_handler[i].connect_id,strcat(buf,"\r\n>"));
			}
			//fflush();
		}
                
            }
            close(connect_d);
            exit(0);
        }
        close(connect_d);
    }
    
    return 0;
}


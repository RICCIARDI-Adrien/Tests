/** @file Main.c
 * Listen for clients on several ports.
 * @author Adrien RICCIARDI
 */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** First server network port. */
#define MAIN_PORT_FIRST_SERVER 10000
/** Second server network port. */
#define MAIN_PORT_SECOND_SERVER 20000
/** Third server network port. */
#define MAIN_PORT_THIRD_SERVER 30000

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
int main(void)
{
	int Socket_First_Server, Socket_Second_Server, Socket_Third_Server, Socket, Socket_Client, Result, Server_Identifier;
	struct sockaddr_in Address;
	fd_set Set_Read;
	socklen_t Address_Size;
	
	// Create first server socket
	Socket_First_Server = socket(AF_INET, SOCK_STREAM, 0);
	if (Socket_First_Server == -1)
	{
		perror("Failed to create first server socket");
		return EXIT_FAILURE;
	}
	
	// Create second server socket
	Socket_Second_Server = socket(AF_INET, SOCK_STREAM, 0);
	if (Socket_Second_Server == -1)
	{
		perror("Failed to create second server socket");
		return EXIT_FAILURE;
	}
	
	// Create third server socket
	Socket_Third_Server = socket(AF_INET, SOCK_STREAM, 0);
	if (Socket_Third_Server == -1)
	{
		perror("Failed to create third server socket");
		return EXIT_FAILURE;
	}
	
	// Bind first server
	Address.sin_family = AF_INET;
	Address.sin_port = htons(MAIN_PORT_FIRST_SERVER);
	Address.sin_addr.s_addr = INADDR_ANY;
	if (bind(Socket_First_Server, (const struct sockaddr *) &Address, sizeof(Address)) == -1)
	{
		perror("Failed to bind first server socket");
		return EXIT_FAILURE;
	}
	
	// Bind second server
	Address.sin_port = htons(MAIN_PORT_SECOND_SERVER);
	if (bind(Socket_Second_Server, (const struct sockaddr *) &Address, sizeof(Address)) == -1)
	{
		perror("Failed to bind second server socket");
		return EXIT_FAILURE;
	}
	
	// Bind third server
	Address.sin_port = htons(MAIN_PORT_THIRD_SERVER);
	if (bind(Socket_Third_Server, (const struct sockaddr *) &Address, sizeof(Address)) == -1)
	{
		perror("Failed to bind third server socket");
		return EXIT_FAILURE;
	}
	
	// Tell first server to listen for clients
	if (listen(Socket_First_Server, 1) == -1)
	{
		perror("Failed to listen first server socket");
		return EXIT_FAILURE;
	}
	
	// Tell second server to listen for clients
	if (listen(Socket_Second_Server, 1) == -1)
	{
		perror("Failed to listen second server socket");
		return EXIT_FAILURE;
	}
	
	// Tell third server to listen for clients
	if (listen(Socket_Third_Server, 1) == -1)
	{
		perror("Failed to listen third server socket");
		return EXIT_FAILURE;
	}
	
	// Wait for clients to connect
	while (1)
	{
		puts("Waiting for a client to connect...");
		
		// Create the set of sockets to monitor
		FD_ZERO(&Set_Read);
		FD_SET(Socket_First_Server, &Set_Read);
		FD_SET(Socket_Second_Server, &Set_Read);
		FD_SET(Socket_Third_Server, &Set_Read);
		Result = select(FD_SETSIZE, &Set_Read, NULL, NULL, NULL);
		if (Result < 0)
		{
			perror("Error in select");
			return EXIT_FAILURE;
		}
		else if (Result == 0)
		{
			puts("WARNING : no client connection detected, retrying. This should not happen.");
			continue;
		}
		
		// Determine the server a client connected to
		if (FD_ISSET(Socket_First_Server, &Set_Read))
		{
			Socket = Socket_First_Server;
			Server_Identifier = 1;
		}
		else if (FD_ISSET(Socket_Second_Server, &Set_Read))
		{
			Socket = Socket_Second_Server;
			Server_Identifier = 2;
		}
		else if (FD_ISSET(Socket_Third_Server, &Set_Read))
		{
			Socket = Socket_Third_Server;
			Server_Identifier = 3;
		}
		else
		{
			puts("WARNING : can't determine the server a client wanted to connect to, retrying. This should not happen.");
			continue;
		}
		
		// Connect with client
		Address_Size = sizeof(Address);
		Socket_Client = accept(Socket, (struct sockaddr *) &Address, &Address_Size);
		if (Socket_Client == -1)
		{
			perror("Failed to accept client socket");
			return EXIT_FAILURE;
		}
		printf("Client connected to server %d. Client port : %d.\n", Server_Identifier, ntohs(Address.sin_port));
	}
	
	return EXIT_SUCCESS;
}

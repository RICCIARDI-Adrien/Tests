/** @file Main.c
 * Listen for clients on several ports.
 * @author Adrien RICCIARDI
 */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

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
	int Socket_First_Server, Socket_Second_Server, Socket_Third_Server;
	struct sockaddr_in Address;
	
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
	
	return EXIT_SUCCESS;
}

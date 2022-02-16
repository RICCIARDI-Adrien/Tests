/** @file Main.c
 * A test application running the Game of Life cellular automaton.
 * @author Adrien RICCIARDI
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <zephyr.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** How many cells on a row. */
#define MAIN_WORLD_WIDTH_CELLS 80
/** How many cells on a column. */
#define MAIN_WORLD_HEIGHT_CELLS 23

/** How many RAM a world requires. */
#define MAIN_WORLD_SIZE_BYTES ((MAIN_WORLD_WIDTH_CELLS * MAIN_WORLD_HEIGHT_CELLS) / 8) // Divide by 8 because cell states are stored on a single bit

/** How many generations are kept to check for hanged worlds. */
#define MAIN_WORLD_GENERATIONS_COUNT 7

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** Useful world generations. */
static unsigned char Main_World_Generations[MAIN_WORLD_GENERATIONS_COUNT][MAIN_WORLD_SIZE_BYTES]; // Current generation is index 0

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Get the specified cell state (dead or alive).
 * @param Pointer_World The world the cell belongs to.
 * @param Row The cell vertical coordinate.
 * @param Column The cell horizontal coordinate.
 * @return 0 if the cell is dead,
 * @return 1 if the cell is alive.
 */
static int MainGetCellState(unsigned char *Pointer_World, int Row, int Column)
{
	int Cells_State, Is_Cell_Alive, Bit_Index;

	// Retrieve the byte the cells are stored in
	Cells_State = Pointer_World[((Row * MAIN_WORLD_WIDTH_CELLS) + Column) >> 3]; // Fast divide by 8 by doing a shift

	// Extract the requested bit
	Bit_Index = Column & 0x07; // Ultrafast modulo 8 operation
	if (Cells_State & (1 << Bit_Index)) Is_Cell_Alive = 1;
	else Is_Cell_Alive = 0;

	return Is_Cell_Alive;
}

/** Set the specified cell state.
 * @param Pointer_World The world the cell belongs to.
 * @param Row The cell vertical coordinate.
 * @param Column The cell horizontal coordinate.
 * @param Is_Cell_Alive Set to 1 if the cell is alive, set to 0 if the cell is dead.
 */
static void MainSetCellState(unsigned char *Pointer_World, int Row, int Column, int Is_Cell_Alive)
{
	int Cells_State, Bit_Index, Cells_State_Index;

	// Retrieve the byte the cells are stored in
	Cells_State_Index = ((Row * MAIN_WORLD_WIDTH_CELLS) + Column) >> 3; // Fast divide by 8 by doing a shift
	Cells_State = Pointer_World[Cells_State_Index];

	// Set the new cell state
	Bit_Index = Column & 0x07; // Ultrafast modulo 8 operation
	if (Is_Cell_Alive) Cells_State |= 1 << Bit_Index;
	else Cells_State &= ~(1 << Bit_Index);
	Pointer_World[Cells_State_Index] = Cells_State;
}

/** Generate a random world. */
static inline void MainGenerateWorld(void)
{
	int Row, Column, Is_Cell_Alive;

	// Reset previous generations to be sure they won't match with the newly generated world
	for (Row = 1; Row < MAIN_WORLD_GENERATIONS_COUNT; Row++) memset(Main_World_Generations[Row], 0, MAIN_WORLD_SIZE_BYTES); // Recycle "Row" variable

	// Generate a new current world
	for (Row = 0; Row < MAIN_WORLD_HEIGHT_CELLS; Row++)
	{
		for (Column = 0; Column < MAIN_WORLD_WIDTH_CELLS; Column++)
		{
			// There is a chance out of 7 to get an alive cell
			if ((rand() % 7) == 2) Is_Cell_Alive = 1;
			else Is_Cell_Alive = 0;

			MainSetCellState(Main_World_Generations[0], Row, Column, Is_Cell_Alive);
		}
	}
}

/** Apply game of life rules on the current generation and generate the next generation (still stored in Main_World_Current_Generation array). */
static inline void MainComputeNextGeneration(void)
{
	static unsigned char World_Next_Generation[MAIN_WORLD_SIZE_BYTES];
	int Row, Column, Neighbor_Cells_Count, Is_Cell_Alive;
	unsigned char *Pointer_World_Current_Generation = Main_World_Generations[0];

	// Save previous generations (replace the oldest one by the old one and so on)
	for (Row = MAIN_WORLD_GENERATIONS_COUNT - 1; Row > 0; Row--) memcpy(Main_World_Generations[Row], Main_World_Generations[Row - 1], MAIN_WORLD_SIZE_BYTES); // Recycle "Row" variable

	// Handle all current world cells
	for (Row = 0; Row < MAIN_WORLD_HEIGHT_CELLS; Row++)
	{
		for (Column = 0; Column < MAIN_WORLD_WIDTH_CELLS; Column++)
		{
			// Compute how many alive neighbors a cell has
			Neighbor_Cells_Count = 0;
			// North-west from the current cell
			if ((Row > 0) && (Column > 0)) Neighbor_Cells_Count += MainGetCellState(Pointer_World_Current_Generation, Row - 1, Column - 1);
			// North from the current cell
			if (Row > 0) Neighbor_Cells_Count += MainGetCellState(Pointer_World_Current_Generation, Row - 1, Column);
			// North-east from the current cell
			if ((Row > 0) && (Column < MAIN_WORLD_WIDTH_CELLS - 1)) Neighbor_Cells_Count += MainGetCellState(Pointer_World_Current_Generation, Row - 1, Column + 1);
			// West from the current cell
			if (Column > 0) Neighbor_Cells_Count += MainGetCellState(Pointer_World_Current_Generation, Row, Column - 1);
			// East from the current cell
			if (Column < MAIN_WORLD_WIDTH_CELLS - 1) Neighbor_Cells_Count += MainGetCellState(Pointer_World_Current_Generation, Row, Column + 1);
			// South-west from the current cell
			if ((Row < MAIN_WORLD_HEIGHT_CELLS - 1) && (Column > 0)) Neighbor_Cells_Count += MainGetCellState(Pointer_World_Current_Generation, Row + 1, Column - 1);
			// South from the current cell
			if (Row < MAIN_WORLD_HEIGHT_CELLS - 1) Neighbor_Cells_Count += MainGetCellState(Pointer_World_Current_Generation, Row + 1, Column);
			// South-east from the current cell
			if ((Row < MAIN_WORLD_HEIGHT_CELLS - 1) && (Column < MAIN_WORLD_WIDTH_CELLS - 1)) Neighbor_Cells_Count += MainGetCellState(Pointer_World_Current_Generation, Row + 1, Column + 1);

			// A cell is born if there are exactly 3 alive neighbors
			if (Neighbor_Cells_Count == 3) Is_Cell_Alive = 1;
			// Cell stays in its current state if it has exactly 2 alive neighbors
			else if (Neighbor_Cells_Count == 2) Is_Cell_Alive = MainGetCellState(Pointer_World_Current_Generation, Row, Column);
			// In all other case an alive cell dies, or a yet-died cell does not return to life
			else Is_Cell_Alive = 0;

			// Store new cell state
			MainSetCellState(World_Next_Generation, Row, Column, Is_Cell_Alive);
		}
	}

	// Next generation is fully generated, copy it to the current one
	memcpy(Pointer_World_Current_Generation, World_Next_Generation, MAIN_WORLD_SIZE_BYTES);
}

/** Check if the current world can evolve or if it is stuck in an infinite loop.
 * @return 0 if the world can't evolve anymore,
 * @return 1 if the world is still evolving.
 */
static inline int MainIsWorldEvolving(void)
{
	// Checking if current and previous generations are the same allows to detect a completely hanged world (this also works by checking with current and before last generations, and allows to display the stuck world). Checking current against before last generations allows to detect 2-period oscillators
	if (memcmp(Main_World_Generations[0], Main_World_Generations[2], MAIN_WORLD_SIZE_BYTES) == 0) return 0;

	// Three-period oscillators and 2-period ones are synchronized only one time out of six, so we have to check the current generation against the older 6th to discard 3-period oscillators
	if (memcmp(Main_World_Generations[0], Main_World_Generations[6], MAIN_WORLD_SIZE_BYTES) == 0) return 0;

	return 1;
}

/** Display the world current generation. */
static inline void MainDisplayWorld(void)
{
	int Cell_Row, Cell_Column;

	// Put cursor on top left screen corner
	printk("\033[H");

	for (Cell_Row = 0; Cell_Row < MAIN_WORLD_HEIGHT_CELLS; Cell_Row++)
	{
		for (Cell_Column = 0; Cell_Column < MAIN_WORLD_WIDTH_CELLS; Cell_Column++)
		{
			// Display the appropriate character according to cell state
			if (MainGetCellState(Main_World_Generations[0], Cell_Row, Cell_Column)) putchar('O');
			else putchar(' ');
		}

		// Go to next line
		putchar('\r');
		putchar('\n');
	}
}

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
void main(void)
{
	srand(time(NULL));

	while (1)
	{
		// Generate a new world
		MainGenerateWorld();

		while (1)
		{
			// Show world state (show it before computing the next generation to see what the generated world looks like)
			MainDisplayWorld();

			// Simulate world
			MainComputeNextGeneration();

			// Generate a new world if this one is stuck
			if (!MainIsWorldEvolving()) break;

			// Wait a bit to let the watchers understand what is happening
			k_busy_wait(250000); // k_msleep() does not work with qemu_x86 for an obscure reason
		}
	}
}

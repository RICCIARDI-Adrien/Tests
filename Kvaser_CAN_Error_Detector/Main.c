/** @file Main.c
 * Use the Kvaser CANlib to get more details about CAN bus errors than the CANKing software provides.
 * @author Adrien RICCIARDI
 */
#include <canlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//-------------------------------------------------------------------------------------------------
// Private types
//-------------------------------------------------------------------------------------------------
/** All detectable CAN transmission errors. See CANlib canMSGERR_XXX documentation for more information. */
typedef enum
{
	CAN_ERROR_ID_HW_OVERRUN,
	CAN_ERROR_ID_SW_OVERRUN,
	CAN_ERROR_ID_STUFF,
	CAN_ERROR_ID_FORM,
	CAN_ERROR_ID_CRC,
	CAN_ERROR_ID_BIT0,
	CAN_ERROR_ID_BIT1,
	CAN_ERROR_IDS_COUNT
} TCANErrorID;

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Return a static string containing the error message description (like strerror()).
 * @param Error_Code The error code to translate to a human-readable message.
 * @return A pointer to a static string containing the message, do not call this function in multithreading context.
 */
static char *GetErrorMessage(canStatus Error_Code)
{
	static char String_Error_Message[256];

	// Try to convert the error code to a string
	if (canGetErrorText(Error_Code, String_Error_Message, sizeof(String_Error_Message)) != canOK) strcpy(String_Error_Message, "Internal error when converting the error code");

	return String_Error_Message;
}

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	canHandle Handle;
	int Channel, Return_Value = EXIT_FAILURE, Reception_Statistics_Counter = 0;
	canStatus Result;
	unsigned int Flags, Data_Length_Code;
	unsigned long Error_Counters[CAN_ERROR_IDS_COUNT] = {0}, Cumulated_Errors_Counter = 0, Received_Valid_Frames_Counter = 0, Received_Valid_Payload_Bytes_Count = 0, Error_Frames_Counter = 0;

	// Check parameters
	if (argc != 2)
	{
		printf("Usage : %s CAN_Channel\n", argv[0]);
		return EXIT_FAILURE;
	}

	// Extract parameters
	if (sscanf(argv[1], "%d", &Channel) != 1)
	{
		printf("Error : the channel parameter must be a number.\n");
		return EXIT_FAILURE;
	}

	// Try to open the requested channel
	canInitializeLibrary();
	Handle = canOpenChannel(Channel, canOPEN_CAN_FD | canOPEN_REQUIRE_INIT_ACCESS);
	if (Handle < 0)
	{
		printf("Error : failed to open channel %d (%s).\n", Channel, GetErrorMessage(Handle));
		return EXIT_FAILURE;
	}

	// Configure the bus parameters
	printf("Configuring bus parameters...\n");
	Result = canSetBusParams(Handle, canBITRATE_1M, 0, 0, 0, 0, 0);
	if (Result != canOK)
	{
		printf("Error : failed to configure classic CAN bus rate (%s).\n", GetErrorMessage(Result));
		goto Exit_Close_Handle;
	}
	Result = canSetBusParamsFd(Handle, canFD_BITRATE_8M_80P, 0, 0, 0);
	if (Result != canOK)
	{
		printf("Error : failed to configure CAN FD bus rate (%s).\n", GetErrorMessage(Result));
		goto Exit_Close_Handle;
	}
	Result = canSetBusOutputControl(Handle, canDRIVER_NORMAL);
	if (Result != canOK)
	{
		printf("Error : failed to configure bus output control (%s).\n", GetErrorMessage(Result));
		goto Exit_Close_Handle;
	}

	// Access to hardware
	printf("Going on bus...\n");
	Result = canBusOn(Handle);
	if (Result != canOK)
	{
		printf("Error : failed to turn bus on (%s).\n", GetErrorMessage(Result));
		goto Exit_Close_Handle;
	}
	
	printf("Waiting for CAN frames. Stop sending frames for more than 10 seconds for the program to automatically exit.\n");
	while (1)
	{
		// Wait for a frame to be received
		Result = canReadWait(Handle, NULL, NULL, &Data_Length_Code, &Flags, NULL, 10000); // Automatically exit program when no frame is received after 10 seconds
		if (Result != canOK)
		{
			if (Result == canERR_TIMEOUT) printf("Timeout while waiting for CAN frames, exiting program.\n");
			else printf("Error : failed to read a CAN frame (%s).\n", GetErrorMessage(Result));
			break;
		}

		// Did any error occurred ?
		if (Flags & canMSG_ERROR_FRAME) Error_Frames_Counter++;
		else if (Flags & canMSGERR_MASK)
		{
			Cumulated_Errors_Counter++;
			if (Flags & canMSGERR_HW_OVERRUN) Error_Counters[CAN_ERROR_ID_HW_OVERRUN]++;
			if (Flags & canMSGERR_SW_OVERRUN) Error_Counters[CAN_ERROR_ID_SW_OVERRUN]++;
			if (Flags & canMSGERR_STUFF) Error_Counters[CAN_ERROR_ID_STUFF]++;
			if (Flags & canMSGERR_FORM) Error_Counters[CAN_ERROR_ID_FORM]++;
			if (Flags & canMSGERR_CRC) Error_Counters[CAN_ERROR_ID_CRC]++;
			if (Flags & canMSGERR_BIT0) Error_Counters[CAN_ERROR_ID_BIT0]++;
			if (Flags & canMSGERR_BIT1) Error_Counters[CAN_ERROR_ID_BIT1]++;
		}
		else
		{
			Received_Valid_Frames_Counter++;
			Received_Valid_Payload_Bytes_Count += Data_Length_Code;
		}

		// Display received frames indicator
		if (Reception_Statistics_Counter < 999) Reception_Statistics_Counter++;
		else
		{
			printf("Received valid frames : %lu, cumulated errors : %lu, error frames : %lu, received valid payload bytes : %lu\n", Received_Valid_Frames_Counter, Cumulated_Errors_Counter, Error_Frames_Counter, Received_Valid_Payload_Bytes_Count);
			Reception_Statistics_Counter = 0;
		}
	}

	// Display statistics
	printf("----------\n"
		"Received valid frames : %lu\n"
		"Received valid payload bytes : %lu\n"
		"Error_Frames_Counter = %lu\n"
		"Cumulated errors : %lu\n"
		"Hardware overruns : %lu\n"
		"Software overruns : %lu\n"
		"Stuffing errors : %lu\n"
		"Form errors : %lu\n"
		"CRC errors : %lu\n"
		"Bit 0 errors : %lu\n"
		"Bit 1 errors : %lu\n",
		Received_Valid_Frames_Counter,
		Received_Valid_Payload_Bytes_Count,
		Error_Frames_Counter,
		Cumulated_Errors_Counter,
		Error_Counters[CAN_ERROR_ID_HW_OVERRUN],
		Error_Counters[CAN_ERROR_ID_SW_OVERRUN]++,
		Error_Counters[CAN_ERROR_ID_STUFF],
		Error_Counters[CAN_ERROR_ID_FORM],
		Error_Counters[CAN_ERROR_ID_CRC],
		Error_Counters[CAN_ERROR_ID_BIT0],
		Error_Counters[CAN_ERROR_ID_BIT1]);

	// Turn bus off
	Result = canBusOff(Handle);
	if (Result != canOK) printf("Error : failed to turn bus off (%s).\n", GetErrorMessage(Result));

Exit_Close_Handle:
	canClose(Handle);
	return Return_Value;
}

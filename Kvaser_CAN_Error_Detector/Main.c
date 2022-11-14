/** @file Main.c
 * Use the Kvaser CANlib to get more details about CAN bus errors than the CANKing software provides.
 * @author Adrien RICCIARDI
 */
#include <canlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
	int Channel, Return_Value = EXIT_FAILURE;
	canStatus Result;
	unsigned char Payload[64];
	unsigned int Data_Length, Flags, i;

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
	
	printf("Waiting for CAN frames ...\n");
	while (1)
	{
		// Wait for a frame to be received
		Result = canReadWait(Handle, NULL, Payload, &Data_Length, &Flags, NULL, 0xFFFFFFFF);
		if (Result != canOK)
		{
			printf("Error : failed to read a CAN frame (%s).\n", GetErrorMessage(Result));
			goto Exit_Turn_Bus_Off;
		}

		printf("TEST %u\n", Data_Length);
		printf("FLAGS : 0x%08X.\n", Flags);
		for (i = 0; i < Data_Length; i++) printf("%d = %02X\n", i, Payload[i]);
		break;
	}

Exit_Turn_Bus_Off:
	Result = canBusOff(Handle);
	if (Result != canOK) printf("Error : failed to turn bus off (%s).\n", GetErrorMessage(Result));

Exit_Close_Handle:
	canClose(Handle);
	return Return_Value;
}

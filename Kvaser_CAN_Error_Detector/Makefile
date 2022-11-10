CC = gcc
CFLAGS = -W -Wall

BINARY = can-error-detector
CAN_SDK_PATH = "/cygdrive/c/Program Files (x86)/Kvaser/Canlib"

all:
	$(CC) $(CFLAGS) Main.c -I $(CAN_SDK_PATH)/INC -L $(CAN_SDK_PATH)/Lib/x64 -l canlib32 -o $(BINARY)

clean:
	rm -f $(BINARY)
	rm -f $(BINARY).exe

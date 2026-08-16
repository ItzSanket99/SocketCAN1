CC = gcc
CFLAGS = -Wall -Wextra

INCLUDE = -Iinclude

all: vehicle_ecu dashboard_ecu logger_ecu can_fd_demo

vehicle_ecu:
	$(CC) $(CFLAGS) $(INCLUDE) \
	src/vehicle_ecu/vehicle_ecu.c \
	-o vehicle_ecu

dashboard_ecu:
	$(CC) $(CFLAGS) $(INCLUDE) \
	src/dashboard_ecu/dashboard_ecu.c \
	-o dashboard_ecu

logger_ecu:
	$(CC) $(CFLAGS) $(INCLUDE) \
	src/logger_ecu/logger_ecu.c \
	-o logger_ecu

can_fd_demo:
	$(CC) $(CFLAGS) $(INCLUDE) \
	src/can_fd/can_fd_demo.c \
	-o can_fd_demo

clean:
	rm -f vehicle_ecu
	rm -f dashboard_ecu
	rm -f logger_ecu
	rm -f can_fd_demo

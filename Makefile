#Compiler
CC = gcc

#Compiler flags
CFLAGS = -Wall -Wextra -g -pthread -I./include

#Source files
SRCS = src/main.c src/santa.c

#Default rule
all: quiet verbose

quiet:
	$(CC) $(CFLAGS) $(SRCS) -o project_quiet
	@echo "Built project_quiet successfully!"

verbose:
	$(CC) $(CFLAGS) -DENABLE_VERBOSE $(SRCS) -o project_verbose
	@echo "Built project_verbose successfully!"

clean:
	rm -f project_quiet project_verbose
	@echo "Cleaned up executables successfully!"

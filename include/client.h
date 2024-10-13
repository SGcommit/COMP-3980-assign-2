#ifndef CLIENT_H
#define CLIENT_H

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define STRING_SIZE 256
#define CONVERSION_TYPE_SIZE 10
#define FIFO_PERMISSIONS 0666

typedef struct
{
    char string[STRING_SIZE];
    char conversionType[CONVERSION_TYPE_SIZE];    // "upper", "lower", "none"
} ClientRequest;

// File paths for FIFOs
#define INPUT_FIFO "/tmp/input_fifo"
#define OUTPUT_FIFO "/tmp/output_fifo"

// Struct to hold the client request

// Function to print the usage message
void print_usage(const char *prog_name);

#endif    // CLIENT_H

#include "../include/client.h"

// Function to print the usage message
noreturn void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s -s <string> -t <conversion type: upper, lower, none>\n", prog_name);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int           opt;
    const char   *inputString    = NULL;
    const char   *conversionType = NULL;
    int           input_fd;
    int           output_fd;
    char          processed_string[STRING_SIZE];
    ClientRequest request;
    ssize_t       bytes_read;

    // Initialize processed_string
    memset(processed_string, 0, sizeof(processed_string));

    // Parse command-line arguments using getopt
    while((opt = getopt(argc, argv, "s:t:")) != -1)
    {
        switch(opt)
        {
            case 's':
                inputString = optarg;
                break;
            case 't':
                conversionType = optarg;
                break;
            default:
                print_usage(argv[0]);
        }
    }

    // Check if required arguments are provided
    if(inputString == NULL || conversionType == NULL)
    {
        print_usage(argv[0]);
    }

    // Validate conversion type
    if(strcmp(conversionType, "upper") != 0 && strcmp(conversionType, "lower") != 0 && strcmp(conversionType, "none") != 0)
    {
        fprintf(stderr, "Invalid conversion type. Please choose from 'upper', 'lower', or 'none'.\n");
        exit(EXIT_FAILURE);
    }

    // Create and populate ClientRequest struct
    strncpy(request.string, inputString, sizeof(request.string) - 1);
    strncpy(request.conversionType, conversionType, sizeof(request.conversionType) - 1);

    // Open input FIFO
    input_fd = open(INPUT_FIFO, O_WRONLY | O_CLOEXEC);
    if(input_fd == -1)
    {
        perror("Error opening input FIFO");
        exit(EXIT_FAILURE);
    }

    // Write request to input FIFO
    if(write(input_fd, &request, sizeof(ClientRequest)) == -1)
    {
        perror("Error writing to input FIFO");
        close(input_fd);
        exit(EXIT_FAILURE);
    }
    close(input_fd);

    // Open output FIFO to receive processed data
    output_fd = open(OUTPUT_FIFO, O_RDONLY | O_CLOEXEC);
    if(output_fd == -1)
    {
        perror("Error opening output FIFO");
        exit(EXIT_FAILURE);
    }

    // Read processed string from output FIFO
    bytes_read = read(output_fd, processed_string, sizeof(processed_string) - 1);
    if(bytes_read == -1)
    {
        perror("Error reading from output FIFO");
        close(output_fd);
        exit(EXIT_FAILURE);
    }

    processed_string[bytes_read] = '\0';    // Null-terminate the string
    close(output_fd);

    // Print processed string
    printf("Processed string: %s\n", processed_string);

    return 0;
}

#include "../include/server.h"

static volatile sig_atomic_t exit_flag = 0;    // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

// Signal handler to terminate the server
void custom_signal_handler(int signum)    // Renamed handler function
{
    (void)signum;     // Mark parameter as unused
    exit_flag = 1;    // Set exit flag
}

// Function to process each client request
void *process_request(void *arg)
{
    ClientRequest *request             = (ClientRequest *)arg;
    char           result[STRING_SIZE] = {0};
    int            i;
    int            output_fd;

    // Copy the input string to result
    strncpy(result, request->string, sizeof(result) - 1);

    // Process the string based on conversionType
    if(strcmp(request->conversionType, "upper") == 0)
    {
        for(i = 0; result[i]; i++)
        {
            result[i] = (char)toupper(result[i]);
        }
    }
    else if(strcmp(request->conversionType, "lower") == 0)
    {
        for(i = 0; result[i]; i++)
        {
            result[i] = (char)tolower(result[i]);
        }
    }

    // Open the output FIFO
    output_fd = open(OUTPUT_FIFO, O_WRONLY | O_CLOEXEC);
    if(output_fd == -1)
    {
        perror("Error opening output FIFO");
        free(request);
        pthread_exit(NULL);
    }

    // Write the result to the output FIFO
    if(write(output_fd, result, sizeof(result)) == -1)
    {
        perror("Error writing to output FIFO");
        close(output_fd);
        free(request);
        pthread_exit(NULL);
    }

    close(output_fd);
    free(request);    // Free the dynamically allocated request
    pthread_exit(NULL);
}

int main(void)
{
    pthread_t      thread;
    ClientRequest *request;

#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#endif

    struct sigaction sa;
    sa.sa_handler = custom_signal_handler;    // Use the renamed signal handler
    sa.sa_flags   = 0;                        // No special flags
    sigemptyset(&sa.sa_mask);                 // No additional signals to block
#if defined(__clang__)
    #pragma clang diagnostic pop
#endif
    // Set up the sigaction structure for SIGINT

    // Handle SIGINT (Ctrl-C) to gracefully terminate the server
    if(sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("Error setting up signal handler");
        return EXIT_FAILURE;
    }

    printf("Server is running...\n");

    while(1)    // Loop indefinitely
    {
        int input_fd;
        if(exit_flag)
        {    // Check for exit condition
            break;
        }
        // Open the input FIFO for reading
        input_fd = open(INPUT_FIFO, O_RDONLY | O_CLOEXEC);
        if(input_fd == -1)
        {
            if(errno == EINTR)    // If interrupted by signal
            {
                continue;    // Just retry opening the FIFO if interrupted
            }
            perror("Error opening input FIFO");
            continue;
        }

        // Allocate memory for the request
        request = (ClientRequest *)malloc(sizeof(ClientRequest));
        if(request == NULL)
        {
            perror("Error allocating memory for ClientRequest");
            close(input_fd);
            continue;
        }

        // Read the request from the input FIFO
        if(read(input_fd, request, sizeof(ClientRequest)) == -1)
        {
            perror("Error reading from input FIFO");
            free(request);
            close(input_fd);
            continue;
        }
        close(input_fd);

        // Create a thread to process the request
        if(pthread_create(&thread, NULL, process_request, (void *)request) != 0)
        {
            perror("Error creating thread");
            free(request);
            continue;
        }

        // Detach the thread to allow it to clean up after itself
        if(pthread_detach(thread) != 0)
        {
            perror("Error detaching thread");
            free(request);
            continue;
        }
    }

    // Cleanup
    unlink(INPUT_FIFO);
    unlink(OUTPUT_FIFO);    // Cleanup FIFOs on termination

    printf("Server terminated gracefully.\n");
    return 0;
}

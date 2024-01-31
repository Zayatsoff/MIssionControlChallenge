#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Swap from and to Big Endians - uint16_t
uint16_t swap_uint16(uint16_t val)
{
    return (val << 8) | (val >> 8);
}

// Swap from and to Big Endians - uint32_t
uint32_t swap_uint32(uint32_t val)
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
    return (val << 16) | (val >> 16);
}

// Scale UINT16_MAX to 100000 (cm)
float scaleMeasure(uint16_t value)
{
    float height = 100000.0f * (float)value / (float)65535;
    return height;
}

int main()
{
    // Variables
    int port = 12778;
    int serverPort = 12777;
    int sockfd;
    int serverSockfd;
    struct sockaddr_in serverAddr;
    struct sockaddr_in hssAddr;
    unsigned char buffer[4096];
    unsigned char send_buffer[10]; // assumed message is 2 bytes header 4 bytes header and 4 bytes payload
    bool isExit = false;
    socklen_t addr_size;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    serverSockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        exit(EXIT_FAILURE);
    }
    memset(&hssAddr, '\0', sizeof(hssAddr));
    memset(&serverAddr, '\0', sizeof(serverAddr));

    // HSS address assign
    hssAddr.sin_family = AF_INET;
    hssAddr.sin_port = htons(port);
    hssAddr.sin_addr.s_addr = INADDR_ANY;

    // Server address assign
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&hssAddr, sizeof(hssAddr)) < 0)
    {
        printf("--[ERROR]Could not bind to port %d--\n", port);
        exit(EXIT_FAILURE);
    }

    printf("Begin transmission. \n");
    do
    {
        int n = recvfrom(sockfd, buffer, sizeof(buffer), MSG_WAITALL, NULL, NULL); // Recieve data from bus
        int count = 0;
        while (count < n / 2)
        {
            uint16_t recieve = *((uint16_t *)buffer + count);
            uint16_t swapped = swap_uint16(recieve);
            count++;
            if (swapped == 0xaa01) // If type is found
            {
                uint16_t sensors[3];
                float readings[3];
                uint16_t header = swapped;
                uint32_t time = *((uint32_t *)buffer + count);
                time = swap_uint32(time);
                count += 2;

                // Add raw measurements to sensors array and converted measurements to readings array
                for (int i = 0; i < 3; i++)
                {
                    sensors[i] = *((uint16_t *)buffer + count);
                    sensors[i] = swap_uint16(sensors[i]);
                    readings[i] = scaleMeasure(sensors[i]);
                    count++;
                }

                printf("Header: %x Time: %d Sensor 1: %.2f Sensor 2: %.2fcm Sensor 3: %.2fcm\n", header, time, readings[0], readings[1], readings[2]);

                // Find average of all measurements
                float avg = (readings[0] + readings[1] + readings[2]) / 3.0f;
                memset(send_buffer, '\0', 10);

                // Landing event
                if (avg < 40.0)
                {
                    // Swap back to Big Endians
                    uint16_t header_big = swap_uint16(0xaa11);
                    uint32_t time_big = swap_uint32(time);
                    memcpy(send_buffer, &header_big, 2);
                    memcpy(send_buffer + 2, &time_big, 4);
                    printf("The Lunar lander has landed!\n");
                    isExit = true; // Exit do loop
                }
                else // Height message
                {
                    // Swap back to Big Endians
                    uint16_t header_big = swap_uint16(0xaa31);
                    uint32_t time_big = swap_uint32(time);
                    uint32_t avg_big = swap_uint32((uint32_t)avg);
                    memcpy(send_buffer, &header_big, 2);
                    memcpy(send_buffer + 2, &time_big, 4);
                    memcpy(send_buffer + 6, &avg_big, 4);
                }

                // Send ENGINE_CUTOFF or HEIGHT message
                int clientResult = sendto(serverSockfd, send_buffer, 10, 0, (sockaddr *)&serverAddr, sizeof(serverAddr));
                if (clientResult == -1)
                {
                    printf("--[ERROR]Could not send to port %d--\n", serverPort);
                    exit(EXIT_FAILURE);
                }
            }
        }
    } while (!isExit);

    return 0;
}

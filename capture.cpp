#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <cmath>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

////////////////////////////////////////////////////////

#define MESSAGE_SIZE 17
static uint8_t sBuffer[MESSAGE_SIZE*1024];

int main(int argc, const char* argv[]) {
    const char* serialPort = (argc >= 2) ? argv[1] : "/dev/ttyUSB0";
    int fd = open(serialPort, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Error %s\n", strerror(errno));
        return 1;
    }
    struct termios tty;

    if(tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return 1;
    }
    tty.c_cflag &= ~PARENB; // Clear parity bit
    tty.c_cflag &= ~CSTOPB; // Clear stop field
    tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 4000000
    cfsetispeed(&tty, B4000000);
    cfsetospeed(&tty, B4000000);

    // Save tty settings, also checking for error
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        fprintf(stderr, "Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    int i = 0;
    while (i < sizeof(sBuffer)/MESSAGE_SIZE) {
        size_t bytes = read(fd, &sBuffer[i*MESSAGE_SIZE], MESSAGE_SIZE);
        if (bytes == MESSAGE_SIZE) {
            for (int q = 0; q < MESSAGE_SIZE; q++) {
                printf("%02X ", sBuffer[i*MESSAGE_SIZE+q]);
            }
            printf("\n");
            i++;
        }
    }
    printf("DONE\n");
    FILE* outfd = fopen("capture01.bin", "wb+");
    if (outfd != nullptr) {
        fwrite(sBuffer, sizeof(sBuffer), 1, outfd);
        fclose(outfd);
    } else {
        fprintf(stderr, "Error %s\n", strerror(errno));
        return 1;
    }
    return 0;
}
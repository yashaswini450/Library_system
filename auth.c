#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <fcntl.h>

#include "auth.h"

void lock_file(int fd, short type) { // to get lock
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = type;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0; // Lock the whole file
    fcntl(fd, F_SETLKW, &lock);
}
// NOTE: record locking possible but useful only when DB entries are indexed i.e. a separate .ndx file

void format_time(time_t rawtime, char *buffer, size_t buffer_size) {
    struct tm *timeinfo = localtime(&rawtime);
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", timeinfo);
}

char* formatHeading(char *text) {

    int textLength = strlen(text);
    int totalWidth = 52;
    int paddingLeft = (totalWidth - textLength - 2) / 2; // Padding for left side (2 extra for asterisks)
    int paddingRight = totalWidth - paddingLeft - textLength - 2; // Padding for right side

    // Allocate memory for the formatted string
    // 3 lines of width totalWidth, plus null terminator
    char* formattedText = (char*)malloc((totalWidth * 3 + 1) * sizeof(char));
    if (formattedText == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    // Construct the formatted string
    // Top border
    memset(formattedText, '*', totalWidth);
    formattedText[totalWidth] = '\n';
    formattedText[totalWidth + 1] = '\0';

    // Line with text
    strcat(formattedText, "*");
    for (int i = 0; i < paddingLeft; i++) strcat(formattedText, " ");
    strcat(formattedText, text);
    for (int i = 0; i < paddingRight; i++) strcat(formattedText, " ");
    strcat(formattedText, "*\n");

    // Bottom border
    memset(formattedText + strlen(formattedText), '*', totalWidth);
    formattedText[strlen(formattedText) + totalWidth] = '\n';
    formattedText[strlen(formattedText) + totalWidth + 1] = '\0';

    return formattedText;
}

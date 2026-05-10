#include "hospitalexception.h"

static int he_len(const char* a) {
    int count = 0;
    if (!a) return 0;
    while (a[count] != '\0') count++;
    return count;
}

hospitalexception::hospitalexception(const char* message) {
    int len = he_len(message);
    if (len > 199) len = 199;
    for (int i = 0; i < len; i++) messages[i] = message[i];
    messages[len] = '\0';
}

char* hospitalexception::what() {
    return messages;
}
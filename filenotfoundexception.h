#pragma once
#include "hospitalexception.h"

class FileNotFoundException : public hospitalexception {
public:
    FileNotFoundException(const char* msg = "Error: Required file could not be opened.");
};
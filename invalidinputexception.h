#pragma once
#include"hospitalexception.h"
class InvalidInputException : public hospitalexception {
public:
    InvalidInputException(const char* msg = "Error: User input failed validation.");
};
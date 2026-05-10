#pragma once
#include "hospitalexception.h"
class InsufficientFundsException : public hospitalexception {
public:
    InsufficientFundsException(const char* msg="Error: Patient balance is insufficient.");
};
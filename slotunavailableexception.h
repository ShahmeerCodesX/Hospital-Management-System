#pragma once
#include"hospitalexception.h"
class SlotUnavailableException : public hospitalexception {
public:
    
    SlotUnavailableException(const char* msg = "Error: Time slot is already occupied.");
};
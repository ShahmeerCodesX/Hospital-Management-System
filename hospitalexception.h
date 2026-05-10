#pragma once
#include <iostream>

class hospitalexception {
protected:
    char messages[200];
public:
    hospitalexception(const char* message);
    virtual char* what();
};
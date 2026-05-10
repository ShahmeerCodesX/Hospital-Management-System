#pragma once
#include <iostream>
#include "patient.h"
#include "doctor.h"

class Appointments {
private:
    int      aID;
    int      pID;   
    int      dID;  
    Patient* patient;
    Doctor* doctor;
    char* time;
    char* date;
    char* status;
public:
    Appointments();
    Appointments(int a, int pid, int did, const char* dt, const char* t, const char* s);
    Appointments(int a, Patient* p, Doctor* d, const char* t, const char* dt, const char* s);
    Appointments(const Appointments& other);
    Appointments& operator=(const Appointments& other);
    ~Appointments();

    int   getid()        const;
    int   getdocid();
    int   getpatientid();
    const char* getstatus();
    const char* getdate();
    const char* gettime();
    Patient* getpatient();
    void        setstatus(const char* s);

    bool operator==(const Appointments& other);
    friend std::ostream& operator<<(std::ostream& out, const Appointments& a);
};
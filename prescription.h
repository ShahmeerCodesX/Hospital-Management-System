#pragma once
#include <iostream>
#include "patient.h"
#include "doctor.h"
#include "appointment.h"

class Prescriptions {
private:
    int          id;
    Patient* pat;
    Doctor* doc;
    Appointments* app;
    char* medicines;
    char* date;
    char* notes;
public:
    Prescriptions();
    Prescriptions(int i, Patient* p, Doctor* d, Appointments* a,
        const char* m, const char* dt, const char* n);
    Prescriptions(const Prescriptions& other);
    Prescriptions& operator=(const Prescriptions& other);
    ~Prescriptions();

    int          getid()          const;
    Patient* getpatient()     const;
    Doctor* getdoctor()      const;
    Appointments* getappointment() const;
    const char* getmedicines()   const;
    const char* getdate()        const;
    const char* getnotes()       const;

    bool operator==(const Prescriptions& other);
    friend std::ostream& operator<<(std::ostream& out, const Prescriptions& p);
};
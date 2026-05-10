#pragma once
#include "person.h"
#include <iostream>

class Appointments; 
class Doctor : public Person {
private:
    int    contact;
    char* special;
    float  fee;
    Appointments** appointments;
    int    appointmentCount;
public:
    Doctor();
    Doctor(int id, char* n, char* p, char* s, int c, float f);
    Doctor(const Doctor& other);
    Doctor& operator=(const Doctor& other);
    ~Doctor();

   
    int   getid()   const override;
    char* getname()       override;

    char* getspecial();
    float getfee();
    int   getAppCount();
    Appointments** getAppointments();
    void  addAppointment(Appointments* newApp);
    void  sortAppointmentsByTime(bool ascending);

    bool operator==(const Doctor& d) const;
    friend std::ostream& operator<<(std::ostream& out, const Doctor& d);
};
#pragma once
#include "person.h"
#include <iostream>

class Appointments; 

class Patient : public Person {
private:
    int   age;
    char  gender;
    int   contact;
    float balance;
    Appointments** appointments;
    int   appointmentCount;
public:
    Patient();
    Patient(int id, char* n, char* p, int a, char g, int c, float b);
    Patient(const Patient& other);
    Patient& operator=(const Patient& other);
    ~Patient();

    
    int   getid()   const override;
    char* getname()       override;

    float getbalance();
    int getAppCount();
    int getcontact();

    Appointments** getAppointments();
    void  addAppointment(Appointments* newApp);
    void  sortAppointmentsByDate(bool ascending);

    Patient& operator+=(float p);
    Patient& operator-=(float p);
    bool     operator==(const Patient& p);
    int getage();
    char getgender();
    const char* getpass();
    friend std::ostream& operator<<(std::ostream& out, const Patient& p);
};
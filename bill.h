#pragma once
#include <iostream>
#include "appointment.h"

class Bill {
private:
    int          billid;
    int          patientid;
    int          appid;      
    Appointments* app;
    float        amount;
    char* status;
    char* date;
public:
    Bill();
    Bill(int billid, int patientid, int appid, float amount, const char* status, const char* date = nullptr);
    Bill(int b_id, int p_id, Appointments* a_obj, float amt, const char* stat);
    Bill(const Bill& other);
    Bill& operator=(const Bill& other);
    ~Bill();

    int          getid()          const;
    int          getpatientid()   const;
    int          getappid()       const;
    Appointments* getappointment() const;
    float        getamount()      const;
    const char* getstatus()      const;
    const char* getdate()        const;

    bool operator==(const Bill& other);
    friend std::ostream& operator<<(std::ostream& out, const Bill& b);
    void setstatus(const char* s);
};
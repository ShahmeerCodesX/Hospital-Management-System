#pragma once
#include "person.h"
#include "appointment.h"
#include "bill.h"
#include <ctime>

class Admin : public Person {
public:
    Admin();
    Admin(int id, char* n, char* p);
    ~Admin();

    
    int   getid()   const override;
    char* getname()       override;

    bool  hasUnpaidBills(int patientID, Bill** bills, int billCount);
    bool  hasPendingAppointments(int personID, bool isDoctor, Appointments** apps, int appCount);
    void  sortAppointmentsDescending(Appointments** apps, int count);
    double calculateDaysDifference(const char* dateStr);
    float calculateDailyRevenue(Bill** bills, int count);
    void  getDailyStats(const char* today, Appointments** apps, int count, int& p, int& comp, int& ns, int& canc);
};
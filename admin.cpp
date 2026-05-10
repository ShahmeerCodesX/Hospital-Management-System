#include "admin.h"
#include <ctime>

Admin::Admin() : Person() {}
Admin::Admin(int id, char* n, char* p) : Person(id, n, p) {}
Admin::~Admin() {}

int   Admin::getid()   const
{ 
    return ID;
}
char* Admin::getname() { 
    return name;
}

bool Admin::hasUnpaidBills(int patientID, Bill** bills, int billCount) {
    for (int i = 0; i < billCount; i++) {
        if (bills[i]->getpatientid() == patientID) {
            const char* s = bills[i]->getstatus();
            if (s && (s[0] == 'u' || s[0] == 'U')) return true;
        }
    }
    return false;
}

bool Admin::hasPendingAppointments(int personID, bool isDoctor, Appointments** apps, int appCount) {
    for (int i = 0; i < appCount; i++) {
        int tid = isDoctor ? apps[i]->getdocid() : apps[i]->getpatientid();
        if (tid == personID) {
            const char* s = apps[i]->getstatus();
            if (s && (s[0] == 'p' || s[0] == 'P')) return true;
        }
    }
    return false;
}

void Admin::sortAppointmentsDescending(Appointments** apps, int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            const char* d1 = apps[j]->getdate(), * d2 = apps[j + 1]->getdate();
            int y1 = (d1[6] - '0') * 1000 + (d1[7] - '0') * 100 + (d1[8] - '0') * 10 + (d1[9] - '0');
            int m1 = (d1[3] - '0') * 10 + (d1[4] - '0'), day1 = (d1[0] - '0') * 10 + (d1[1] - '0');
            int y2 = (d2[6] - '0') * 1000 + (d2[7] - '0') * 100 + (d2[8] - '0') * 10 + (d2[9] - '0');
            int m2 = (d2[3] - '0') * 10 + (d2[4] - '0'), day2 = (d2[0] - '0') * 10 + (d2[1] - '0');
            bool sw = (y1 < y2) || ((y1 == y2) && (m1 < m2)) || ((y1 == y2) && (m1 == m2) && (day1 < day2));
            if (sw) { Appointments* t = apps[j]; apps[j] = apps[j + 1]; apps[j + 1] = t; }
        }
    }
}

double Admin::calculateDaysDifference(const char* d) {
    struct tm t = { 0 };
    t.tm_mday = (d[0] - '0') * 10 + (d[1] - '0');
    t.tm_mon = ((d[3] - '0') * 10 + (d[4] - '0')) - 1;
    t.tm_year = ((d[6] - '0') * 1000 + (d[7] - '0') * 100 + (d[8] - '0') * 10 + (d[9] - '0')) - 1900;
    time_t bTime = mktime(&t), cTime = time(0);
    return difftime(cTime, bTime) / (60 * 60 * 24);
}

float Admin::calculateDailyRevenue(Bill** bills, int count) {
    float rev = 0;
    for (int i = 0; i < count; i++) {
        const char* s = bills[i]->getstatus();
        if (s && (s[0] == 'p' || s[0] == 'P')) rev += bills[i]->getamount();
    }
    return rev;
}

void Admin::getDailyStats(const char* today, Appointments** apps, int count,
    int& p, int& comp, int& ns, int& canc) {
    p = 0; comp = 0; ns = 0; canc = 0;
    for (int i = 0; i < count; i++) {
        bool match = true;
        const char* aDate = apps[i]->getdate();
        for (int k = 0; k < 10; k++) if (aDate[k] != today[k]) { match = false; break; }
        if (match) {
            const char* s = apps[i]->getstatus();
            if (s[0] == 'p' || s[0] == 'P') p++;
            else if (s[0] == 'c' && s[1] == 'o') comp++;
            else if (s[0] == 'n' || s[0] == 'N') ns++;
            else if (s[0] == 'c' && s[1] == 'a') canc++;
        }
    }
}


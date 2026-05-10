#include "patient.h"
#include "appointment.h"

Patient::Patient() : Person() {
    age = 0; gender = '\0'; contact = 0; balance = 0.0f;
    appointments = nullptr; appointmentCount = 0;
}

Patient::Patient(int id, char* n, char* p, int a, char g, int c, float b) : Person(id, n, p) {
    age = a; gender = g; contact = c; balance = b;
    appointments = nullptr; appointmentCount = 0;
}

Patient::Patient(const Patient& other) : Person(other) {
    age = other.age; gender = other.gender;
    contact = other.contact; balance = other.balance;
    appointments = nullptr; appointmentCount = 0;
}

Patient& Patient::operator=(const Patient& other) {
    if (this == &other) return *this;
   
    delete[] name; delete[] pass;
    ID = other.ID;
    if (other.name) {
        int l = 0; while (other.name[l]) l++;
        name = new char[l + 1];
        for (int i = 0; i <= l; i++) name[i] = other.name[i];
    }
    else { name = nullptr; }
    if (other.pass) {
        int l = 0; while (other.pass[l]) l++;
        pass = new char[l + 1];
        for (int i = 0; i <= l; i++) pass[i] = other.pass[i];
    }
    else { pass = nullptr; }
    age = other.age; gender = other.gender;
    contact = other.contact; balance = other.balance;
    delete[] appointments;
    appointments = nullptr; appointmentCount = 0;
    return *this;
}

Patient::~Patient() {
    delete[] appointments;
}

int   Patient::getid()   const { return ID; }
char* Patient::getname() { return name; }

float Patient::getbalance() { return balance; }
int   Patient::getAppCount() { return appointmentCount; }
Appointments** Patient::getAppointments() { return appointments; }

void Patient::addAppointment(Appointments* newApp) {
    Appointments** temp = new Appointments * [appointmentCount + 1];
    for (int i = 0; i < appointmentCount; i++) temp[i] = appointments[i];
    delete[] appointments;
    appointments = temp;
    appointments[appointmentCount] = newApp;
    appointmentCount++;
}

void Patient::sortAppointmentsByDate(bool ascending) {
    if (appointmentCount < 2) return;
    for (int i = 0; i < appointmentCount - 1; i++) {
        for (int j = 0; j < appointmentCount - i - 1; j++) {
            const char* d1 = appointments[j]->getdate();
            const char* d2 = appointments[j + 1]->getdate();
            int y1 = (d1[6] - '0') * 1000 + (d1[7] - '0') * 100 + (d1[8] - '0') * 10 + (d1[9] - '0');
            int m1 = (d1[3] - '0') * 10 + (d1[4] - '0');
            int day1 = (d1[0] - '0') * 10 + (d1[1] - '0');
            int y2 = (d2[6] - '0') * 1000 + (d2[7] - '0') * 100 + (d2[8] - '0') * 10 + (d2[9] - '0');
            int m2 = (d2[3] - '0') * 10 + (d2[4] - '0');
            int day2 = (d2[0] - '0') * 10 + (d2[1] - '0');
            bool sw = false;
            if (y1 > y2) sw = true;
            else if (y1 == y2 && m1 > m2) sw = true;
            else if (y1 == y2 && m1 == m2 && day1 > day2) sw = true;
            if (!ascending) sw = !sw;
            if (sw) { Appointments* t = appointments[j]; appointments[j] = appointments[j + 1]; appointments[j + 1] = t; }
        }
    }
}

Patient& Patient::operator+=(float p) { balance += p; return *this; }
Patient& Patient::operator-=(float p) { balance -= p; return *this; }
bool     Patient::operator==(const Patient& p) { return ID == p.ID; }

std::ostream& operator<<(std::ostream& out, const Patient& p) {
    out << p.ID << "|" << p.name << "|" << p.age << "|" << p.gender << "|" << p.contact << "|" << p.balance;
    return out;
}

int Patient::getage() {
    return age;
}

char Patient::getgender() {
    return gender;
}

const char* Patient::getpass() {
    return pass;
}

int Patient::getcontact() {
    return contact;
}
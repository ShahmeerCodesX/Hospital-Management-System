#include "doctor.h"
#include "appointment.h"

static int d_len(const char* s) { int l = 0; if (!s) return 0; while (s[l] != '\0') l++; return l; }
static void d_copy(const char* src, char* dest) { int i = 0; while (src[i] != '\0') { dest[i] = src[i]; i++; } dest[i] = '\0'; }

Doctor::Doctor() : Person() {
    contact = 0; special = nullptr; fee = 0.0f;
    appointments = nullptr; appointmentCount = 0;
}

Doctor::Doctor(int id, char* n, char* p, char* s, int c, float f) : Person(id, n, p) {
    contact = c; fee = f;
    special = new char[d_len(s) + 1];
    d_copy(s, special);
    appointments = nullptr; appointmentCount = 0;
}

Doctor::Doctor(const Doctor& other) : Person(other) {
    contact = other.contact; fee = other.fee;
    if (other.special) {
        special = new char[d_len(other.special) + 1];
        d_copy(other.special, special);
    }
    else { special = nullptr; }
    appointments = nullptr; appointmentCount = 0;
}

Doctor& Doctor::operator=(const Doctor& other) {
    if (this == &other) return *this;
    delete[] name; delete[] pass; delete[] special; delete[] appointments;
    ID = other.ID;
    if (other.name) { int l = d_len(other.name); name = new char[l + 1]; for (int i = 0; i <= l; i++) name[i] = other.name[i]; }
    else { name = nullptr; }
    if (other.pass) { int l = d_len(other.pass); pass = new char[l + 1]; for (int i = 0; i <= l; i++) pass[i] = other.pass[i]; }
    else { pass = nullptr; }
    contact = other.contact; fee = other.fee;
    if (other.special) { special = new char[d_len(other.special) + 1]; d_copy(other.special, special); }
    else { special = nullptr; }
    appointments = nullptr; appointmentCount = 0;
    return *this;
}

Doctor::~Doctor() {
    delete[] special;
    delete[] appointments;
}

int   Doctor::getid()   const { return ID; }
char* Doctor::getname() { return name; }
char* Doctor::getspecial() { return special; }
float Doctor::getfee() { return fee; }
int   Doctor::getAppCount() { return appointmentCount; }
Appointments** Doctor::getAppointments() { return appointments; }

void Doctor::addAppointment(Appointments* newApp) {
    Appointments** temp = new Appointments * [appointmentCount + 1];
    for (int i = 0; i < appointmentCount; i++) temp[i] = appointments[i];
    delete[] appointments;
    appointments = temp;
    appointments[appointmentCount] = newApp;
    appointmentCount++;
}

void Doctor::sortAppointmentsByTime(bool ascending) {
    if (appointmentCount < 2) return;
    for (int i = 0; i < appointmentCount - 1; i++) {
        for (int j = 0; j < appointmentCount - i - 1; j++) {
            const char* t1 = appointments[j]->gettime();
            const char* t2 = appointments[j + 1]->gettime();
            int h1 = (t1[0] - '0') * 10 + (t1[1] - '0'), m1 = (t1[3] - '0') * 10 + (t1[4] - '0');
            int h2 = (t2[0] - '0') * 10 + (t2[1] - '0'), m2 = (t2[3] - '0') * 10 + (t2[4] - '0');
            bool sw = (h1 > h2) || (h1 == h2 && m1 > m2);
            if (!ascending) sw = !sw;
            if (sw) { Appointments* t = appointments[j]; appointments[j] = appointments[j + 1]; appointments[j + 1] = t; }
        }
    }
}

bool Doctor::operator==(const Doctor& d) const { return ID == d.ID; }

std::ostream& operator<<(std::ostream& out, const Doctor& d) {
    out << d.ID << "|" << d.name << "|" << d.special << "|" << d.contact << "|" << d.fee;
    return out;
}
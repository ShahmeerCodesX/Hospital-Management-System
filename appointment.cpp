#include "appointment.h"

static int a_len(const char* a) { int c = 0; if (!a) return 0; while (a[c] != '\0') c++; return c; }
static void a_copy(const char* a, char* b) { int l = a_len(a); for (int i = 0; i < l; i++) b[i] = a[i]; b[l] = '\0'; }

Appointments::Appointments() {
    aID = 0; pID = 0; dID = 0;
    patient = nullptr; doctor = nullptr;
    time = nullptr; date = nullptr; status = nullptr;
}

Appointments::Appointments(int a, int pid, int did, const char* dt, const char* t, const char* s) {
    aID = a; pID = pid; dID = did;
    patient = nullptr; doctor = nullptr;
    time = new char[a_len(t) + 1];  a_copy(t, time);
    date = new char[a_len(dt) + 1]; a_copy(dt, date);
    status = new char[a_len(s) + 1];  a_copy(s, status);
}

Appointments::Appointments(int a, Patient* p, Doctor* d, const char* t, const char* dt, const char* s) {
    aID = a; patient = p; doctor = d;
    pID = p ? p->getid() : 0;
    dID = d ? d->getid() : 0;
    time = new char[a_len(t) + 1];  a_copy(t, time);
    date = new char[a_len(dt) + 1]; a_copy(dt, date);
    status = new char[a_len(s) + 1];  a_copy(s, status);
}

Appointments::Appointments(const Appointments& other) {
    aID = other.aID; pID = other.pID; dID = other.dID;
    patient = other.patient; doctor = other.doctor;
    time = other.time ? (new char[a_len(other.time) + 1], a_copy(other.time, time = new char[a_len(other.time) + 1]), time) : nullptr;
    date = other.date ? (a_copy(other.date, date = new char[a_len(other.date) + 1]), date) : nullptr;
    status = other.status ? (a_copy(other.status, status = new char[a_len(other.status) + 1]), status) : nullptr;
}

Appointments& Appointments::operator=(const Appointments& other) {
    if (this == &other) return *this;
    delete[] time; delete[] date; delete[] status;
    aID = other.aID; pID = other.pID; dID = other.dID;
    patient = other.patient; doctor = other.doctor;
    time = other.time ? (a_copy(other.time, time = new char[a_len(other.time) + 1]), time) : nullptr;
    date = other.date ? (a_copy(other.date, date = new char[a_len(other.date) + 1]), date) : nullptr;
    status = other.status ? (a_copy(other.status, status = new char[a_len(other.status) + 1]), status) : nullptr;
    return *this;
}

Appointments::~Appointments() {
    delete[] time; delete[] date; delete[] status;
}

int Appointments::getid()        const { return aID; }
int Appointments::getdocid() { return doctor ? doctor->getid() : dID; }
int Appointments::getpatientid() { return patient ? patient->getid() : pID; }
const char* Appointments::getstatus() { return status; }
const char* Appointments::getdate() { return date; }
const char* Appointments::gettime() { return time; }
Patient* Appointments::getpatient() { return patient; }

void Appointments::setstatus(const char* s) {
    delete[] status;
    status = new char[a_len(s) + 1];
    a_copy(s, status);
}

bool Appointments::operator==(const Appointments& other) { return aID == other.aID; }

std::ostream& operator<<(std::ostream& out, const Appointments& a) {
    out << a.aID << "|";
    if (a.doctor) out << a.doctor->getname() << "|" << a.doctor->getspecial() << "|";
    else          out << "Unknown|Unknown|";
    out << a.date << "|" << a.time << "|" << (a.status ? a.status : "Unknown") << std::endl;
    return out;
}  


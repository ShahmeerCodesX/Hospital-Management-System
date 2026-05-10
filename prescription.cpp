#include "prescription.h"

static int rx_len(const char* s) { if (!s) return 0; int l = 0; while (s[l] != '\0') l++; return l; }
static void rx_copy(const char* src, char* dest) { if (!src || !dest) return; int i = 0; while (src[i] != '\0') { dest[i] = src[i]; i++; } dest[i] = '\0'; }

Prescriptions::Prescriptions() {
    id = 0; pat = nullptr; doc = nullptr; app = nullptr;
    medicines = nullptr; date = nullptr; notes = nullptr;
}

Prescriptions::Prescriptions(int i, Patient* p, Doctor* d, Appointments* a,
    const char* m, const char* dt, const char* n) {
    id = i; pat = p; doc = d; app = a;
    medicines = m ? (rx_copy(m, medicines = new char[rx_len(m) + 1]), medicines) : nullptr;
    date = dt ? (rx_copy(dt, date = new char[rx_len(dt) + 1]), date) : nullptr;
    notes = n ? (rx_copy(n, notes = new char[rx_len(n) + 1]), notes) : nullptr;
}

Prescriptions::Prescriptions(const Prescriptions& other) {
    id = other.id; pat = other.pat; doc = other.doc; app = other.app;
    medicines = other.medicines ? (rx_copy(other.medicines, medicines = new char[rx_len(other.medicines) + 1]), medicines) : nullptr;
    date = other.date ? (rx_copy(other.date, date = new char[rx_len(other.date) + 1]), date) : nullptr;
    notes = other.notes ? (rx_copy(other.notes, notes = new char[rx_len(other.notes) + 1]), notes) : nullptr;
}

Prescriptions& Prescriptions::operator=(const Prescriptions& other) {
    if (this == &other) return *this;
    delete[] medicines; delete[] date; delete[] notes;
    id = other.id; pat = other.pat; doc = other.doc; app = other.app;
    medicines = other.medicines ? (rx_copy(other.medicines, medicines = new char[rx_len(other.medicines) + 1]), medicines) : nullptr;
    date = other.date ? (rx_copy(other.date, date = new char[rx_len(other.date) + 1]), date) : nullptr;
    notes = other.notes ? (rx_copy(other.notes, notes = new char[rx_len(other.notes) + 1]), notes) : nullptr;
    return *this;
}

Prescriptions::~Prescriptions() { delete[] medicines; delete[] date; delete[] notes; }

int          Prescriptions::getid()          const { return id; }
Patient* Prescriptions::getpatient()     const { return pat; }
Doctor* Prescriptions::getdoctor()      const { return doc; }
Appointments* Prescriptions::getappointment() const { return app; }
const char* Prescriptions::getmedicines()   const { return medicines; }
const char* Prescriptions::getdate()        const { return date; }
const char* Prescriptions::getnotes()       const { return notes; }

bool Prescriptions::operator==(const Prescriptions& other) { return id == other.id; }

std::ostream& operator<<(std::ostream& out, const Prescriptions& p) {
    out << p.id << " | " << (p.date ? p.date : "N/A") << " | "
        << (p.medicines ? p.medicines : "N/A") << " | "
        << (p.notes ? p.notes : "N/A");
    return out;
}
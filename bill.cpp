#include "bill.h"

static int b_len(const char* a) { int c = 0; if (!a) return 0; while (a[c] != '\0') c++; return c; }
static void b_copy(const char* a, char* b) { int l = b_len(a); for (int i = 0; i < l; i++) b[i] = a[i]; b[l] = '\0'; }

Bill::Bill() {
    billid = 0; patientid = 0; appid = 0; app = nullptr;
    amount = 0.0f; status = nullptr; date = nullptr;
}

Bill::Bill(int bid, int pid, int aid, float amt, const char* s, const char* d) {
    billid = bid; patientid = pid; appid = aid; app = nullptr; amount = amt;
    status = s ? (b_copy(s, status = new char[b_len(s) + 1]), status) : nullptr;
    date = d ? (b_copy(d, date = new char[b_len(d) + 1]), date) : nullptr;
}

Bill::Bill(int b_id, int p_id, Appointments* a_obj, float amt, const char* stat) {
    billid = b_id; patientid = p_id; app = a_obj;
    appid = a_obj ? a_obj->getid() : 0;
    amount = amt;
    status = stat ? (b_copy(stat, status = new char[b_len(stat) + 1]), status) : nullptr;
    date = nullptr;
}

Bill::Bill(const Bill& other) {
    billid = other.billid; patientid = other.patientid; appid = other.appid;
    app = other.app; amount = other.amount;
    status = other.status ? (b_copy(other.status, status = new char[b_len(other.status) + 1]), status) : nullptr;
    date = other.date ? (b_copy(other.date, date = new char[b_len(other.date) + 1]), date) : nullptr;
}

Bill& Bill::operator=(const Bill& other) {
    if (this == &other) return *this;
    delete[] status; delete[] date;
    billid = other.billid; patientid = other.patientid; appid = other.appid;
    app = other.app; amount = other.amount;
    status = other.status ? (b_copy(other.status, status = new char[b_len(other.status) + 1]), status) : nullptr;
    date = other.date ? (b_copy(other.date, date = new char[b_len(other.date) + 1]), date) : nullptr;
    return *this;
}

Bill::~Bill() { delete[] status; delete[] date; }

int          Bill::getid()          const { return billid; }
int          Bill::getpatientid()   const { return patientid; }
int          Bill::getappid()       const { return appid; }
Appointments* Bill::getappointment() const { return app; }
float        Bill::getamount()      const { return amount; }
const char* Bill::getstatus()      const { return status; }
const char* Bill::getdate()        const { return date; }

bool Bill::operator==(const Bill& other) { return billid == other.billid; }

std::ostream& operator<<(std::ostream& out, const Bill& b) {
    out << b.billid << "|" << b.patientid << "|" << b.appid << "|" << b.amount << "|"
        << (b.status ? b.status : "N/A") << std::endl;
    return out;
}

void Bill::setstatus(const char* s) {
    if (status)
        delete[]status;

    status = new char[b_len(s) + 1];
    b_copy(s, status);
}
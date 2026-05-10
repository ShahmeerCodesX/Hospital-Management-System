
#include "filehandler.h"
#include "filenotfoundexception.h"
#include <fstream>
#include <ctime>

static int fh_len(const char* str) {
    int len = 0; if (!str) return 0;
    while (str[len] != '\0') len++; return len;
}
static bool fh_cmp(const char* a, const char* b) {
    int l1 = fh_len(a), l2 = fh_len(b); if (l1 != l2) return false;
    for (int i = 0; i < l1; i++) if (a[i] != b[i]) return false; return true;
}
static void fh_trim(char* s) {
    if (!s) return; int len = fh_len(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\r' || s[len - 1] == '\n' || s[len - 1] == '\t'))
        s[--len] = '\0';
}
static int fh_atoi(const char* str) {
    if (!str) return 0; int res = 0;
    for (int i = 0; str[i] != '\0'; i++)
        if (str[i] >= '0' && str[i] <= '9') res = res * 10 + (str[i] - '0');
    return res;
}
static float fh_atof(const char* str) {
    if (!str) return 0.0f;
    float res = 0.0f, div = 1.0f; bool dot = false;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '.') { dot = true; }
        else if (str[i] >= '0' && str[i] <= '9') {
            if (!dot) res = res * 10.0f + (str[i] - '0');
            else { div *= 10.0f; res += (float)(str[i] - '0') / div; }
        }
    }
    return res;
}
static void fh_itoa(int num, char* str) {
    if (num == 0) { str[0] = '0'; str[1] = '\0'; return; }
    char tmp[20]; int i = 0; bool neg = (num < 0); if (neg) num = -num;
    while (num != 0) { tmp[i++] = '0' + num % 10; num /= 10; }
    if (neg) tmp[i++] = '-';
    int j = 0; while (i > 0) str[j++] = tmp[--i]; str[j] = '\0';
}


static int fh_split(char* line, char** cols, int maxcols) {
    int colCount = 0;
    if (colCount < maxcols) cols[colCount++] = &line[0];
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == ',') {
            line[i] = '\0';
            if (colCount < maxcols) cols[colCount++] = &line[i + 1];
        }
    }
    return colCount;
}



bool FileHandler::checkPrescriptionExists(int appid) {
    std::ifstream file("prescriptions.txt");
    if (!file.is_open()) return false;
    char line[512];
    
    while (file.getline(line, 512)) {
        char* cols[10] = { nullptr }; int nc = 0;
        char buf[512];
        int bi = 0;
        while (line[bi] != '\0' && bi < 511) { buf[bi] = line[bi]; bi++; } buf[bi] = '\0';
        nc = fh_split(buf, cols, 10);
        if (nc >= 2 && fh_atoi(cols[1]) == appid) {
            file.close(); return true;
        }
    }
    file.close(); return false;
}

bool FileHandler::checkDoctorHasPending(int docid) {
    
    std::ifstream file("appointments.txt");
    if (!file.is_open()) return false;
    char line[512]; bool first = true;
    while (file.getline(line, 512)) {
        if (first) { first = false; continue; }
        char* cols[10] = { nullptr }; int nc = fh_split(line, cols, 10);
        if (nc >= 6) {
            fh_trim(cols[5]);
            if (fh_atoi(cols[2]) == docid && fh_cmp(cols[5], "pending")) {
                file.close(); return true;
            }
        }
    }
    file.close(); return false;
}


bool FileHandler::authenticateUser(const char* role, const char* inID,
    const char* inPw, char* outName, char* outExtra) {

    
    char filename[50]; int fi = 0;
    while (role[fi] != '\0') { filename[fi] = role[fi]; fi++; }
    filename[fi++] = '.'; filename[fi++] = 't'; filename[fi++] = 'x';
    filename[fi++] = 't'; filename[fi] = '\0';

    std::ifstream file(filename);
    if (!file.is_open()) throw FileNotFoundException("File missing.");

    char line[512];
    file.getline(line, 512); 

    while (file.getline(line, 512)) {
        if (line[0] == '\0') continue;

       
        char buf[512]; int bi = 0;
        while (line[bi] != '\0' && bi < 511) { buf[bi] = line[bi]; bi++; } buf[bi] = '\0';

        char* cols[10] = { nullptr };
        int nc = fh_split(buf, cols, 10);
        if (nc < 3) continue;

        fh_trim(cols[0]);
        if (!fh_cmp(cols[0], inID)) continue;

      
        int passCol = -1, nameCol = 1, extraCol = -1;
        if (fh_cmp(role, "patient")) {
           
            passCol = 5; extraCol = 6;
        }
        else if (fh_cmp(role, "doctor")) {
            
            passCol = 4; extraCol = 2;
        }
        else if (fh_cmp(role, "admin")) {
           
            passCol = 2; extraCol = -1;
        }

        if (passCol < 0 || passCol >= nc) continue;

        fh_trim(cols[passCol]);
        if (fh_cmp(cols[passCol], inPw)) {
            
            if (outName && cols[nameCol]) {
                fh_trim(cols[nameCol]);
                int k = 0; while (cols[nameCol][k] && k < 99) { outName[k] = cols[nameCol][k]; k++; }
                outName[k] = '\0';
            }
            
            if (outExtra && extraCol >= 0 && extraCol < nc && cols[extraCol]) {
                fh_trim(cols[extraCol]);
                int k = 0; while (cols[extraCol][k] && k < 99) { outExtra[k] = cols[extraCol][k]; k++; }
                outExtra[k] = '\0';
            }
            file.close(); return true;
        }
    }
    file.close(); return false;
}



void FileHandler::writeattempttosecuritylog(const char* role,
    const char* inputid, const char* result) {
    std::ofstream file("security_log.txt", std::ios::app);
    if (!file.is_open()) throw FileNotFoundException("Cannot open security log.");
    time_t now = time(0); tm ltm; localtime_s(&ltm, &now);
    int day = ltm.tm_mday, month = 1 + ltm.tm_mon, year = 1900 + ltm.tm_year;
    int hour = ltm.tm_hour, min = ltm.tm_min, sec = ltm.tm_sec;
    if (day < 10) file << "0"; file << day << "-";
    if (month < 10) file << "0"; file << month << "-" << year << " ";
    if (hour < 10) file << "0"; file << hour << ":";
    if (min < 10) file << "0"; file << min << ":";
    if (sec < 10) file << "0"; file << sec;
    file << "," << role << "," << inputid << "," << result << "\n";
    file.close();
}

void FileHandler::writePrescription(int prescid, int appid, int patid,
    int docid, const char* date, char* medicines, char* notes) {
    if (medicines) { int i = 0; while (medicines[i] && i < 499)i++; medicines[i] = '\0'; }
    if (notes) { int j = 0; while (notes[j] && j < 299)j++; notes[j] = '\0'; }
    std::ofstream file("prescriptions.txt", std::ios::app);
    if (!file.is_open()) throw FileNotFoundException("Cannot open prescriptions file.");
    file << prescid << "," << appid << "," << patid << "," << docid << "," << date << ","
        << medicines << "," << notes << "\n";
    file.close();
}

void FileHandler::writenewappointment(int appid, int patid, int docid,
    const char* date, const char* slot, const char* status) {
    std::ofstream file("appointments.txt", std::ios::app);
    if (!file.is_open()) throw FileNotFoundException("Cannot open appointments file.");
    file << appid << "," << patid << "," << docid << "," << date << "," << slot << "," << status << "\n";
    file.close();
}

void FileHandler::writenewbill(int billid, int patid, int appid,
    float amount, const char* status, const char* date) {
    std::ofstream file("bills.txt", std::ios::app);
    if (!file.is_open()) throw FileNotFoundException("Cannot open bills file.");
    file << billid << "," << patid << "," << appid << "," << amount << "," << status << "," << date << "\n";
    file.close();
}

void FileHandler::writeDoctor(int id, const char* name, const char* spec,
    const char* contact, const char* pass, float fee) {
    std::ofstream file("doctor.txt", std::ios::app);
    if (!file.is_open()) throw FileNotFoundException("Cannot open doctors file.");
    file << id << "," << name << "," << spec << "," << contact << "," << pass << "," << fee << "\n";
    file.close();
}

static void removeByID(const char* inFilename, const char* outFilename, int targetID) {
    std::ifstream infile(inFilename);
    std::ofstream outfile(outFilename);
    if (!infile.is_open() || !outfile.is_open())
        throw FileNotFoundException("Cannot open file for removal.");
    char target[20]; fh_itoa(targetID, target);
    char line[512];
    while (infile.getline(line, sizeof(line))) {
        if (line[0] == '\0') continue;
        // Match first field before comma
        bool match = true; int i = 0, ti = 0;
        while (line[i] != ',' && line[i] != '\0' && target[ti] != '\0') {
            if (line[i] != target[ti]) { match = false; break; }
            i++; ti++;
        }
        if (match && (line[i] == ',' || line[i] == '\0') && target[ti] == '\0') continue;
        outfile << line << "\n";
    }
    infile.close(); outfile.close();
}

void FileHandler::removeDoctor(int docid) {
    removeByID("doctor.txt", "temp_doc.txt", docid);
    std::remove("doctor.txt"); std::rename("temp_doc.txt", "doctor.txt");
}

void FileHandler::removePatient(int patid) {
    removeByID("patients.txt", "temp_pat.txt", patid);
    std::remove("patient.txt"); std::rename("temp_pat.txt", "patient.txt");
}



void FileHandler::updateappointmentstatus(int appid, const char* newstatus) {
    // appointments.txt: appt_id,pat_id,doc_id,date,slot,status
    std::ifstream infile("appointments.txt");
    std::ofstream outfile("temp_app.txt");
    if (!infile.is_open() || !outfile.is_open())
        throw FileNotFoundException("Cannot open appointment files.");
    char target[20]; fh_itoa(appid, target);
    char line[512];
    while (infile.getline(line, sizeof(line))) {
        if (line[0] == '\0') continue;
        char buf[512]; int bi = 0;
        while (line[bi] != '\0' && bi < 511) { buf[bi] = line[bi]; bi++; } buf[bi] = '\0';
        bool match = true; int i = 0, ti = 0;
        while (buf[i] != ',' && buf[i] != '\0' && target[ti] != '\0') {
            if (buf[i] != target[ti]) { match = false; break; } i++; ti++;
        }
        if (match && (buf[i] == ',' || buf[i] == '\0') && target[ti] == '\0') {
            
            int cc = 0, j = 0;
            while (line[j] != '\0') {
                outfile << line[j];
                if (line[j] == ',') { cc++; if (cc == 5) break; }
                j++;
            }
            outfile << newstatus << "\n";
        }
        else {
            outfile << line << "\n";
        }
    }
    infile.close(); outfile.close();
    std::remove("appointments.txt"); std::rename("temp_app.txt", "appointments.txt");
}

void FileHandler::updatebillstatus(int billid, const char* newstatus) {
   
    std::ifstream infile("bills.txt");
    std::ofstream outfile("temp_bil.txt");
    if (!infile.is_open() || !outfile.is_open())
        throw FileNotFoundException("Cannot open bill files.");
    char target[20]; fh_itoa(billid, target);
    char line[512];
    while (infile.getline(line, sizeof(line))) {
        if (line[0] == '\0') continue;
        char buf[512]; int bi = 0;
        while (line[bi] != '\0' && bi < 511) { buf[bi] = line[bi]; bi++; } buf[bi] = '\0';
        bool match = true; int i = 0, ti = 0;
        while (buf[i] != ',' && buf[i] != '\0' && target[ti] != '\0') {
            if (buf[i] != target[ti]) { match = false; break; } i++; ti++;
        }
        if (match && (buf[i] == ',' || buf[i] == '\0') && target[ti] == '\0') {
           
            int cc = 0, j = 0;
            while (line[j] != '\0') {
                outfile << line[j];
                if (line[j] == ',') { cc++; if (cc == 4) { j++; break; } }
                j++;
            }
            outfile << newstatus;
           
            while (line[j] != '\0' && line[j] != ',') j++;
            
            while (line[j] != '\0') { outfile << line[j]; j++; }
            outfile << "\n";
        }
        else {
            outfile << line << "\n";
        }
    }
    infile.close(); outfile.close();
    std::remove("bills.txt"); std::rename("temp_bil.txt", "bills.txt");
}

void FileHandler::updatepatientbalance(int patid, float newbalance) {
   
    std::ifstream infile("patient.txt");
    std::ofstream outfile("temp_pts.txt");
    if (!infile.is_open() || !outfile.is_open())
        throw FileNotFoundException("Cannot open patient files.");
    char target[20]; fh_itoa(patid, target);
    char line[512];
    while (infile.getline(line, sizeof(line))) {
        if (line[0] == '\0') continue;
        char buf[512]; int bi = 0;
        while (line[bi] != '\0' && bi < 511) { buf[bi] = line[bi]; bi++; } buf[bi] = '\0';
        bool match = true; int i = 0, ti = 0;
        while (buf[i] != ',' && buf[i] != '\0' && target[ti] != '\0') {
            if (buf[i] != target[ti]) { match = false; break; } i++; ti++;
        }
        if (match && (buf[i] == ',' || buf[i] == '\0') && target[ti] == '\0') {
           
            int cc = 0, j = 0;
            while (line[j] != '\0') {
                outfile << line[j];
                if (line[j] == ',') { cc++; if (cc == 6) break; }
                j++;
            }
            outfile << newbalance << "\n";
        }
        else {
            outfile << line << "\n";
        }
    }
    infile.close(); outfile.close();
    std::remove("patient.txt"); std::rename("temp_pts.txt", "patient.txt");
}

void FileHandler::writedischargedpatient(int patid, const char* name, int age,
    const char* gender, const char* contact,
    const char* password, float balance) {
    std::ofstream file("discharged_patients.txt", std::ios::app);
    if (!file.is_open()) throw FileNotFoundException("Cannot open discharged patients file.");
    file << patid << "," << name << "," << age << "," << gender << "," << contact << "," << password << "," << balance << "\n";
    file.close();
}



void FileHandler::loadAllDoctors(Storage<Doctor>& storage) {
    
    std::ifstream file("doctor.txt");
    if (!file.is_open()) return;
    char line[512]; file.getline(line, 512);
    while (file.getline(line, 512)) {
        if (line[0] == '\0') continue;
        char* cols[10] = { nullptr }; int nc = fh_split(line, cols, 10);
        if (nc >= 6) {
            fh_trim(cols[5]);
            int  id = fh_atoi(cols[0]);
            char* name = cols[1];
            char* spec = cols[2];
            char* cont = cols[3];  
            char* pass = cols[4];
            float fee = fh_atof(cols[5]);
            int contactI = fh_atoi(cols[3]);
           
            storage.add(Doctor(id, name, pass, spec, contactI, fee));
        }
    }
    file.close();
}

void FileHandler::loadAllPatients(Storage<Patient>& storage) {
    
    std::ifstream file("patient.txt");
    if (!file.is_open()) return;
    char line[512]; file.getline(line, 512); 
    while (file.getline(line, 512)) {
        if (line[0] == '\0') continue;
        char* cols[10] = { nullptr }; int nc = fh_split(line, cols, 10);
        if (nc >= 7) {
            fh_trim(cols[6]);
            int   id = fh_atoi(cols[0]);
            char* name = cols[1];
            int   age = fh_atoi(cols[2]);
            char  gender = cols[3][0];
            int   contact = fh_atoi(cols[4]);
            char* pass = cols[5];
            float balance = fh_atof(cols[6]);
          
            storage.add(Patient(id, name, pass, age, gender, contact, balance));
        }
    }
    file.close();
}

void FileHandler::loadAllAppointments(Storage<Appointments>& storage) {
    
    std::ifstream file("appointments.txt");
    if (!file.is_open()) return;
    char line[512]; file.getline(line, 512); 
    while (file.getline(line, 512)) {
        if (line[0] == '\0') continue;
        char* cols[10] = { nullptr }; int nc = fh_split(line, cols, 10);
        if (nc >= 6) {
            fh_trim(cols[5]);
            int   id = fh_atoi(cols[0]);
            int   pId = fh_atoi(cols[1]);
            int   dId = fh_atoi(cols[2]);
            char* date = cols[3];
            char* slot = cols[4];
            char* status = cols[5];
            storage.add(Appointments(id, pId, dId, date, slot, status));
        }
    }
    file.close();
}

void FileHandler::loadAllBills(Storage<Bill>& storage) {
   
    std::ifstream file("bills.txt");
    if (!file.is_open()) return;
    char line[512]; file.getline(line, 512); 
    while (file.getline(line, 512)) {
        if (line[0] == '\0') continue;
        char* cols[10] = { nullptr }; int nc = fh_split(line, cols, 10);
        if (nc >= 6) {
            fh_trim(cols[5]);
            int   id = fh_atoi(cols[0]);
            int   pId = fh_atoi(cols[1]);
            int   aId = fh_atoi(cols[2]);
            float amt = fh_atof(cols[3]);
            char* status = cols[4];
            char* date = cols[5];
            fh_trim(date);
            storage.add(Bill(id, pId, aId, amt, status, date));
        }
    }
    file.close();
}

int FileHandler::generateID(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return 1;
    char line[512]; int maxID = 0;
    file.getline(line, 512); 
    while (file.getline(line, 512)) {
        int id = 0, i = 0;
        while (line[i] >= '0' && line[i] <= '9') { id = id * 10 + (line[i] - '0'); i++; }
        if (id > maxID) maxID = id;
    }
    file.close(); return maxID + 1;
}
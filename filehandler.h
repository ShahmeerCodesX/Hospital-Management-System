#pragma once
#include "storage.h"
#include "patient.h"
#include "doctor.h"
#include "appointment.h"
#include "bill.h"

class FileHandler {
public:
    static bool checkPrescriptionExists(int appid);
    static bool checkDoctorHasPending(int docid);
    static bool authenticateUser(const char* role, const char* inID, const char* inPw,
        char* outName, char* outExtra);
    static void writeattempttosecuritylog(const char* role, const char* inputid, const char* result);
    static void writePrescription(int prescid, int appid, int patid, int docid,
        const char* date, char* medicines, char* notes);
    static void writenewappointment(int appid, int patid, int docid,
        const char* date, const char* slot, const char* status);
    static void writenewbill(int billid, int patid, int appid,
        float amount, const char* status, const char* date);
    static void writeDoctor(int id, const char* name, const char* spec,
        const char* contact, const char* pass, float fee);
    static void removeDoctor(int docid);
    static void removePatient(int patid);
    static void updateappointmentstatus(int appid, const char* newstatus);
    static void updatebillstatus(int billid, const char* newstatus);
    static void updatepatientbalance(int patid, float newbalance);
    static void writedischargedpatient(int patid, const char* name, int age,
        const char* gender, const char* contact,
        const char* password, float balance);
    static void loadAllDoctors(Storage<Doctor>& storage);
    static void loadAllPatients(Storage<Patient>& storage);
    static void loadAllAppointments(Storage<Appointments>& storage);
    static void loadAllBills(Storage<Bill>& storage);
    static int  generateID(const char* filename);

};
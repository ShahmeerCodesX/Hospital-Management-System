#pragma once
#include "storage.h"
#include "patient.h"
#include "doctor.h"
#include "appointment.h"
#include "bill.h"
#include "invalidinputexception.h"
#include "slotunavailableexception.h"

char myToLower(char c);
bool compareInsensitive(const char* str1, const char* str2);
bool comparestr(const char* a, const char* b);

class Validator {
public:
    static void validateID(int id);
    static void validatePositiveFloat(float value);
    static void validateMenuChoice(int choice, int min, int max);
    static void validatePassword(const char* pass);
    static void validateDate(const char* date);
    static void validateContact(const char* con);
    static void validateDoctorID(int id, Storage<Doctor>& doc);
    static void validateAppointmentID(int id, Storage<Appointments>& app);
    static void validateSlot(const char* slot, int docID, const char* date, const Appointments& app);
    static void validateDoctorhasPatient(int id, int docid, Storage<Appointments>& applist, Storage<Patient>& patlist);
    static void validatePatienthasAppointment(int id, int patid, Storage<Appointments>& applist);
    static void validatePatienthasBill(int id, int patid, Storage<Bill>& bil);
    static void validateDoctorhasAppointmentPending(int id, int docid, Storage<Appointments>& applist);
    static void validateDoctorhasAppointmentCompleted(int id, int docid, Storage<Appointments>& applist);
    static void validateDoctorhasAppointmentCompletedNotDuplicated(int id, int docid, Storage<Appointments>& applist);
    static void DoctorhasPendingAppointments(int id);
    static void PatienthasPendingAppointments(int patid, Storage<Appointments>& applist);
    static void PatienthasUnpaidBills(int patid, Storage<Bill>& bil);
    static void isDateOverdue(const char* date);
    static void validateUser(const char* role, char* inID, char* inPw, char* outName, char* outExtra);
    static void validateRefundEligibility(int appID, Storage<Appointments>& appList);
};
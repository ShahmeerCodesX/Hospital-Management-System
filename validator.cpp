#include "validator.h"
#include "filehandler.h"
#include "filenotfoundexception.h"
#include <ctime>

char myToLower(char c) {
	if (c >= 'A' && c <= 'Z') return c + 32;
	return c;
}

bool compareInsensitive(const char* str1, const char* str2) {
	int i = 0;
	while (str1[i] != '\0' && str2[i] != '\0') {
		if (myToLower(str1[i]) != myToLower(str2[i])) return false;
		i++;
	}
	return str1[i] == '\0' && str2[i] == '\0';
}

static int v_len(const char* str) { int len = 0; if (!str) return 0; while (str[len] != '\0') len++; return len; }

bool comparestr(const char* a, const char* b) {
	int l1 = v_len(a), l2 = v_len(b);
	if (l1 != l2) return false;
	for (int i = 0; i < l1; i++) if (a[i] != b[i]) return false;
	return true;
}

static int chartoint(char c) { return c - '0'; }

void Validator::validateID(int id) {
	if (id <= 0) throw InvalidInputException("Error: ID must be a positive integer.");
}

void Validator::validatePositiveFloat(float value) {
	if (value < 0) throw InvalidInputException("Error: Fee cannot be negative.");
}

void Validator::validateMenuChoice(int choice, int min, int max) {
	if (choice < min || choice > max) throw InvalidInputException("Error: Invalid menu selection.");
}

void Validator::validatePassword(const char* pass) {
	if (v_len(pass) < 6) throw InvalidInputException("Error: Password must be at least 6 characters.");
}

void Validator::validateDate(const char* date) {
	if (v_len(date) != 10) throw InvalidInputException("Error: Date must be DD-MM-YYYY.");
	if (date[2] != '-' || date[5] != '-') throw InvalidInputException("Error: Date must use dashes.");
	for (int i = 0; i < 10; i++) {
		if (i == 2 || i == 5) continue;
		if (date[i] < '0' || date[i]>'9') throw InvalidInputException("Error: Date has invalid characters.");
	}
	int day = chartoint(date[0]) * 10 + chartoint(date[1]);
	int month = chartoint(date[3]) * 10 + chartoint(date[4]);
	int year = chartoint(date[6]) * 1000 + chartoint(date[7]) * 100 + chartoint(date[8]) * 10 + chartoint(date[9]);
	if (day < 1 || day>31) throw InvalidInputException("Invalid day (01-31).");
	if (month < 1 || month>12) throw InvalidInputException("Invalid month (01-12).");
	if (year < 2026) throw InvalidInputException("Year must be 2026 or later.");
	if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30) throw InvalidInputException("Month cannot have 31 days.");
	if (month == 2 && day > 28 && year % 4 != 0) throw InvalidInputException("February cannot exceed 28 days.");
}

void Validator::validateContact(const char* con) {
	if (v_len(con) != 11) throw InvalidInputException("Error: Contact must be exactly 11 digits.");
	for (int i = 0; i < 11; i++) if (con[i] < '0' || con[i]>'9') throw InvalidInputException("Error: Contact must be numeric.");
}

void Validator::validateDoctorID(int id, Storage<Doctor>& doc) {
	if (doc.findbyid(id) == nullptr) throw InvalidInputException("Doctor Not Found");
}

void Validator::validateAppointmentID(int id, Storage<Appointments>& app) {
	if (app.findbyid(id) == nullptr) throw InvalidInputException("Appointment Not Found");
}

void Validator::validateSlot(const char* slot, int docID, const char* date, const Appointments& app) {
	const char* slots[8] = { "09:00","10:00","11:00","12:00","13:00","14:00","15:00","16:00" };
	bool valid = false;
	for (int i = 0; i < 8; i++) {
		bool found = true;
		for (int j = 0; j < 5; j++) if (slot[j] != slots[i][j]) { found = false; break; }
		if (found) { valid = true; break; }
	}
	if (!valid) throw SlotUnavailableException("Slot unavailable, try again.");
}

void Validator::validateDoctorhasPatient(int id, int docid, Storage<Appointments>& applist, Storage<Patient>& patlist) {
	if (patlist.findbyid(id) == nullptr) throw InvalidInputException("Patient Not Found");
	bool found = false;
	Appointments* app = applist.getall(); int s = applist.getsize();
	for (int i = 0; i < s; i++) {
		if (app[i].getdocid() == docid && app[i].getpatientid() == id && comparestr(app[i].getstatus(), "completed"))
		{
			found = true; break;
		}
	}
	if (!found) throw InvalidInputException("Access Denied");
}

void Validator::validatePatienthasAppointment(int id, int patid, Storage<Appointments>& applist) {
	Appointments* P = applist.findbyid(id);
	if (P == nullptr) throw InvalidInputException("Invalid Appointment ID");
	if (!(P->getpatientid() == patid && comparestr(P->getstatus(), "pending")))
		throw InvalidInputException("Invalid Appointment ID");
}

void Validator::validatePatienthasBill(int id, int patid, Storage<Bill>& bil) {
	Bill* P = bil.findbyid(id);
	if (P == nullptr) throw InvalidInputException("Invalid Bill ID");
	if (!(P->getpatientid() == patid && comparestr(P->getstatus(), "unpaid")))
		throw InvalidInputException("Invalid Bill ID");
}

void Validator::validateDoctorhasAppointmentPending(int id, int docid, Storage<Appointments>& applist) {
	Appointments* P = applist.findbyid(id);
	if (P == nullptr) throw InvalidInputException("Invalid Appointment ID");
	if (P->getdocid() != docid || !comparestr(P->getstatus(), "pending"))
		throw InvalidInputException("Invalid Appointment ID");
	time_t now = time(0); tm ltm; localtime_s(&ltm, &now);
	char buffer[11];
	int day = ltm.tm_mday, month = 1 + ltm.tm_mon, year = 1900 + ltm.tm_year;
	buffer[0] = (day / 10) + '0'; buffer[1] = (day % 10) + '0'; buffer[2] = '-';
	buffer[3] = (month / 10) + '0'; buffer[4] = (month % 10) + '0'; buffer[5] = '-';
	buffer[6] = (year / 1000) + '0'; buffer[7] = (year / 100 % 10) + '0';
	buffer[8] = (year / 10 % 10) + '0'; buffer[9] = (year % 10) + '0'; buffer[10] = '\0';
	const char* dat = P->getdate();
	for (int i = 0; i < 10; i++) if (buffer[i] != dat[i]) throw InvalidInputException("Appointment is not today.");
}

void Validator::validateDoctorhasAppointmentCompleted(int id, int docid, Storage<Appointments>& applist) {
	Appointments* P = applist.findbyid(id);
	if (P == nullptr) throw InvalidInputException("Invalid Appointment ID");
	if (P->getdocid() != docid || !comparestr(P->getstatus(), "completed"))
		throw InvalidInputException("Invalid Appointment ID");
}

void Validator::validateDoctorhasAppointmentCompletedNotDuplicated(int id, int docid, Storage<Appointments>& applist) {
	validateDoctorhasAppointmentCompleted(id, docid, applist);
	if (FileHandler::checkPrescriptionExists(id))
		throw InvalidInputException("Prescription already written for this appointment.");
}

void Validator::DoctorhasPendingAppointments(int id) {
	if (FileHandler::checkDoctorHasPending(id))
		throw InvalidInputException("Cannot remove doctor with pending appointments.");
}

void Validator::PatienthasPendingAppointments(int patid, Storage<Appointments>& applist) {
	Appointments* app = applist.getall(); int s = applist.getsize();
	for (int i = 0; i < s; i++)
		if (app[i].getpatientid() == patid && comparestr(app[i].getstatus(), "pending"))
			throw InvalidInputException("Cannot discharge patient with pending appointments.");
}

void Validator::PatienthasUnpaidBills(int patid, Storage<Bill>& bil) {
	Bill* b = bil.getall(); int s = bil.getsize();
	for (int i = 0; i < s; i++)
		if (b[i].getpatientid() == patid && comparestr(b[i].getstatus(), "unpaid"))
			throw InvalidInputException("Cannot discharge patient with unpaid bills.");
}

void Validator::isDateOverdue(const char* date) {
	if (!date) throw InvalidInputException("Date is null");
	int day = (date[0] - '0') * 10 + (date[1] - '0');
	int month = (date[3] - '0') * 10 + (date[4] - '0');
	int year = (date[6] - '0') * 1000 + (date[7] - '0') * 100 + (date[8] - '0') * 10 + (date[9] - '0');
	tm billDate = { 0 }; billDate.tm_mday = day; billDate.tm_mon = month - 1; billDate.tm_year = year - 1900; billDate.tm_isdst = -1;
	time_t billTime = mktime(&billDate), now = time(0);
	if (difftime(now, billTime) > 604800.0) throw InvalidInputException("OVERDUE");
}

void Validator::validateUser(const char* role, char* inID, char* inPw, char* outName, char* outExtra) {
	if (!FileHandler::authenticateUser(role, inID, inPw, outName, outExtra))
		throw InvalidInputException("Invalid ID or Password.");
}

void Validator::validateRefundEligibility(int appID, Storage<Appointments>& appList) {
	Appointments* app = appList.findbyid(appID);
	if (!app) throw InvalidInputException("Appointment not found.");
	int day = (app->getdate()[0] - '0') * 10 + (app->getdate()[1] - '0');
	int month = (app->getdate()[3] - '0') * 10 + (app->getdate()[4] - '0');
	int year = (app->getdate()[6] - '0') * 1000 + (app->getdate()[7] - '0') * 100 + (app->getdate()[8] - '0') * 10 + (app->getdate()[9] - '0');
	tm appDate = { 0 }; appDate.tm_mday = day; appDate.tm_mon = month - 1; appDate.tm_year = year - 1900;
	time_t appTime = mktime(&appDate), now = time(0);
	if (difftime(appTime, now) < 86400.0) throw InvalidInputException("Cancellation allowed only 24h in advance.");
}
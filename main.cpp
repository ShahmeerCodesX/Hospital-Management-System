// ============================================================
//  MediCore HMS  -  main.cpp  |  SFML 3.1
// ============================================================
#include <SFML/Graphics.hpp>
#include <fstream>
#include <ctime>
#include <cstdio>
#include <cstdint>

#include "person.h"
#include "patient.h"
#include "doctor.h"
#include "admin.h"
#include "appointment.h"
#include "bill.h"
#include "storage.h"
#include "filehandler.h"
#include "validator.h"
#include "hospitalexception.h"
#include "filenotfoundexception.h"
#include "insufficientfundsexception.h"
#include "invalidinputexception.h"
#include "slotunavailableexception.h"

// ── Manual string helpers ─────────────────────────────────────────
static int  ms_len(const char* s) { int n = 0; if (!s)return 0; while (s[n])n++; return n; }
static void ms_cpy(const char* src, char* dst, int max) { int i = 0; while (src[i] && i < max - 1) { dst[i] = src[i]; i++; }dst[i] = '\0'; }
static void ms_cat(char* dst, const char* src, int max) { int d = ms_len(dst), i = 0; while (src[i] && d + i < max - 1) { dst[d + i] = src[i]; i++; }dst[d + i] = '\0'; }
static bool ms_eq(const char* a, const char* b) { int i = 0; while (a[i] && b[i]) { if (a[i] != b[i])return false; i++; }return a[i] == '\0' && b[i] == '\0'; }
static int  ms_toi(const char* s) { int r = 0; if (!s)return 0; for (int i = 0; s[i] >= '0' && s[i] <= '9'; i++)r = r * 10 + (s[i] - '0'); return r; }
static void ms_itos(int v, char* buf) { if (!v) { buf[0] = '0'; buf[1] = '\0'; return; }char t[20]; int i = 0; bool neg = (v < 0); if (neg)v = -v; while (v > 0) { t[i++] = '0' + v % 10; v /= 10; }if (neg)t[i++] = '-'; for (int j = 0; j < i; j++)buf[j] = t[i - 1 - j]; buf[i] = '\0'; }
static void ms_ftos(float v, char* buf) { int w = (int)v, f = (int)((v - w) * 100 + 0.5f); char t[30], t2[20]; int i = 0, j = 0; if (!w) { t[i++] = '0'; } else { int ww = w; while (ww > 0) { t2[j++] = '0' + ww % 10; ww /= 10; }for (int k = j - 1; k >= 0; k--)t[i++] = t2[k]; }t[i++] = '.'; t[i++] = '0' + f / 10; t[i++] = '0' + f % 10; t[i] = '\0'; ms_cpy(t, buf, 30); }
static void ms_today(char* buf) {
    time_t now = time(0); tm lt;
#ifdef _WIN32
    localtime_s(&lt, &now);
#else
    tm* tmp = localtime(&now); if (tmp)lt = *tmp;
#endif
    int d = lt.tm_mday, m = 1 + lt.tm_mon, y = 1900 + lt.tm_year;
    buf[0] = '0' + d / 10; buf[1] = '0' + d % 10; buf[2] = '-';
    buf[3] = '0' + m / 10; buf[4] = '0' + m % 10; buf[5] = '-';
    buf[6] = '0' + y / 1000; buf[7] = '0' + (y / 100) % 10; buf[8] = '0' + (y / 10) % 10; buf[9] = '0' + y % 10; buf[10] = '\0';
}

// ── Slot check ───────────────────────────────────────────────────
static bool slotTaken(Storage<Appointments>& adb, int docID, const char* date, const char* slot) {
    Appointments* arr = adb.getall(); int sz = adb.getsize();
    for (int i = 0; i < sz; i++) {
        if (arr[i].getdocid() != docID)continue;
        const char* s = arr[i].getstatus(); if (s && s[0] == 'c' && s[1] == 'a')continue;
        if (ms_eq(arr[i].getdate(), date) && ms_eq(arr[i].gettime(), slot))return true;
    }
    return false;
}

// ── Brighten color (replaces sf::Uint8 arithmetic) ───────────────
static sf::Color brighten(sf::Color c, int by) {
    int r = (int)c.r + by, g = (int)c.g + by, b = (int)c.b + by;
    return sf::Color((uint8_t)(r > 255 ? 255 : r), (uint8_t)(g > 255 ? 255 : g), (uint8_t)(b > 255 ? 255 : b), c.a);
}

// ════════════════════════════════════════════════════════════
//  DRAWING HELPERS
// ════════════════════════════════════════════════════════════
static void drawRect(sf::RenderWindow& w, float x, float y, float wd, float ht,
    sf::Color col, float outline = 0.f, sf::Color outCol = sf::Color::White) {
    sf::RectangleShape r({ wd,ht }); r.setPosition({ x,y }); r.setFillColor(col);
    if (outline > 0.f) { r.setOutlineThickness(outline); r.setOutlineColor(outCol); }w.draw(r);
}

static void drawText(sf::RenderWindow& w, const sf::Font& f, const char* str,
    float x, float y, unsigned sz = 20, sf::Color col = sf::Color::White, bool outline = false) {
    sf::Text t(f, str, sz); t.setFillColor(col); t.setPosition({ x,y });
    if (outline) { t.setOutlineThickness(2.f); t.setOutlineColor(sf::Color::Black); }w.draw(t);
}

static void drawCentredText(sf::RenderWindow& w, const sf::Font& f, const char* str,
    float rx, float ry, float rw, float rh, unsigned sz, sf::Color col) {
    sf::Text t(f, str, sz); t.setFillColor(col);
    auto lb = t.getLocalBounds();
    t.setOrigin({ lb.position.x + lb.size.x / 2.f,lb.position.y + lb.size.y / 2.f });
    t.setPosition({ rx + rw / 2.f,ry + rh / 2.f }); w.draw(t);
}

static void drawMenuBtn(sf::RenderWindow& w, const sf::Font& f,
    float x, float y, float bw, float bh, const char* lbl, sf::Color fill, bool selected = false) {
    drawRect(w, x + 4, y + 4, bw, bh, sf::Color(0, 0, 0, 80));
    sf::RectangleShape b({ bw,bh }); b.setPosition({ x,y }); b.setFillColor(fill);
    if (selected) { b.setOutlineThickness(3.f); b.setOutlineColor(sf::Color::White); }
    w.draw(b); drawCentredText(w, f, lbl, x, y, bw, bh, 22, sf::Color::White);
}

static void drawInputField(sf::RenderWindow& w, const sf::Font& f,
    float x, float y, float fw, const char* lbl, const char* val, bool active, bool pw = false) {
    drawText(w, f, lbl, x, y, 20, sf::Color::White);
    sf::Color boxCol = active ? sf::Color(0, 0, 0, 200) : sf::Color(0, 0, 0, 150);
    sf::Color borderCol = active ? sf::Color::Yellow : sf::Color::White;
    sf::RectangleShape box({ fw,50.f }); box.setPosition({ x,y + 28 });
    box.setFillColor(boxCol); box.setOutlineThickness(2.f); box.setOutlineColor(borderCol); w.draw(box);
    int vl = ms_len(val); char disp[120]; int di = 0;
    if (pw) { for (int i = 0; i < vl && di < 118; i++)disp[di++] = '*'; }
    else { int st = vl > 48 ? vl - 48 : 0; for (int i = st; i < vl && di < 118; i++)disp[di++] = val[i]; }
    if (active)disp[di++] = '|'; disp[di] = '\0';
    sf::Text tv(f, disp, 22); tv.setFillColor(sf::Color::White); tv.setPosition({ x + 8,y + 36 }); w.draw(tv);
}

static void drawStatus(sf::RenderWindow& w, const sf::Font& f, const char* msg, bool ok) {
    if (!msg || !msg[0])return;
    drawRect(w, 50, 648, 1180, 44, ok ? sf::Color(0, 100, 50, 220) : sf::Color(120, 20, 20, 220));
    drawText(w, f, msg, 62, 660, 16, sf::Color::White);
}

static void drawSectionTitle(sf::RenderWindow& w, const sf::Font& f, float y, const char* txt) {
    drawText(w, f, txt, 50, y, 26, sf::Color::White, true);
    drawRect(w, 50, y + 36, 1180, 2, sf::Color(255, 255, 255, 60));
}

static void drawRow(sf::RenderWindow& w, const sf::Font& f, float y,
    const char* txt, sf::Color tc, sf::Color bg) {
    drawRect(w, 50, y, 1180, 44, bg); drawText(w, f, txt, 65, y + 12, 15, tc);
}

static void drawHint(sf::RenderWindow& w, const sf::Font& f) {
    drawText(w, f, "ESC = Back  |  ENTER = Confirm", 50, 702, 14, sf::Color(160, 160, 160));
}

// ════════════════════════════════════════════════════════════
//  main
// ════════════════════════════════════════════════════════════
int main() {
    sf::RenderWindow window(sf::VideoMode({ 1280u,720u }), "MediCore HMS - SFML 3.1");
    window.setFramerateLimit(60);
    sf::Font font;
    if (!font.openFromFile("InclusiveSans-Bold.ttf"))return -1;
    sf::Texture bgTex; bgTex.loadFromFile("bg.jpg"); sf::Sprite bg(bgTex);

    Storage<Patient>      patDB;
    Storage<Doctor>       docDB;
    Storage<Appointments> appDB;
    Storage<Bill>         billDB;
    FileHandler::loadAllPatients(patDB);
    FileHandler::loadAllDoctors(docDB);
    FileHandler::loadAllAppointments(appDB);
    FileHandler::loadAllBills(billDB);

    int  state = 0, menuSel = 0, bookStep = 0, attempts = 0;
    bool actionOk = false, locked = false;
    Patient* curPat = nullptr;
    Doctor* curDoc = nullptr;

    char buf1[512] = "", buf2[512] = "", buf3[512] = "", buf4[512] = "", buf5[512] = "";
    int  len1 = 0, len2 = 0, len3 = 0, len4 = 0, len5 = 0;
    char longBuf1[500] = "", longBuf2[300] = "";
    int  llen1 = 0, llen2 = 0;
    char statusMsg[300] = "";
    char loginID[50] = "", loginPw[50] = "";
    int  loginIDLen = 0, loginPwLen = 0, loginStep = 0;
    char uName[100] = "", uExtra[100] = "";

    auto clearBufs = [&]() {
        buf1[0] = buf2[0] = buf3[0] = buf4[0] = buf5[0] = '\0';
        longBuf1[0] = longBuf2[0] = '\0';
        len1 = len2 = len3 = len4 = len5 = llen1 = llen2 = 0;
        statusMsg[0] = '\0'; bookStep = 0; actionOk = false; };
    auto clearLogin = [&]() {
        loginID[0] = loginPw[0] = '\0';
        loginIDLen = loginPwLen = loginStep = 0;
        attempts = 0; locked = false; };
    auto setMsg = [&](const char* m, bool ok) {ms_cpy(m, statusMsg, 300); actionOk = ok; };

    while (window.isOpen()) {
        while (const std::optional<sf::Event> ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>())window.close();

            // ESC = back
            if (const auto* kp = ev->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::Escape) {
                    if (state == 1 || state == 2 || state == 3) { state = 0; clearLogin(); clearBufs(); }
                    else if (state >= 20 && state <= 26) { state = 11; clearBufs(); }
                    else if (state >= 40 && state <= 44) { state = 31; clearBufs(); }
                    else if (state >= 61 && state <= 69) { state = 51; clearBufs(); }
                }
            }

            // Text input
            if (const auto* te = ev->getIf<sf::Event::TextEntered>()) {
                if (te->unicode < 128) {
                    char c = static_cast<char>(te->unicode);

                    // LOGIN
                    if (state == 1 || state == 2 || state == 3) {
                        if (c == '\b') {
                            if (loginStep == 0 && loginIDLen > 0)loginID[--loginIDLen] = '\0';
                            else if (loginStep == 1 && loginPwLen > 0)loginPw[--loginPwLen] = '\0';
                        }
                        else if ((c == '\r' || c == '\n') && !locked) {
                            if (loginStep == 0 && loginIDLen > 0) { loginStep = 1; }
                            else if (loginStep == 1 && loginPwLen > 0) {
                                const char* role = (state == 1) ? "patient" : (state == 2) ? "doctor" : "admin";
                                try {
                                    Validator::validateUser(role, loginID, loginPw, uName, uExtra);
                                    if (state == 1) { curPat = patDB.findbyid(ms_toi(loginID)); state = 11; }
                                    else if (state == 2) { curDoc = docDB.findbyid(ms_toi(loginID)); state = 31; }
                                    else state = 51;
                                    menuSel = 0; attempts = 0; clearBufs(); statusMsg[0] = '\0';
                                }
                                catch (InvalidInputException& e) {
                                    attempts++;
                                    const char* r2 = (state == 1) ? "patient" : (state == 2) ? "doctor" : "admin";
                                    if (attempts >= 3) { locked = true; FileHandler::writeattempttosecuritylog(r2, loginID, "LOCKED"); }
                                    else FileHandler::writeattempttosecuritylog(r2, loginID, "FAILED");
                                    setMsg(e.what(), false);
                                    loginStep = 0; loginIDLen = loginPwLen = 0; loginID[0] = loginPw[0] = '\0';
                                }
                            }
                        }
                        else if (te->unicode >= 32) {
                            if (loginStep == 0 && loginIDLen < 49) { loginID[loginIDLen++] = c; loginID[loginIDLen] = '\0'; }
                            else if (loginStep == 1 && loginPwLen < 49) { loginPw[loginPwLen++] = c; loginPw[loginPwLen] = '\0'; }
                        }
                    }
                    // [PAT] BOOK APPOINTMENT
                    else if (state == 20) {
                        if (c == '\b') {
                            if (bookStep == 0 && len1 > 0)buf1[--len1] = '\0';
                            else if (bookStep == 1 && len2 > 0)buf2[--len2] = '\0';
                            else if (bookStep == 2 && len3 > 0)buf3[--len3] = '\0';
                        }
                        else if (c == '\r' || c == '\n') {
                            if (bookStep == 0 && len1 > 0) {
                                try { Validator::validateDoctorID(ms_toi(buf1), docDB); statusMsg[0] = '\0'; bookStep = 1; }
                                catch (InvalidInputException& e) { setMsg(e.what(), false); len1 = 0; buf1[0] = '\0'; }
                            }
                            else if (bookStep == 1 && len2 > 0) {
                                try { Validator::validateDate(buf2); statusMsg[0] = '\0'; bookStep = 2; }
                                catch (InvalidInputException& e) { setMsg(e.what(), false); len2 = 0; buf2[0] = '\0'; }
                            }
                            else if (bookStep == 2 && len3 > 0) {
                                int did = ms_toi(buf1); Doctor* doc = docDB.findbyid(did);
                                try {
                                    Validator::validateSlot(buf3, did, buf2, Appointments());
                                    if (slotTaken(appDB, did, buf2, buf3))throw SlotUnavailableException("Slot already taken.");
                                    if (!curPat || curPat->getbalance() < doc->getfee())throw InsufficientFundsException("Insufficient balance. Please top up.");
                                    int aid = FileHandler::generateID("appointments.txt");
                                    int bid = FileHandler::generateID("bills.txt");
                                    FileHandler::writenewappointment(aid, curPat->getid(), did, buf2, buf3, "pending");
                                    FileHandler::writenewbill(bid, curPat->getid(), aid, doc->getfee(), "unpaid", buf2);
                                    *curPat -= doc->getfee();
                                    FileHandler::updatepatientbalance(curPat->getid(), curPat->getbalance());
                                    Appointments na(aid, curPat->getid(), did, buf2, buf3, "pending"); appDB.add(na);
                                    Bill nb(bid, curPat->getid(), aid, doc->getfee(), "unpaid", buf2); billDB.add(nb);
                                    char ok[80] = "Booked! Appt ID: "; char ids[10]; ms_itos(aid, ids); ms_cat(ok, ids, 80);
                                    setMsg(ok, true); bookStep = 3;
                                }
                                catch (SlotUnavailableException& e) { setMsg(e.what(), false); len3 = 0; buf3[0] = '\0'; }
                                catch (InsufficientFundsException& e) { setMsg(e.what(), false); }
                                catch (InvalidInputException& e) { setMsg(e.what(), false); len3 = 0; buf3[0] = '\0'; }
                            }
                        }
                        else if (te->unicode >= 32) {
                            if (bookStep == 0 && len1 < 19 && c >= '0' && c <= '9') { buf1[len1++] = c; buf1[len1] = '\0'; }
                            else if (bookStep == 1 && len2 < 10 && (c == '-' || (c >= '0' && c <= '9'))) { buf2[len2++] = c; buf2[len2] = '\0'; }
                            else if (bookStep == 2 && len3 < 5) { buf3[len3++] = c; buf3[len3] = '\0'; }
                        }
                    }
                    // [PAT] CANCEL APPOINTMENT
                    else if (state == 21) {
                        if (c == '\b' && len1 > 0)buf1[--len1] = '\0';
                        else if ((c == '\r' || c == '\n') && len1 > 0) {
                            try {
                                Validator::validatePatienthasAppointment(ms_toi(buf1), curPat->getid(), appDB);
                                Appointments* a = appDB.findbyid(ms_toi(buf1));
                                Doctor* doc = docDB.findbyid(a->getdocid());
                                a->setstatus("cancelled");
                                FileHandler::updateappointmentstatus(a->getid(), "cancelled");
                                if (doc && curPat) { *curPat += doc->getfee(); FileHandler::updatepatientbalance(curPat->getid(), curPat->getbalance()); }
                                Bill* ba = billDB.getall(); int bs = billDB.getsize();
                                for (int i = 0; i < bs; i++)if (ba[i].getappid() == a->getid()) { FileHandler::updatebillstatus(ba[i].getid(), "cancelled"); ba[i].setstatus("cancelled"); break; }
                                char ok[80] = "Cancelled. Refund: PKR "; char fv[20]; if (doc)ms_ftos(doc->getfee(), fv); else ms_cpy("0", fv, 20);
                                ms_cat(ok, fv, 80); setMsg(ok, true); len1 = 0; buf1[0] = '\0';
                            }
                            catch (InvalidInputException& e) { setMsg(e.what(), false); len1 = 0; buf1[0] = '\0'; }
                        }
                        else if (c >= '0' && c <= '9' && len1 < 19) { buf1[len1++] = c; buf1[len1] = '\0'; }
                    }
                    // [PAT] PAY BILL
                    else if (state == 25) {
                        if (c == '\b' && len1 > 0)buf1[--len1] = '\0';
                        else if ((c == '\r' || c == '\n') && len1 > 0) {
                            try {
                                Validator::validatePatienthasBill(ms_toi(buf1), curPat->getid(), billDB);
                                Bill* b = billDB.findbyid(ms_toi(buf1));
                                if (!curPat || curPat->getbalance() < b->getamount())throw InsufficientFundsException("Insufficient balance.");
                                *curPat -= b->getamount();
                                FileHandler::updatepatientbalance(curPat->getid(), curPat->getbalance());
                                FileHandler::updatebillstatus(b->getid(), "paid");
                                b->setstatus("paid");
                                char ok[80] = "Paid! Remaining: PKR "; char fv[20]; ms_ftos(curPat->getbalance(), fv); ms_cat(ok, fv, 80);
                                setMsg(ok, true); len1 = 0; buf1[0] = '\0';
                            }
                            catch (InsufficientFundsException& e) { setMsg(e.what(), false); len1 = 0; buf1[0] = '\0'; }
                            catch (InvalidInputException& e) { setMsg(e.what(), false); len1 = 0; buf1[0] = '\0'; }
                        }
                        else if (c >= '0' && c <= '9' && len1 < 19) { buf1[len1++] = c; buf1[len1] = '\0'; }
                    }
                    // [PAT] TOP UP
                    else if (state == 26) {
                        if (c == '\b' && len1 > 0)buf1[--len1] = '\0';
                        else if ((c == '\r' || c == '\n') && len1 > 0) {
                            float amt = 0; bool dot = false; float frac = 0.1f;
                            for (int i = 0; buf1[i]; i++) { if (buf1[i] == '.') { dot = true; continue; }if (!dot)amt = amt * 10 + (buf1[i] - '0'); else { amt += (buf1[i] - '0') * frac; frac *= 0.1f; } }
                            try {
                                if (amt <= 0)throw InvalidInputException("Amount must be greater than 0.");
                                Validator::validatePositiveFloat(amt);
                                *curPat += amt;
                                FileHandler::updatepatientbalance(curPat->getid(), curPat->getbalance());
                                char ok[80] = "Balance updated: PKR "; char fv[20]; ms_ftos(curPat->getbalance(), fv); ms_cat(ok, fv, 80);
                                setMsg(ok, true); len1 = 0; buf1[0] = '\0';
                            }
                            catch (InvalidInputException& e) { setMsg(e.what(), false); len1 = 0; buf1[0] = '\0'; }
                        }
                        else if ((c >= '0' && c <= '9' || c == '.') && len1 < 19) { buf1[len1++] = c; buf1[len1] = '\0'; }
                    }
                    // [DOC] MARK COMPLETE
                    else if (state == 41) {
                        if (c == '\b' && len1 > 0)buf1[--len1] = '\0';
                        else if ((c == '\r' || c == '\n') && len1 > 0) {
                            try {
                                Validator::validateDoctorhasAppointmentPending(ms_toi(buf1), curDoc->getid(), appDB);
                                Appointments* a = appDB.findbyid(ms_toi(buf1));
                                a->setstatus("completed");
                                FileHandler::updateappointmentstatus(a->getid(), "completed");
                                setMsg("Appointment marked as completed.", true); len1 = 0; buf1[0] = '\0';
                            }
                            catch (InvalidInputException& e) { setMsg(e.what(), false); len1 = 0; buf1[0] = '\0'; }
                        }
                        else if (c >= '0' && c <= '9' && len1 < 19) { buf1[len1++] = c; buf1[len1] = '\0'; }
                    }
                    // [DOC] MARK NO-SHOW
                    else if (state == 42) {
                        if (c == '\b' && len1 > 0)buf1[--len1] = '\0';
                        else if ((c == '\r' || c == '\n') && len1 > 0) {
                            try {
                                Validator::validateDoctorhasAppointmentPending(ms_toi(buf1), curDoc->getid(), appDB);
                                Appointments* a = appDB.findbyid(ms_toi(buf1));
                                a->setstatus("no-show");
                                FileHandler::updateappointmentstatus(a->getid(), "no-show");
                                Bill* ba = billDB.getall(); int bs = billDB.getsize();
                                for (int i = 0; i < bs; i++)if (ba[i].getappid() == a->getid()) { FileHandler::updatebillstatus(ba[i].getid(), "cancelled"); ba[i].setstatus("cancelled"); break; }
                                setMsg("Marked as no-show. Bill cancelled.", true); len1 = 0; buf1[0] = '\0';
                            }
                            catch (InvalidInputException& e) { setMsg(e.what(), false); len1 = 0; buf1[0] = '\0'; }
                        }
                        else if (c >= '0' && c <= '9' && len1 < 19) { buf1[len1++] = c; buf1[len1] = '\0'; }
                    }
                    // [DOC] WRITE PRESCRIPTION
                    else if (state == 43) {
                        if (c == '\b') {
                            if (bookStep == 0 && len1 > 0)buf1[--len1] = '\0';
                            else if (bookStep == 1 && llen1 > 0)longBuf1[--llen1] = '\0';
                            else if (bookStep == 2 && llen2 > 0)longBuf2[--llen2] = '\0';
                        }
                        else if (c == '\r' || c == '\n') {
                            if (bookStep == 0 && len1 > 0) {
                                try { Validator::validateDoctorhasAppointmentCompletedNotDuplicated(ms_toi(buf1), curDoc->getid(), appDB); statusMsg[0] = '\0'; bookStep = 1; }
                                catch (InvalidInputException& e) { setMsg(e.what(), false); len1 = 0; buf1[0] = '\0'; }
                            }
                            else if (bookStep == 1 && llen1 > 0) { statusMsg[0] = '\0'; bookStep = 2; }
                            else if (bookStep == 2) {
                                Appointments* a = appDB.findbyid(ms_toi(buf1));
                                if (a) {
                                    int rid = FileHandler::generateID("prescriptions.txt");
                                    FileHandler::writePrescription(rid, a->getid(), a->getpatientid(), curDoc->getid(), a->getdate(), longBuf1, longBuf2);
                                    setMsg("Prescription saved.", true); bookStep = 3;
                                }
                            }
                        }
                        else if (te->unicode >= 32) {
                            if (bookStep == 0 && len1 < 19 && c >= '0' && c <= '9') { buf1[len1++] = c; buf1[len1] = '\0'; }
                            else if (bookStep == 1 && llen1 < 499) { longBuf1[llen1++] = c; longBuf1[llen1] = '\0'; }
                            else if (bookStep == 2 && llen2 < 299) { longBuf2[llen2++] = c; longBuf2[llen2] = '\0'; }
                        }
                    }
                    // [DOC] PATIENT HISTORY
                    else if (state == 44) {
                        if (c == '\b' && len1 > 0)buf1[--len1] = '\0';
                        else if ((c == '\r' || c == '\n') && len1 > 0) {
                            try { Validator::validateDoctorhasPatient(ms_toi(buf1), curDoc->getid(), appDB, patDB); statusMsg[0] = '\0'; bookStep = 1; }
                            catch (InvalidInputException& e) { setMsg(e.what(), false); len1 = 0; buf1[0] = '\0'; }
                        }
                        else if (c >= '0' && c <= '9' && len1 < 19) { buf1[len1++] = c; buf1[len1] = '\0'; }
                    }
                    // [ADM] ADD DOCTOR
                    else if (state == 61) {
                        if (c == '\b') {
                            if (bookStep == 0 && len1 > 0)buf1[--len1] = '\0';
                            else if (bookStep == 1 && len2 > 0)buf2[--len2] = '\0';
                            else if (bookStep == 2 && len3 > 0)buf3[--len3] = '\0';
                            else if (bookStep == 3 && len4 > 0)buf4[--len4] = '\0';
                            else if (bookStep == 4 && len5 > 0)buf5[--len5] = '\0';
                        }
                        else if (c == '\r' || c == '\n') {
                            if (bookStep == 0 && len1 > 0) { bookStep = 1; statusMsg[0] = '\0'; }
                            else if (bookStep == 1 && len2 > 0) { bookStep = 2; statusMsg[0] = '\0'; }
                            else if (bookStep == 2) {
                                try { Validator::validateContact(buf3); bookStep = 3; statusMsg[0] = '\0'; }
                                catch (InvalidInputException& e) { setMsg(e.what(), false); len3 = 0; buf3[0] = '\0'; }
                            }
                            else if (bookStep == 3) {
                                try { Validator::validatePassword(buf4); bookStep = 4; statusMsg[0] = '\0'; }
                                catch (InvalidInputException& e) { setMsg(e.what(), false); len4 = 0; buf4[0] = '\0'; }
                            }
                            else if (bookStep == 4 && len5 > 0) {
                                float fee = 0; bool dot = false; float frac = 0.1f;
                                for (int i = 0; buf5[i]; i++) { if (buf5[i] == '.') { dot = true; continue; }if (!dot)fee = fee * 10 + (buf5[i] - '0'); else { fee += (buf5[i] - '0') * frac; frac *= 0.1f; } }
                                try {
                                    Validator::validatePositiveFloat(fee);
                                    int nid = FileHandler::generateID("doctor.txt");
                                    FileHandler::writeDoctor(nid, buf1, buf2, buf3, buf4, fee);
                                    Doctor nd(nid, buf1, buf4, buf2, ms_toi(buf3), fee); docDB.add(nd);
                                    char ok[80] = "Doctor added. ID: "; char ids[10]; ms_itos(nid, ids); ms_cat(ok, ids, 80);
                                    setMsg(ok, true); bookStep = 5;
                                }
                                catch (InvalidInputException& e) { setMsg(e.what(), false); len5 = 0; buf5[0] = '\0'; }
                            }
                        }
                        else if (te->unicode >= 32) {
                            if (bookStep == 0 && len1 < 49) { buf1[len1++] = c; buf1[len1] = '\0'; }
                            else if (bookStep == 1 && len2 < 49) { buf2[len2++] = c; buf2[len2] = '\0'; }
                            else if (bookStep == 2 && len3 < 11 && c >= '0' && c <= '9') { buf3[len3++] = c; buf3[len3] = '\0'; }
                            else if (bookStep == 3 && len4 < 49) { buf4[len4++] = c; buf4[len4] = '\0'; }
                            else if (bookStep == 4 && len5 < 19 && (c >= '0' && c <= '9' || c == '.')) { buf5[len5++] = c; buf5[len5] = '\0'; }
                        }
                    }
                    // [ADM] REMOVE DOCTOR
                    else if (state == 62) {
                        if (c == '\b' && len1 > 0)buf1[--len1] = '\0';
                        else if ((c == '\r' || c == '\n') && len1 > 0) {
                            try {
                                Validator::validateDoctorID(ms_toi(buf1), docDB);
                                Validator::DoctorhasPendingAppointments(ms_toi(buf1));
                                Doctor* d = docDB.findbyid(ms_toi(buf1));
                                FileHandler::removeDoctor(ms_toi(buf1));
                                if (d)docDB.removebyid(*d);
                                setMsg("Doctor removed successfully.", true); len1 = 0; buf1[0] = '\0';
                            }
                            catch (InvalidInputException& e) { setMsg(e.what(), false); len1 = 0; buf1[0] = '\0'; }
                        }
                        else if (c >= '0' && c <= '9' && len1 < 19) { buf1[len1++] = c; buf1[len1] = '\0'; }
                    }
                    // [ADM] DISCHARGE PATIENT
                    else if (state == 67) {
                        if (c == '\b' && len1 > 0)buf1[--len1] = '\0';
                        else if ((c == '\r' || c == '\n') && len1 > 0) {
                            try {
                                int pid = ms_toi(buf1);
                                Patient* p = patDB.findbyid(pid);
                                if (!p)throw InvalidInputException("Patient not found.");
                                Validator::PatienthasUnpaidBills(pid, billDB);
                                Validator::PatienthasPendingAppointments(pid, appDB);
                                char gstr[3] = { p->getgender(),'\0' };
                                char cstr[20]; ms_itos(p->getcontact(), cstr);
                                FileHandler::writedischargedpatient(pid, p->getname(), p->getage(), gstr, cstr, p->getpass(), p->getbalance());
                                FileHandler::removePatient(pid);
                                patDB.removebyid(*p);
                                setMsg("Patient discharged and archived.", true); len1 = 0; buf1[0] = '\0';
                            }
                            catch (InvalidInputException& e) { setMsg(e.what(), false); len1 = 0; buf1[0] = '\0'; }
                        }
                        else if (c >= '0' && c <= '9' && len1 < 19) { buf1[len1++] = c; buf1[len1] = '\0'; }
                    }
                }
            }

            // Keyboard navigation
            if (const auto* kp = ev->getIf<sf::Event::KeyPressed>()) {
                if (state == 0) {
                    if (kp->code == sf::Keyboard::Key::Up) { menuSel--; if (menuSel < 0)menuSel = 3; }
                    if (kp->code == sf::Keyboard::Key::Down) { menuSel++; if (menuSel > 3)menuSel = 0; }
                    if (kp->code == sf::Keyboard::Key::Enter) {
                        clearBufs(); clearLogin(); statusMsg[0] = '\0';
                        if (menuSel == 0)state = 1; else if (menuSel == 1)state = 2; else if (menuSel == 2)state = 3; else window.close();
                    }
                }
                else if (state == 11) {
                    if (kp->code == sf::Keyboard::Key::Up) { menuSel--; if (menuSel < 0)menuSel = 7; }
                    if (kp->code == sf::Keyboard::Key::Down) { menuSel++; if (menuSel > 7)menuSel = 0; }
                    if (kp->code == sf::Keyboard::Key::Enter) {
                        clearBufs(); int dst[] = { 20,21,22,23,24,25,26,-1 };
                        if (menuSel == 7) { curPat = nullptr; clearLogin(); state = 0; menuSel = 0; }
                        else state = dst[menuSel];
                    }
                }
                else if (state == 31) {
                    if (kp->code == sf::Keyboard::Key::Up) { menuSel--; if (menuSel < 0)menuSel = 5; }
                    if (kp->code == sf::Keyboard::Key::Down) { menuSel++; if (menuSel > 5)menuSel = 0; }
                    if (kp->code == sf::Keyboard::Key::Enter) {
                        clearBufs(); int dst[] = { 40,41,42,43,44,-1 };
                        if (menuSel == 5) { curDoc = nullptr; clearLogin(); state = 0; menuSel = 0; }
                        else state = dst[menuSel];
                    }
                }
                else if (state == 51) {
                    if (kp->code == sf::Keyboard::Key::Up) { menuSel--; if (menuSel < 0)menuSel = 9; }
                    if (kp->code == sf::Keyboard::Key::Down) { menuSel++; if (menuSel > 9)menuSel = 0; }
                    if (kp->code == sf::Keyboard::Key::Enter) {
                        clearBufs(); int dst[] = { 61,62,63,64,65,66,67,68,69,-1 };
                        if (menuSel == 9) { clearLogin(); state = 0; menuSel = 0; }
                        else state = dst[menuSel];
                    }
                }
            }
        }

        // ════════════════════════════════════════════════════
        //  RENDER
        // ════════════════════════════════════════════════════
        window.clear(sf::Color(30, 30, 30));
        window.draw(bg);
        drawRect(window, 0, 0, 1280, 70, sf::Color(0, 0, 0, 160));
        drawText(window, font, "MediCore HMS", 30, 15, 34, sf::Color::White, true);
        if (state != 0) { drawRect(window, 20, 20, 100, 38, sf::Color(180, 30, 30)); drawCentredText(window, font, "BACK", 20, 20, 100, 38, 18, sf::Color::White); }

        const char* mpPat[] = { "1. Book Appointment","2. Cancel Appointment","3. View My Appointments",
                             "4. Medical Records","5. View Bills","6. Pay Bill","7. Top Up Balance","8. Logout" };
        const char* mpDoc[] = { "1. Today's Appointments","2. Mark Complete","3. Mark No-Show",
                             "4. Write Prescription","5. Patient History","6. Logout" };
        const char* mpAdm[] = { "1. Add Doctor","2. Remove Doctor","3. View All Patients","4. View All Doctors",
                             "5. View All Appointments","6. Unpaid Bills","7. Discharge Patient",
                             "8. Security Log","9. Daily Report","10. Logout" };

        // ROLE SELECT
        if (state == 0) {
            drawText(window, font, "WELCOME - SELECT YOUR ROLE", 370, 210, 35, sf::Color::White, true);
            const char* roleLabels[4] = { "PATIENT","DOCTOR","ADMIN","EXIT" };
            sf::Color roleCols[4] = { sf::Color(0,0,200,200),sf::Color(220,110,0,200),sf::Color(0,160,80,200),sf::Color(90,90,90,200) };
            for (int i = 0; i < 4; i++) {
                float x = (i % 2 == 0) ? 340.f : 660.f, y = (i < 2) ? 300.f : 420.f;
                sf::Color col = (menuSel == i) ? brighten(roleCols[i], 60) : roleCols[i];
                drawMenuBtn(window, font, x, y, 280.f, 80.f, roleLabels[i], col, menuSel == i);
            }
            drawText(window, font, "Click a button  or  UP/DOWN + ENTER", 390, 540, 18, sf::Color(200, 200, 200));
        }
        // LOGIN
        else if (state == 1 || state == 2 || state == 3) {
            const char* pname = (state == 1) ? "PATIENT LOGIN" : (state == 2) ? "DOCTOR LOGIN" : "ADMIN LOGIN";
            drawRect(window, 390, 230, 500, 330, sf::Color(0, 0, 0, 180), 2.f, sf::Color::White);
            drawText(window, font, pname, 500, 245, 28, sf::Color::White, true);
            drawRect(window, 410, 285, 460, 2, sf::Color(255, 255, 255, 80));
            drawInputField(window, font, 420, 295, 450, "Enter ID:", loginID, loginStep == 0);
            drawInputField(window, font, 420, 380, 450, "Enter Password:", loginPw, loginStep == 1, true);
            char atStr[60] = "Attempts Remaining: "; char an[5]; ms_itos(3 - attempts, an); ms_cat(atStr, an, 60);
            drawText(window, font, atStr, 420, 460, 18, locked ? sf::Color(255, 80, 80) : sf::Color(255, 220, 0));
            if (locked) { drawRect(window, 390, 490, 500, 44, sf::Color(130, 15, 15, 230)); drawText(window, font, "Account locked. Contact admin.", 405, 502, 16, sf::Color::White); }
            else if (statusMsg[0]) { drawRect(window, 390, 490, 500, 44, sf::Color(130, 15, 15, 230)); drawText(window, font, statusMsg, 405, 502, 15, sf::Color::White); }
            drawText(window, font, "ENTER to confirm each field  |  ESC = Back", 405, 543, 14, sf::Color(180, 180, 180));
        }
        // PATIENT DASHBOARD
        else if (state == 11) {
            char wlc[180] = "Welcome, "; ms_cat(wlc, curPat->getname(), 180);
            ms_cat(wlc, "   |   Balance: PKR ", 180); char bv[20]; ms_ftos(curPat->getbalance(), bv); ms_cat(wlc, bv, 180);
            drawText(window, font, wlc, 200, 95, 22, sf::Color(0, 240, 180));
            drawText(window, font, "PATIENT DASHBOARD", 480, 200, 28, sf::Color::White, true);
            sf::Color btnCols[8] = { sf::Color(0,0,200,190),sf::Color(180,80,0,190),sf::Color(0,150,70,190),sf::Color(100,0,180,190),
                                  sf::Color(0,120,160,190),sf::Color(160,120,0,190),sf::Color(20,100,40,190),sf::Color(100,20,20,190) };
            for (int i = 0; i < 8; i++) {
                float x = 200.f + (i % 2) * 450.f, y = 260.f + (i / 2) * 85.f;
                sf::Color col = (menuSel == i) ? brighten(btnCols[i], 60) : btnCols[i];
                drawMenuBtn(window, font, x, y, 400.f, 65.f, mpPat[i], col, menuSel == i);
            }
            drawText(window, font, "UP/DOWN + ENTER to select  |  ESC = Logout", 390, 615, 16, sf::Color(180, 180, 180));
        }
        // [PAT] BOOK APPOINTMENT
        else if (state == 20) {
            drawSectionTitle(window, font, 100, "BOOK APPOINTMENT");
            drawRect(window, 200, 155, 880, 430, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            if (bookStep == 0) { drawInputField(window, font, 220, 180, 840, "Doctor ID:", buf1, true); drawText(window, font, "Tip: View doctors from Admin panel to get valid IDs.", 220, 270, 15, sf::Color(180, 180, 180)); }
            else if (bookStep == 1) { char r[60] = "Doctor ID: "; ms_cat(r, buf1, 60); ms_cat(r, " - confirmed", 60); drawText(window, font, r, 220, 182, 18, sf::Color(0, 215, 145)); drawInputField(window, font, 220, 215, 840, "Date (DD-MM-YYYY):", buf2, true); }
            else if (bookStep == 2) {
                drawText(window, font, "Available slots: 09:00  10:00  11:00  12:00  13:00  14:00  15:00  16:00", 220, 175, 16, sf::Color(180, 220, 180));
                const char* slots[8] = { "09:00","10:00","11:00","12:00","13:00","14:00","15:00","16:00" };
                float sx = 220.f, sy = 200.f;
                for (int i = 0; i < 8; i++) {
                    bool tk = slotTaken(appDB, ms_toi(buf1), buf2, slots[i]);
                    drawRect(window, sx, sy, 130.f, 38.f, tk ? sf::Color(100, 10, 10, 200) : sf::Color(10, 80, 40, 200), 1.f, sf::Color(60, 60, 60));
                    drawCentredText(window, font, tk ? "TAKEN" : slots[i], sx, sy, 130.f, 38.f, 15, tk ? sf::Color(255, 80, 80) : sf::Color(0, 230, 140));
                    sx += 144.f; if (i == 3) { sx = 220.f; sy += 46.f; }
                }
                drawInputField(window, font, 220, 300, 260, "Time Slot:", buf3, true);
            }
            else if (bookStep == 3) { drawText(window, font, statusMsg, 220, 280, 20, sf::Color(0, 215, 145)); }
            drawStatus(window, font, bookStep < 3 ? statusMsg : "", actionOk); drawHint(window, font);
        }
        // [PAT] CANCEL APPOINTMENT
        else if (state == 21) {
            drawSectionTitle(window, font, 100, "CANCEL APPOINTMENT");
            drawRect(window, 50, 155, 1180, 450, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            float y = 175.f; bool found = false;
            Appointments* arr = appDB.getall(); int sz = appDB.getsize();
            for (int i = 0; i < sz; i++) {
                if (arr[i].getpatientid() != curPat->getid())continue;
                const char* s = arr[i].getstatus(); if (!s || s[0] != 'p')continue;
                char row[200]; char id[10]; ms_itos(arr[i].getid(), id);
                ms_cpy("Appt#", row, 200); ms_cat(row, id, 200); ms_cat(row, "  Dr:", 200);
                Doctor* d = docDB.findbyid(arr[i].getdocid()); ms_cat(row, d ? d->getname() : "?", 200);
                ms_cat(row, "  ", 200); ms_cat(row, arr[i].getdate(), 200); ms_cat(row, "  ", 200); ms_cat(row, arr[i].gettime(), 200);
                drawRow(window, font, y, row, sf::Color(255, 200, 80), sf::Color(30, 25, 0, 180)); y += 50.f; found = true; if (y > 500)break;
            }
            if (!found)drawText(window, font, "No pending appointments.", 70, 250, 20, sf::Color(160, 160, 160));
            drawInputField(window, font, 70, 530, 400, "Appointment ID to cancel:", buf1, true);
            drawStatus(window, font, statusMsg, actionOk); drawHint(window, font);
        }
        // [PAT] VIEW APPOINTMENTS
        else if (state == 22) {
            drawSectionTitle(window, font, 100, "MY APPOINTMENTS  (date ascending)");
            drawRect(window, 50, 155, 1180, 460, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            Appointments* arr = appDB.getall(); int sz = appDB.getsize();
            Appointments* sorted[100]; int cnt = 0;
            for (int i = 0; i < sz && cnt < 100; i++)if (arr[i].getpatientid() == curPat->getid())sorted[cnt++] = &arr[i];
            for (int i = 0; i < cnt - 1; i++)for (int j = 0; j < cnt - i - 1; j++) {
                const char* d1 = sorted[j]->getdate(), * d2 = sorted[j + 1]->getdate();
                int y1 = (d1[6] - '0') * 1000 + (d1[7] - '0') * 100 + (d1[8] - '0') * 10 + (d1[9] - '0'), m1 = (d1[3] - '0') * 10 + (d1[4] - '0'), day1 = (d1[0] - '0') * 10 + (d1[1] - '0');
                int y2 = (d2[6] - '0') * 1000 + (d2[7] - '0') * 100 + (d2[8] - '0') * 10 + (d2[9] - '0'), m2 = (d2[3] - '0') * 10 + (d2[4] - '0'), day2 = (d2[0] - '0') * 10 + (d2[1] - '0');
                if (y1 > y2 || (y1 == y2 && m1 > m2) || (y1 == y2 && m1 == m2 && day1 > day2)) { Appointments* t = sorted[j]; sorted[j] = sorted[j + 1]; sorted[j + 1] = t; }
            }
            float y = 175.f; bool found = false;
            for (int i = 0; i < cnt; i++) {
                char row[250]; char id[10]; ms_itos(sorted[i]->getid(), id);
                ms_cpy("ID:", row, 250); ms_cat(row, id, 250); ms_cat(row, "  Dr:", 250);
                Doctor* d = docDB.findbyid(sorted[i]->getdocid()); ms_cat(row, d ? d->getname() : "?", 250);
                if (d) { ms_cat(row, "(", 250); ms_cat(row, d->getspecial(), 250); ms_cat(row, ")", 250); }
                ms_cat(row, "  ", 250); ms_cat(row, sorted[i]->getdate(), 250); ms_cat(row, "  ", 250); ms_cat(row, sorted[i]->gettime(), 250);
                ms_cat(row, "  [", 250); ms_cat(row, sorted[i]->getstatus(), 250); ms_cat(row, "]", 250);
                const char* s = sorted[i]->getstatus();
                sf::Color tc = (s && s[0] == 'c' && s[1] == 'o') ? sf::Color(0, 215, 145) : (s && s[0] == 'p') ? sf::Color(255, 200, 0) : sf::Color(150, 150, 150);
                drawRow(window, font, y, row, tc, sf::Color(20, 30, 60, 180)); y += 50.f; found = true; if (y > 590)break;
            }
            if (!found)drawText(window, font, "No appointments found.", 70, 300, 20, sf::Color(160, 160, 160));
            drawHint(window, font);
        }
        // [PAT] MEDICAL RECORDS
        else if (state == 23) {
            drawSectionTitle(window, font, 100, "MEDICAL RECORDS");
            drawRect(window, 50, 155, 1180, 470, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            std::ifstream rf("prescriptions.txt"); float y = 175.f; bool found = false; char line[1024];
            rf.getline(line, 1024);
            while (rf.getline(line, 1024) && y < 600) {
                char* cols[10]; int nc = 0; cols[nc++] = line;
                for (int i = 0; line[i]; i++)if (line[i] == ',') { line[i] = '\0'; if (nc < 10)cols[nc++] = line + i + 1; }
                if (nc < 7 || ms_toi(cols[2]) != curPat->getid())continue;
                char row[300]; ms_cpy("Date:", row, 300); ms_cat(row, cols[4], 300);
                Doctor* d = docDB.findbyid(ms_toi(cols[3])); ms_cat(row, "  Dr:", 300); ms_cat(row, d ? d->getname() : "?", 300);
                ms_cat(row, "  Meds:", 300); ms_cat(row, cols[5], 300);
                drawRow(window, font, y, row, sf::Color(0, 215, 180), sf::Color(10, 40, 30, 180)); y += 26.f;
                char row2[200]; ms_cpy("Notes: ", row2, 200); ms_cat(row2, cols[6], 200);
                drawRow(window, font, y, row2, sf::Color(180, 220, 255), sf::Color(10, 20, 50, 180)); y += 38.f; found = true;
            }
            rf.close();
            if (!found)drawText(window, font, "No medical records found.", 70, 300, 20, sf::Color(160, 160, 160));
            drawHint(window, font);
        }
        // [PAT] VIEW BILLS
        else if (state == 24) {
            drawSectionTitle(window, font, 100, "MY BILLS");
            drawRect(window, 50, 155, 1180, 460, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            float y = 175.f, total = 0; bool found = false;
            Bill* ba = billDB.getall(); int bs = billDB.getsize();
            for (int i = 0; i < bs; i++) {
                if (ba[i].getpatientid() != curPat->getid())continue;
                char row[200]; char bid[10], aid[10], amt[20];
                ms_itos(ba[i].getid(), bid); ms_itos(ba[i].getappid(), aid); ms_ftos(ba[i].getamount(), amt);
                ms_cpy("Bill#", row, 200); ms_cat(row, bid, 200); ms_cat(row, "  Appt#", 200); ms_cat(row, aid, 200);
                ms_cat(row, "  PKR:", 200); ms_cat(row, amt, 200); ms_cat(row, "  [", 200); ms_cat(row, ba[i].getstatus(), 200); ms_cat(row, "]", 200);
                const char* s = ba[i].getstatus();
                sf::Color tc = (s && s[0] == 'p') ? sf::Color(0, 215, 145) : (s && s[0] == 'u') ? sf::Color(255, 200, 0) : sf::Color(150, 150, 150);
                drawRow(window, font, y, row, tc, sf::Color(20, 30, 60, 180)); y += 50.f; found = true;
                if (s && s[0] == 'u')total += ba[i].getamount(); if (y > 580)break;
            }
            if (!found)drawText(window, font, "No bills found.", 70, 300, 20, sf::Color(160, 160, 160));
            else {
                char tot[80] = "Total Outstanding: PKR "; char tv[20]; ms_ftos(total, tv); ms_cat(tot, tv, 80);
                drawRect(window, 50, 630, 1180, 42, sf::Color(60, 40, 0, 220)); drawText(window, font, tot, 65, 642, 18, sf::Color(255, 200, 80));
            }
            drawHint(window, font);
        }
        // [PAT] PAY BILL
        else if (state == 25) {
            drawSectionTitle(window, font, 100, "PAY BILL");
            drawRect(window, 200, 155, 880, 440, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            float y = 175.f; bool found = false;
            Bill* ba = billDB.getall(); int bs = billDB.getsize();
            for (int i = 0; i < bs; i++) {
                if (ba[i].getpatientid() != curPat->getid())continue;
                const char* s = ba[i].getstatus(); if (!s || s[0] != 'u')continue;
                char row[150]; char bid[10], amt[20]; ms_itos(ba[i].getid(), bid); ms_ftos(ba[i].getamount(), amt);
                ms_cpy("Bill#", row, 150); ms_cat(row, bid, 150); ms_cat(row, "  PKR:", 150); ms_cat(row, amt, 150); ms_cat(row, "  [unpaid]", 150);
                drawRow(window, font, y, row, sf::Color(255, 200, 0), sf::Color(20, 30, 60, 180)); y += 50.f; found = true; if (y > 480)break;
            }
            if (!found)drawText(window, font, "No unpaid bills.", 220, 300, 20, sf::Color(160, 160, 160));
            drawInputField(window, font, 220, 510, 400, "Bill ID to pay:", buf1, true);
            drawStatus(window, font, statusMsg, actionOk); drawHint(window, font);
        }
        // [PAT] TOP UP
        else if (state == 26) {
            drawSectionTitle(window, font, 100, "TOP UP BALANCE");
            drawRect(window, 300, 200, 680, 290, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            char balS[80] = "Current Balance: PKR "; char bv[20]; ms_ftos(curPat->getbalance(), bv); ms_cat(balS, bv, 80);
            drawText(window, font, balS, 320, 220, 20, sf::Color(0, 215, 180));
            drawInputField(window, font, 320, 260, 580, "Amount to add (PKR):", buf1, true);
            drawStatus(window, font, statusMsg, actionOk); drawHint(window, font);
        }
        // DOCTOR DASHBOARD
        else if (state == 31) {
            char wlc[160] = "Welcome, Dr. "; ms_cat(wlc, curDoc->getname(), 160);
            ms_cat(wlc, "   |   Specialization: ", 160); ms_cat(wlc, curDoc->getspecial(), 160);
            drawText(window, font, wlc, 200, 95, 22, sf::Color(0, 240, 180));
            drawText(window, font, "DOCTOR DASHBOARD", 480, 200, 28, sf::Color::White, true);
            sf::Color docCols[6] = { sf::Color(0,100,180,190),sf::Color(0,150,80,190),sf::Color(160,80,0,190),
                                  sf::Color(120,0,160,190),sf::Color(0,130,150,190),sf::Color(120,20,20,190) };
            for (int i = 0; i < 6; i++) {
                float x = 200.f + (i % 2) * 450.f, y = 260.f + (i / 2) * 90.f;
                sf::Color col = (menuSel == i) ? brighten(docCols[i], 60) : docCols[i];
                drawMenuBtn(window, font, x, y, 400.f, 70.f, mpDoc[i], col, menuSel == i);
            }
            drawText(window, font, "UP/DOWN + ENTER to select  |  ESC = Logout", 390, 615, 16, sf::Color(180, 180, 180));
        }
        // [DOC] TODAY'S APPOINTMENTS
        else if (state == 40) {
            drawSectionTitle(window, font, 100, "TODAY'S APPOINTMENTS  (sorted by time)");
            drawRect(window, 50, 155, 1180, 460, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            char today[12]; ms_today(today);
            Appointments* arr = appDB.getall(); int sz = appDB.getsize();
            Appointments* list[50]; int cnt = 0;
            for (int i = 0; i < sz && cnt < 50; i++)if (arr[i].getdocid() == curDoc->getid() && ms_eq(arr[i].getdate(), today))list[cnt++] = &arr[i];
            for (int i = 0; i < cnt - 1; i++)for (int j = 0; j < cnt - i - 1; j++) {
                const char* t1 = list[j]->gettime(), * t2 = list[j + 1]->gettime();
                int h1 = (t1[0] - '0') * 10 + (t1[1] - '0'), m1 = (t1[3] - '0') * 10 + (t1[4] - '0');
                int h2 = (t2[0] - '0') * 10 + (t2[1] - '0'), m2 = (t2[3] - '0') * 10 + (t2[4] - '0');
                if (h1 > h2 || (h1 == h2 && m1 > m2)) { Appointments* t = list[j]; list[j] = list[j + 1]; list[j + 1] = t; }
            }
            float y = 175.f; bool found = false;
            for (int i = 0; i < cnt; i++) {
                char row[200]; char id[10]; ms_itos(list[i]->getid(), id);
                ms_cpy("Appt#", row, 200); ms_cat(row, id, 200); ms_cat(row, "  Pat:", 200);
                Patient* p = patDB.findbyid(list[i]->getpatientid()); ms_cat(row, p ? p->getname() : "?", 200);
                ms_cat(row, "  ", 200); ms_cat(row, list[i]->gettime(), 200); ms_cat(row, "  [", 200); ms_cat(row, list[i]->getstatus(), 200); ms_cat(row, "]", 200);
                const char* s = list[i]->getstatus();
                sf::Color tc = (s && s[0] == 'p') ? sf::Color(255, 200, 0) : (s && s[0] == 'c' && s[1] == 'o') ? sf::Color(0, 215, 145) : sf::Color(150, 150, 150);
                drawRow(window, font, y, row, tc, sf::Color(20, 30, 60, 180)); y += 50.f; found = true; if (y > 590)break;
            }
            if (!found)drawText(window, font, "No appointments today.", 70, 300, 20, sf::Color(160, 160, 160));
            drawHint(window, font);
        }
        // [DOC] MARK COMPLETE
        else if (state == 41) {
            drawSectionTitle(window, font, 100, "MARK APPOINTMENT COMPLETE");
            drawRect(window, 300, 200, 680, 270, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            drawText(window, font, "Only today's pending appointments can be marked complete.", 320, 220, 15, sf::Color(200, 200, 200));
            drawInputField(window, font, 320, 252, 580, "Appointment ID:", buf1, true);
            drawStatus(window, font, statusMsg, actionOk); drawHint(window, font);
        }
        // [DOC] MARK NO-SHOW
        else if (state == 42) {
            drawSectionTitle(window, font, 100, "MARK NO-SHOW");
            drawRect(window, 300, 200, 680, 230, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            drawInputField(window, font, 320, 225, 580, "Appointment ID:", buf1, true);
            drawStatus(window, font, statusMsg, actionOk); drawHint(window, font);
        }
        // [DOC] WRITE PRESCRIPTION
        else if (state == 43) {
            drawSectionTitle(window, font, 100, "WRITE PRESCRIPTION");
            drawRect(window, 200, 155, 880, 400, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            if (bookStep == 0) { drawInputField(window, font, 220, 200, 500, "Completed Appointment ID:", buf1, true); }
            else if (bookStep == 1) { char r[60] = "Appt# "; ms_cat(r, buf1, 60); ms_cat(r, " - confirmed", 60); drawText(window, font, r, 220, 200, 18, sf::Color(0, 215, 145)); drawInputField(window, font, 220, 235, 840, "Medicines (semicolon-separated):", longBuf1, true); }
            else if (bookStep == 2) { drawInputField(window, font, 220, 200, 840, "Notes (max 300 chars):", longBuf2, true); }
            else { drawText(window, font, "Prescription saved successfully!", 220, 300, 22, sf::Color(0, 215, 145)); }
            drawStatus(window, font, statusMsg, actionOk); drawHint(window, font);
        }
        // [DOC] PATIENT HISTORY
        else if (state == 44) {
            drawSectionTitle(window, font, 100, "PATIENT MEDICAL HISTORY");
            drawRect(window, 200, 155, 880, 460, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            if (bookStep == 0) { drawInputField(window, font, 220, 200, 500, "Patient ID:", buf1, true); drawStatus(window, font, statusMsg, actionOk); }
            else {
                int pid = ms_toi(buf1);
                std::ifstream rf("prescriptions.txt"); float y = 175.f; bool found = false; char line[1024];
                rf.getline(line, 1024);
                while (rf.getline(line, 1024) && y < 590) {
                    char* cols[10]; int nc = 0; cols[nc++] = line;
                    for (int i = 0; line[i]; i++)if (line[i] == ',') { line[i] = '\0'; if (nc < 10)cols[nc++] = line + i + 1; }
                    if (nc < 7 || ms_toi(cols[2]) != pid || ms_toi(cols[3]) != curDoc->getid())continue;
                    char row[250]; ms_cpy("Date:", row, 250); ms_cat(row, cols[4], 250); ms_cat(row, "  Meds:", 250); ms_cat(row, cols[5], 250);
                    drawRow(window, font, y, row, sf::Color(0, 215, 180), sf::Color(10, 40, 30, 180)); y += 26.f;
                    char row2[200]; ms_cpy("Notes: ", row2, 200); ms_cat(row2, cols[6], 200);
                    drawRow(window, font, y, row2, sf::Color(180, 220, 255), sf::Color(10, 20, 50, 180)); y += 38.f; found = true;
                }
                rf.close();
                if (!found)drawText(window, font, "No prescriptions found.", 220, 350, 18, sf::Color(160, 160, 160));
            }
            drawHint(window, font);
        }
        // ADMIN DASHBOARD
        else if (state == 51) {
            drawText(window, font, "ADMIN PANEL  -  MediCore HMS", 380, 95, 24, sf::Color(0, 240, 180));
            drawText(window, font, "ADMIN DASHBOARD", 480, 190, 28, sf::Color::White, true);
            sf::Color admCols[10] = { sf::Color(0,120,60,190),sf::Color(160,30,30,190),sf::Color(0,90,160,190),sf::Color(0,110,180,190),
                                   sf::Color(100,60,0,190),sf::Color(140,100,0,190),sf::Color(80,0,120,190),
                                   sf::Color(40,80,100,190),sf::Color(0,100,100,190),sf::Color(100,20,20,190) };
            for (int i = 0; i < 10; i++) {
                float x = 200.f + (i % 2) * 450.f, y = 240.f + (i / 2) * 72.f;
                sf::Color col = (menuSel == i) ? brighten(admCols[i], 60) : admCols[i];
                drawMenuBtn(window, font, x, y, 400.f, 58.f, mpAdm[i], col, menuSel == i);
            }
            drawText(window, font, "UP/DOWN + ENTER to select", 490, 625, 16, sf::Color(180, 180, 180));
        }
        // [ADM] ADD DOCTOR
        else if (state == 61) {
            drawSectionTitle(window, font, 100, "ADD DOCTOR");
            drawRect(window, 200, 155, 880, 430, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            const char* steps[] = { "Full Name:","Specialization:","Contact (11 digits):","Password (min 6 chars):","Consultation Fee (PKR):" };
            const char* vals[] = { buf1,buf2,buf3,buf4,buf5 };
            for (int i = 0; i <= bookStep && i < 5; i++) {
                float y = 180.f + (float)i * 46.f;
                if (i < bookStep) { char r[100]; ms_cpy(steps[i], r, 100); ms_cat(r, " ", 100); ms_cat(r, vals[i], 100); drawText(window, font, r, 220, y, 16, sf::Color(0, 215, 145)); }
                else drawInputField(window, font, 220, y, 840, steps[i], (char*)vals[i], true, i == 3);
            }
            if (bookStep == 5)drawText(window, font, statusMsg, 220, 425, 18, sf::Color(0, 215, 145));
            drawStatus(window, font, statusMsg, actionOk); drawHint(window, font);
        }
        // [ADM] REMOVE DOCTOR
        else if (state == 62) {
            drawSectionTitle(window, font, 100, "REMOVE DOCTOR");
            drawRect(window, 50, 155, 1180, 460, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            float y = 175.f; Doctor* da = docDB.getall(); int ds = docDB.getsize();
            for (int i = 0; i < ds && y < 495; i++) {
                char row[150]; char id[10], fe[20]; ms_itos(da[i].getid(), id); ms_ftos(da[i].getfee(), fe);
                ms_cpy("ID:", row, 150); ms_cat(row, id, 150); ms_cat(row, "  ", 150); ms_cat(row, da[i].getname(), 150);
                ms_cat(row, "  ", 150); ms_cat(row, da[i].getspecial(), 150); ms_cat(row, "  Fee:PKR", 150); ms_cat(row, fe, 150);
                drawRow(window, font, y, row, sf::Color(0, 195, 255), sf::Color(20, 30, 60, 180)); y += 50.f;
            }
            drawInputField(window, font, 70, 520, 400, "Doctor ID to remove:", buf1, true);
            drawStatus(window, font, statusMsg, actionOk); drawHint(window, font);
        }
        // [ADM] VIEW ALL PATIENTS
        else if (state == 63) {
            drawSectionTitle(window, font, 100, "ALL PATIENTS");
            drawRect(window, 50, 155, 1180, 460, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            float y = 175.f; bool found = false;
            Patient* pa = patDB.getall(); int ps = patDB.getsize(); Bill* ba = billDB.getall(); int bs = billDB.getsize();
            for (int i = 0; i < ps; i++) {
                char row[260]; char id[10], ag[5], bal[20]; ms_itos(pa[i].getid(), id); ms_itos(pa[i].getage(), ag); ms_ftos(pa[i].getbalance(), bal);
                ms_cpy("ID:", row, 260); ms_cat(row, id, 260); ms_cat(row, "  ", 260); ms_cat(row, pa[i].getname(), 260);
                ms_cat(row, "  Age:", 260); ms_cat(row, ag, 260); char gs[3] = { pa[i].getgender(),'\0' }; ms_cat(row, "  G:", 260); ms_cat(row, gs, 260);
                ms_cat(row, "  Bal:PKR", 260); ms_cat(row, bal, 260);
                int ub = 0; for (int j = 0; j < bs; j++)if (ba[j].getpatientid() == pa[i].getid()) { const char* s = ba[j].getstatus(); if (s && s[0] == 'u')ub++; }
                if (ub) { char ubs[5]; ms_itos(ub, ubs); ms_cat(row, "  UnpaidBills:", 260); ms_cat(row, ubs, 260); }
                drawRow(window, font, y, row, sf::Color(0, 215, 145), sf::Color(20, 30, 60, 180)); y += 50.f; found = true; if (y > 590)break;
            }
            if (!found)drawText(window, font, "No patients registered.", 70, 300, 20, sf::Color(160, 160, 160));
            drawHint(window, font);
        }
        // [ADM] VIEW ALL DOCTORS
        else if (state == 64) {
            drawSectionTitle(window, font, 100, "ALL DOCTORS");
            drawRect(window, 50, 155, 1180, 460, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            float y = 175.f; bool found = false; Doctor* da = docDB.getall(); int ds = docDB.getsize();
            for (int i = 0; i < ds; i++) {
                char row[200]; char id[10], fe[20]; ms_itos(da[i].getid(), id); ms_ftos(da[i].getfee(), fe);
                ms_cpy("ID:", row, 200); ms_cat(row, id, 200); ms_cat(row, "  ", 200); ms_cat(row, da[i].getname(), 200);
                ms_cat(row, "  Spec:", 200); ms_cat(row, da[i].getspecial(), 200); ms_cat(row, "  Fee:PKR", 200); ms_cat(row, fe, 200);
                drawRow(window, font, y, row, sf::Color(0, 195, 255), sf::Color(20, 30, 60, 180)); y += 50.f; found = true; if (y > 590)break;
            }
            if (!found)drawText(window, font, "No doctors registered.", 70, 300, 20, sf::Color(160, 160, 160));
            drawHint(window, font);
        }
        // [ADM] VIEW ALL APPOINTMENTS
        else if (state == 65) {
            drawSectionTitle(window, font, 100, "ALL APPOINTMENTS  (date descending)");
            drawRect(window, 50, 155, 1180, 460, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            Appointments* arr = appDB.getall(); int sz = appDB.getsize();
            Appointments* sorted[100]; int cnt = 0;
            for (int i = 0; i < sz && cnt < 100; i++)sorted[cnt++] = &arr[i];
            for (int i = 0; i < cnt - 1; i++)for (int j = 0; j < cnt - i - 1; j++) {
                const char* d1 = sorted[j]->getdate(), * d2 = sorted[j + 1]->getdate();
                int y1 = (d1[6] - '0') * 1000 + (d1[7] - '0') * 100 + (d1[8] - '0') * 10 + (d1[9] - '0'), m1 = (d1[3] - '0') * 10 + (d1[4] - '0'), day1 = (d1[0] - '0') * 10 + (d1[1] - '0');
                int y2 = (d2[6] - '0') * 1000 + (d2[7] - '0') * 100 + (d2[8] - '0') * 10 + (d2[9] - '0'), m2 = (d2[3] - '0') * 10 + (d2[4] - '0'), day2 = (d2[0] - '0') * 10 + (d2[1] - '0');
                if (y1 < y2 || (y1 == y2 && m1 < m2) || (y1 == y2 && m1 == m2 && day1 < day2)) { Appointments* t = sorted[j]; sorted[j] = sorted[j + 1]; sorted[j + 1] = t; }
            }
            float y = 175.f; bool found = false;
            for (int i = 0; i < cnt; i++) {
                char row[260]; char id[10]; ms_itos(sorted[i]->getid(), id);
                ms_cpy("ID:", row, 260); ms_cat(row, id, 260); ms_cat(row, "  Pat:", 260);
                Patient* p = patDB.findbyid(sorted[i]->getpatientid()); ms_cat(row, p ? p->getname() : "?", 260);
                ms_cat(row, "  Dr:", 260); Doctor* d = docDB.findbyid(sorted[i]->getdocid()); ms_cat(row, d ? d->getname() : "?", 260);
                ms_cat(row, "  ", 260); ms_cat(row, sorted[i]->getdate(), 260); ms_cat(row, "  ", 260); ms_cat(row, sorted[i]->gettime(), 260);
                ms_cat(row, "  [", 260); ms_cat(row, sorted[i]->getstatus(), 260); ms_cat(row, "]", 260);
                const char* s = sorted[i]->getstatus();
                sf::Color tc = (s && s[0] == 'c' && s[1] == 'a') ? sf::Color(255, 80, 80) : (s && s[0] == 'p') ? sf::Color(255, 200, 0) : (s && s[0] == 'c' && s[1] == 'o') ? sf::Color(0, 215, 145) : sf::Color(150, 150, 150);
                drawRow(window, font, y, row, tc, sf::Color(20, 30, 60, 180)); y += 50.f; found = true; if (y > 590)break;
            }
            if (!found)drawText(window, font, "No appointments found.", 70, 300, 20, sf::Color(160, 160, 160));
            drawHint(window, font);
        }
        // [ADM] UNPAID BILLS
        else if (state == 66) {
            drawSectionTitle(window, font, 100, "UNPAID BILLS");
            drawRect(window, 50, 155, 1180, 460, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            float y = 175.f, total = 0; bool found = false;
            Bill* ba = billDB.getall(); int bs = billDB.getsize();
            for (int i = 0; i < bs; i++) {
                const char* s = ba[i].getstatus(); if (!s || s[0] != 'u')continue;
                char row[260]; char bid[10], amt[20]; ms_itos(ba[i].getid(), bid); ms_ftos(ba[i].getamount(), amt);
                Patient* p = patDB.findbyid(ba[i].getpatientid());
                ms_cpy("Bill#", row, 260); ms_cat(row, bid, 260); ms_cat(row, "  Pat:", 260); ms_cat(row, p ? p->getname() : "?", 260); ms_cat(row, "  PKR:", 260); ms_cat(row, amt, 260);
                bool overdue = false;
                try { Validator::isDateOverdue(ba[i].getdate()); }
                catch (InvalidInputException&) { overdue = true; }
                if (overdue)ms_cat(row, "  [OVERDUE]", 260);
                drawRow(window, font, y, row, overdue ? sf::Color(255, 100, 0) : sf::Color(255, 200, 0), sf::Color(20, 30, 60, 180));
                y += 50.f; total += ba[i].getamount(); found = true; if (y > 580)break;
            }
            if (!found)drawText(window, font, "No unpaid bills. All clear!", 70, 300, 20, sf::Color(0, 215, 145));
            else {
                char tot[80] = "Total Outstanding: PKR "; char tv[20]; ms_ftos(total, tv); ms_cat(tot, tv, 80);
                drawRect(window, 50, 630, 1180, 42, sf::Color(60, 40, 0, 220)); drawText(window, font, tot, 65, 642, 18, sf::Color(255, 200, 80));
            }
            drawHint(window, font);
        }
        // [ADM] DISCHARGE PATIENT
        else if (state == 67) {
            drawSectionTitle(window, font, 100, "DISCHARGE PATIENT");
            drawRect(window, 300, 200, 680, 290, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            drawText(window, font, "Warning: This archives all patient data permanently.", 320, 220, 15, sf::Color(255, 180, 0));
            drawInputField(window, font, 320, 255, 580, "Patient ID to discharge:", buf1, true);
            drawStatus(window, font, statusMsg, actionOk); drawHint(window, font);
        }
        // [ADM] SECURITY LOG
        else if (state == 68) {
            drawSectionTitle(window, font, 100, "SECURITY LOG");
            drawRect(window, 50, 155, 1180, 460, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            std::ifstream logf("security_log.txt"); float y = 175.f; bool found = false; char line[250];
            while (logf.getline(line, 250) && y < 590) { drawRow(window, font, y, line, sf::Color(255, 155, 80), sf::Color(40, 18, 5, 180)); y += 44.f; found = true; }
            logf.close();
            if (!found)drawText(window, font, "Security log is empty.", 70, 300, 20, sf::Color(160, 160, 160));
            drawHint(window, font);
        }
        // [ADM] DAILY REPORT
        else if (state == 69) {
            drawSectionTitle(window, font, 100, "DAILY REPORT");
            drawRect(window, 50, 155, 1180, 460, sf::Color(0, 0, 0, 160), 1.f, sf::Color(80, 80, 80));
            char today[12]; ms_today(today);
            int pen = 0, comp = 0, nos = 0, canc = 0;
            Appointments* arr = appDB.getall(); int sz = appDB.getsize();
            for (int i = 0; i < sz; i++) {
                if (!ms_eq(arr[i].getdate(), today))continue; const char* s = arr[i].getstatus();
                if (s[0] == 'p')pen++; else if (s[0] == 'c' && s[1] == 'o')comp++; else if (s[0] == 'n')nos++; else if (s[0] == 'c' && s[1] == 'a')canc++;
            }
            float rev = 0; Bill* ba = billDB.getall(); int bs = billDB.getsize();
            for (int i = 0; i < bs; i++) { const char* s = ba[i].getstatus(); if (s && s[0] == 'p')rev += ba[i].getamount(); }
            char l1[150]; ms_cpy("Today's Appointments:  Pending=", l1, 150); char t[10]; ms_itos(pen, t); ms_cat(l1, t, 150);
            ms_cat(l1, "  Completed=", 150); ms_itos(comp, t); ms_cat(l1, t, 150); ms_cat(l1, "  No-Show=", 150); ms_itos(nos, t); ms_cat(l1, t, 150); ms_cat(l1, "  Cancelled=", 150); ms_itos(canc, t); ms_cat(l1, t, 150);
            char l2[80]; ms_cpy("Revenue Collected (paid bills): PKR ", l2, 80); char rv[20]; ms_ftos(rev, rv); ms_cat(l2, rv, 80);
            drawRow(window, font, 175, l1, sf::Color(0, 215, 145), sf::Color(10, 40, 20, 180));
            drawRow(window, font, 225, l2, sf::Color(255, 200, 80), sf::Color(40, 30, 0, 180));
            drawRow(window, font, 275, "Patients with Outstanding Bills:", sf::Color(0, 195, 255), sf::Color(10, 20, 50, 180));
            float y = 320.f; Patient* pa = patDB.getall(); int ps = patDB.getsize();
            for (int i = 0; i < ps && y < 590; i++) {
                float owed = 0; for (int j = 0; j < bs; j++)if (ba[j].getpatientid() == pa[i].getid()) { const char* s = ba[j].getstatus(); if (s && s[0] == 'u')owed += ba[j].getamount(); }
                if (owed <= 0)continue;
                char row[120]; char ows[20]; ms_ftos(owed, ows); ms_cpy(pa[i].getname(), row, 120); ms_cat(row, "  Owes: PKR ", 120); ms_cat(row, ows, 120);
                drawRow(window, font, y, row, sf::Color(255, 120, 80), sf::Color(40, 10, 10, 180)); y += 48.f;
            }
            drawHint(window, font);
        }

        // Mouse: BACK button
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            sf::Vector2f m = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            sf::FloatRect backRect({ 20.f,20.f }, { 100.f,38.f });
            static bool wasDown = false;
            if (!wasDown && backRect.contains(m) && state != 0) {
                if (state == 1 || state == 2 || state == 3) { state = 0; clearLogin(); clearBufs(); }
                else if (state >= 20 && state <= 26) { state = 11; clearBufs(); }
                else if (state >= 40 && state <= 44) { state = 31; clearBufs(); }
                else if (state >= 61 && state <= 69) { state = 51; clearBufs(); }
                wasDown = true;
            }
            if (backRect.contains(m))wasDown = true; else wasDown = false;
        }

        // Mouse: Role buttons
        {
            static bool roleMouseWasDown = false;
            bool leftDown = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
            if (state == 0 && leftDown && !roleMouseWasDown) {
                sf::Vector2f m = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                for (int i = 0; i < 4; i++) {
                    float x = (i % 2 == 0) ? 340.f : 660.f, y = (i < 2) ? 300.f : 420.f;
                    sf::FloatRect r({ x,y }, { 280.f,80.f });
                    if (r.contains(m)) {
                        clearBufs(); clearLogin(); statusMsg[0] = '\0';
                        if (i == 3)window.close(); else { state = i + 1; menuSel = 0; }
                    }
                }
            }
            roleMouseWasDown = leftDown;
        }

        window.display();
    }
    return 0;
}
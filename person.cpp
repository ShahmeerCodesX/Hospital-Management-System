#include "person.h"

static int p_len(const char* a) {
    int count = 0;
    if (!a) return 0;
    while (a[count] != '\0') count++;
    return count;
}

static void p_copy(const char* a, char* b) {
    int l = p_len(a);
    for (int i = 0; i < l; i++) b[i] = a[i];
    b[l] = '\0';
}

Person::Person() {
    ID = 0;
    name = nullptr;
    pass = nullptr;
}

Person::Person(int id, char* n, char* p) {
    ID = id;
    name = new char[p_len(n) + 1];
    p_copy(n, name);
    pass = new char[p_len(p) + 1];
    p_copy(p, pass);
}

Person::Person(const Person& other) {
    ID = other.ID;
    if (other.name) {
        name = new char[p_len(other.name) + 1];
        p_copy(other.name, name);
    }
    else {
        name = nullptr;
    }
    if (other.pass) {
        pass = new char[p_len(other.pass) + 1];
        p_copy(other.pass, pass);
    }
    else {
        pass = nullptr;
    }
}

Person::~Person() {
    delete[] name;
    delete[] pass;
}
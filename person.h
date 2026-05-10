#pragma once
#include <iostream>

class Person {
protected:
    int    ID;
    char* name;
    char* pass;
public:
    Person();
    Person(int id, char* n, char* p);
    Person(const Person& other);
    virtual ~Person();

   
    virtual int   getid()   const = 0;
    virtual char* getname() = 0;

    virtual void displayProfile() const {}
};
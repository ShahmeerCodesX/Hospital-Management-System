#pragma once

template<typename T>
class Storage {
    T   data[100];
    int count;
public:
    Storage() : count(0) {}

    bool add(const T& item) {
        if (count >= 100) return false;
        data[count] = item;
        count++;
        return true;
    }

    bool addbyid(const T& newitem) {
        for (int i = 0; i < count; i++)
            if (newitem.getid() == data[i].getid()) return false;
        if (count >= 100) return false;
        data[count] = newitem;
        count++;
        return true;
    }

    bool removebyid(const T& item) {
        int index = -1;
        for (int i = 0; i < count; i++)
            if (item.getid() == data[i].getid()) { index = i; break; }
        if (index == -1) return false;
        for (int i = index; i < count - 1; i++)
            data[i] = data[i + 1];
        count--;
        return true;
    }

    T* findbyid(int id) {
        for (int i = 0; i < count; i++)
            if (id == data[i].getid()) return &data[i];
        return nullptr;
    }

    T* getall() { return data; }
    int getsize() { return count; }
};
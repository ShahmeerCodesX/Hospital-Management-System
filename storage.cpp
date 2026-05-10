#include "storage.h"

template <typename T>
Storage<T>::Storage() {
	count = 0;
}

template <typename T>
bool Storage<T>::add(const T& newitem) {
	if (count >= 100) return false;
	data[count] = newitem;
	count++;
	return true;
}

template <typename T>
bool Storage<T>::addbyid(const T& newitem) {
	for (int i = 0; i < count; i++) {
		if (newitem.getid() == data[i].getid()) {
			return false;
		}
	}
	data[count] = newitem;
	count++;
	return true;
}

template<typename T>
bool Storage<T>::removebyid(const T& item) {
	int index = -1;
	for (int i = 0; i < count; i++) {
		if (item.getid() == data[i].getid()) {
			index = i;
		}
	}
	if (index == -1) return false;
	for (int i = index; i < count - 1; i++) {
		data[i] = data[i + 1];
	}
	count--;
	return true;
}

template<typename T>
T* Storage<T>::findbyid(int id) {
	for (int i = 0; i < count; i++) {
		if (id == data[i].getid()) {
			return &data[i];
		}
	}
	return nullptr;
}

template<typename T>
T* Storage<T>::getall() {
	return data;
}

template<typename T>
int Storage<T>::getsize() {
	return count;
}
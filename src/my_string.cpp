#include "my_string.h"
#include "debug.h"
#include "memory.h"
#include "container.h"
#include "debug.h"

#include <string.h>
#include <stdlib.h>

#define STRING_MIN_SIZE 10

String
str_create(const char* text) {
	
	sizet len = strlen(text);

	String out;
	if (len < STRING_MIN_SIZE) {
		out.data = (char*)malloc(sizeof(char) * STRING_MIN_SIZE);
		out.capacity = STRING_MIN_SIZE;
	}
	else {
		out.data = (char*)malloc(sizeof(char) * len);
		out.capacity = len;
	}
	out.length = len;
	out.refCount = (i16*)malloc(sizeof(i16));
	(*out.refCount) = 1;
	memcpy(out.data, text, len * sizeof(char));
	ASSERT_MSG(out.data, "string malloc failed");
	ASSERT_MSG(out.refCount, "string malloc failed");

	return out;

}

String
str_create(sizet size) {
	
	String out;
	out.capacity = size;
	if (size < STRING_MIN_SIZE) {
		out.data = (char*)malloc(sizeof(char) * STRING_MIN_SIZE);
		out.capacity = STRING_MIN_SIZE;
	}
	else {
		out.data = (char*)malloc(sizeof(char) * size);
		out.capacity = size;
	}
	out.length = 0;
	out.data = (char*)malloc(sizeof(char) * size);
	out.refCount = (i16*)malloc(sizeof(i16));
	(*out.refCount) = 1;

	ASSERT_MSG(out.data, "string malloc failed");
	ASSERT_MSG(out.refCount, "string malloc failed");

	return out;
}

String
str_create(String& other) {
	
	String out = str_create(other.length);

	out.data = (char*)memcpy(out.data, other.data, other.length * sizeof(char));
	out.length = other.length;

	return out;
}

char&
String::operator[](sizet index) {

	return data[index];
}

b8
String::operator==(const String& str) {

	if (str.length == length) {

		for (sizet i = 0; i < length; ++i) {
			if (str.data[i] != data[i])
				return 0;
		}
		return 1;
	}

	return 0;
}

b8
String::operator==(const char* str) {

	sizet len = strlen(str);
	if (len == length) {

		for (sizet i = 0; i < length; ++i) {
			if (str[i] != data[i])
				return 0;
		}
		return 1;
	}

	return 0;
}

String
String::operator+(String& str) {
	
	for (sizet i = 0; i < str.length; ++i) {

		str_push(this, str[i]);
	}

	return *this;
}

char*
String::as_cstr() {

	if (length >= capacity) {
		capacity += 1;
		data = (char*)realloc(data, sizeof(char) * capacity);
	}
	data[length] = '\0';

	return data;
}

String&
String::operator=(const String& other) {

	if (this != &other) {
		
		if (refCount) {
			(*refCount)--;
			if (*refCount == 0) {

				free(refCount);
				free(data);
			}
		}

		data = other.data;
		refCount = other.refCount;
		capacity = other.capacity;
		length = other.length;

		if (refCount) 
			(*refCount) += 1;
	}

	return *this;
}


String::~String() {

	if (refCount) {
		(*refCount)--;
		if ((*refCount) == 0) {

			free(refCount);
			free(data);

			refCount = NULL;
			data = NULL;
		}
	}
}

String::String(const String& other) {

	data = other.data;
	refCount = other.refCount;
	capacity = other.capacity;
	length = other.length;

	if (refCount)
		(*refCount) += 1;

}

String::String(const char* cstr) {

	String str = str_create(cstr);
	data = str.data;
	refCount = str.refCount;
	capacity = str.capacity;
	length = str.length;

	(*refCount) += 1;

}

void
str_copy(String* dest, String* src) {

	ASSERT(dest->capacity >= src->capacity);

	for (sizet i = 0; i < src->length; ++i) {
		
		dest->data[i] = src->data[i];
	}

}

void
str_reverse(String* str) {

	sizet index = str->length - 1;
	for (sizet i = 0; i < str->length / 2; ++i) {

		char c = str->data[i];
		str->data[i] = str->data[index - i];
		str->data[index - i] = c;
	}
}


void
str_push(String* str, char c) {

	if (str->length >= str->capacity) {
		str->capacity *= 2;
		str->data = (char*)realloc(str->data, sizeof(char) * str->capacity);
	}

	str->data[str->length] = c;
	str->length++;
}

void
str_skip(String* str, sizet count) {
  
	str->length += count;
}


void
str_concat(String* s1, char* s2) {
	
	i32 len = strlen(s2);
	for (sizet i = 0; i < len; ++i) {
		str_push(s1, s2[i]);
	}

}

String
str_substring(String* str, sizet start, sizet end) {
	
	sizet len = end - start;
	String out = str_create(len);

	for (sizet i = start; i < end; ++i) {
		
		str_push(&out, str->data[i]);
	}

	return out;
}


i8
cstr_equal(const char* s1, const char* s2) {
	
	sizet i = 0;

	sizet len = strlen(s1);
	for (; i < len; ++i) {

		if (s1[i] != s2[i]) return 0;
	}

	if (s2[i] != '\0') {
		return 0;
	} else {
		return 1;
	}

}

void
str_clear(String* str) {

	str->length = 0;
}

void
str_free(String* str) {
	
	if (str->data) {
		
		ASSERT_MSG(*(str->refCount) > 1, "Deleting a string with refcount higher than 1");
		free(str->data);
		free(str->refCount);
		str->length = 0;
		str->capacity = 0;
		str->refCount = NULL;
		str->data = NULL;
	}
	else {
		WARN_MSG("Trying to delete already deleted string \n", NULL);
	}
}

void
str_add(String* to, String* from) {

	for (sizet i = 0; i < from->length; ++i) {

		str_push(to, (*from)[i]);
	}
}

void
str_print(String& str) {
	
	for (sizet i = 0; i < str.length; ++i) {
		printf("%c", str[i]);
	}
	printf("\n");
}


void
str_array_free(Array<String>& arr) {

	for (sizet i = 0; i < arr.length; ++i) {

		free(arr[i].data);
		free(arr[i].refCount);
		arr[i].length = 0;
		arr[i].capacity = 0;
		arr[i].refCount = NULL;
		arr[i].data = NULL;
	}

	array_free(&arr);
}


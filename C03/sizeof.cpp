//: C03:sizeof.cpp
// From Thinking in C++, 2nd Edition
// Available at http://www.BruceEckel.com
// (c) Bruce Eckel 2000
// Copyright notice in Copyright.txt
#include <iostream>
using namespace std;
int main() {
  cout << "sizeof(double) = " << sizeof(double);
  cout << ", sizeof(char) = " << sizeof(char) << endl;
  cout << "sizeof(int) = " << sizeof(int) << endl;
  cout << "sizeof(unsigned int) = " << sizeof(unsigned int) << endl;
  cout << "sizeof(short) = " << sizeof(short) << endl;
  cout << "sizeof(float) = " << sizeof(float) << endl;
  cout << "sizeof(unsigned char) = " << sizeof(unsigned char) << endl;
} ///:~

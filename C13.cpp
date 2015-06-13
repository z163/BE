//: C13:MallocClass.cpp
// Malloc with class objects
// What you'd have to do if not for "new"
#include <cassert>
#include <cstdlib> // malloc() & free()
#include <cstring> // memset()
#include <iostream>
using namespace std;

class Obj {
    int i, j, k;
    enum { sz = 100 };
    char buf[sz];
public:
    void initialize() { // Can't use constructor
        cout << "initializing Obj" << endl;
        i = j = k = 0;
        memset(buf, 0, sz);
    }
    void destroy() const { // Can't use destructor
        cout << "destroying Obj" << endl;
    }
};

int main() {
    Obj* obj = (Obj*)malloc(sizeof(Obj));
    assert(obj != 0);
    obj->initialize();
    // ... sometime later:
    obj->destroy();
    free(obj);
} ///:~
//initializing Obj
//destroying Obj
/**********************************************************/
//: C13:Tree.h
#ifndef TREE_H
#define TREE_H
#include <iostream>

class Tree {
    int height;
public:
    Tree(int treeHeight) : height(treeHeight) {}
    ~Tree() { std::cout << "*"; }
    friend std::ostream&
    operator<<(std::ostream& os, const Tree* t) {
        return os << "Tree height is: "
        << t->height << std::endl;
    }
};
#endif // TREE_H ///:~

//: C13:NewAndDelete.cpp
// Simple demo of new & delete
#include "Tree.h"
using namespace std;

int main() {
    Tree* t = new Tree(40);
    cout << t;
    delete t;
} ///:~
//Tree height is: 40
//*
/**********************************************************/
//: C13:BadVoidPointerDeletion.cpp
// Deleting void pointers can cause memory leaks
#include <iostream>
using namespace std;

class Object {
    void* data; // Some storage
    const int size;
    const char id;
public:
    Object(int sz, char c) : size(sz), id(c) {
        data = new char[size];
        cout << "Constructing object " << id
        << ", size = " << size << endl;
    }
    ~Object() {
        cout << "Destructing object " << id << endl;
        delete []data; // OK, just releases storage,
        // no destructor calls are necessary
    }
};

int main() {
    Object* a = new Object(40, 'a');
    delete a;
    void* b = new Object(40, 'b');
    delete b;
} ///:~
//Constructing object a, size = 40
//Destructing object a
//Constructing object b, size = 40
/**********************************************************/
//Stash for pointers
/**********************************************************/
//: C13:NewHandler.cpp
// Changing the new-handler
#include <iostream>
#include <cstdlib>
#include <new>
using namespace std;

int cnt = 0;

void out_of_memory() {
    cerr << "memory exhausted after " << cnt
    << " allocations!" << endl;
    exit(1);
}

int main() {
    set_new_handler(out_of_memory);
    while(1) {
        cnt++;
        new int[1000000000000]; // Exhausts memory
    }
} ///:~
//Test1(7692,0x7fff7dc85300) malloc: *** mach_vm_map(size=4000000000000) failed (error code=3)
//*** error: can't allocate region
//*** set a breakpoint in malloc_error_break to debug
//memory exhausted after 36 allocations!
/**********************************************************/
//: C13:GlobalOperatorNew.cpp
// Overload global new/delete
#include <cstdio>
#include <cstdlib>
#include <iostream>
using namespace std;

void* operator new(size_t sz) {
    printf("operator new: %d Bytes\n", sz);
    void* m = malloc(sz);
    if(!m) puts("out of memory");
    return m;
}

void operator delete(void* m) {
    puts("operator delete");
    cout << m << endl;
    free(m);
}

class S {
    int i[100];
public:
    S() { puts("S::S()"); }
    ~S() { puts("S::~S()"); }
};

int main() {
    puts("creating & destroying an int");
    int* p = new int(47);
    cout << p << endl;
    delete p;
    puts("creating & destroying an s");
    S* s = new S;
    cout << s << endl;
    delete s;
    puts("creating & destroying S[3]");
    S* sa = new S[3];
    cout << sa << endl;
    long* plong = (long*)sa;
    cout <<  *plong << endl; //not desired
    delete []sa;
    cout << sizeof(int*) << endl;
    cout << sizeof(long) << endl;
} ///:~
//creating & destroying an int
//operator new: 4 Bytes
//0x100204ba0
//operator delete
//0x100204ba0
//creating & destroying an s
//operator new: 400 Bytes
//S::S()
//0x100204ca0
//S::~S()
//operator delete
//0x100204ca0
//creating & destroying S[3]
//operator new: 1208 Bytes
//S::S()
//S::S()
//S::S()
//0x101002008
//0
//S::~S()
//S::~S()
//S::~S()
//operator delete
//0x101002000
//8
//8
/**********************************************************/
//: C13:Framis.cpp
// Local overloaded new & delete
#include <cstddef> // Size_t
#include <fstream>
#include <iostream>
#include <new>
using namespace std;
//ofstream out("Framis.out");
ostream& out = std::cout;

class Framis {
    enum { sz = 5 };
    char c[sz]; // To take up space, not used
    static unsigned char pool[];
    static bool alloc_map[];
public:
    enum { psize = 10 };  // frami allowed
    Framis() { out << "Framis()\n"; }
    ~Framis() { out << "~Framis() ... "; }
    void* operator new(size_t) throw(bad_alloc);
    void operator delete(void*);
};
unsigned char Framis::pool[psize * sizeof(Framis)];
bool Framis::alloc_map[psize] = {false};

// Size is ignored -- assume a Framis object
void* Framis::operator new(size_t) throw(bad_alloc) {
    for(int i = 0; i < psize; i++)
        if(!alloc_map[i]) {
            out << "using block " << i << " ... ";
            alloc_map[i] = true; // Mark it used
            return pool + (i * sizeof(Framis));
        }
    out << "out of memory" << endl;
    throw bad_alloc();
}

void Framis::operator delete(void* m) {
    if(!m) return; // Check for null pointer
    // Assume it was created in the pool
    // Calculate which block number it is:
    unsigned long block = (unsigned long)m
    - (unsigned long)pool;
    block /= sizeof(Framis);
    out << "freeing block " << block << endl;
    // Mark it free:
    alloc_map[block] = false;
}

int main() {
    
    Framis* f[Framis::psize];
    try {
        for(int i = 0; i < Framis::psize; i++)
            f[i] = new Framis;
        new Framis; // Out of memory
    } catch(bad_alloc) {
        cerr << "Out of memory!" << endl;
    }
    delete f[5];
    f[5] = 0;
    // Use released memory:
    Framis* x = new Framis;
    delete x;
    for(int j = 0; j < Framis::psize; j++)
        delete f[j]; // Delete f[10] OK
} ///:~
//using block 0 ... Framis()
//using block 1 ... Framis()
//using block 2 ... Framis()
//using block 3 ... Framis()
//using block 4 ... Framis()
//using block 5 ... Framis()
//using block 6 ... Framis()
//using block 7 ... Framis()
//using block 8 ... Framis()
//using block 9 ... Framis()
//out of memory
//Out of memory!
//~Framis() ... freeing block 5
//using block 5 ... Framis()
//~Framis() ... freeing block 5
//~Framis() ... freeing block 0
//~Framis() ... freeing block 1
//~Framis() ... freeing block 2
//~Framis() ... freeing block 3
//~Framis() ... freeing block 4
//~Framis() ... freeing block 6
//~Framis() ... freeing block 7
//~Framis() ... freeing block 8
//~Framis() ... freeing block 9
/**********************************************************/
//: C13:ArrayOperatorNew.cpp
// Operator new for arrays
#include <new> // Size_t definition
#include <fstream>
#include <iostream>
using namespace std;
//ofstream trace("ArrayOperatorNew.out");
ostream& trace = std::cout;

class Widget {
    enum { sz = 10 };
    int i[sz];
public:
    Widget() { trace << "*"; }
    ~Widget() { trace << "~"; }
    void* operator new(size_t sz) {
        trace << "Widget::new: "
        << sz << " bytes" << endl;
        return ::new char[sz];
    }
    void operator delete(void* p) {
        trace << "Widget::delete" << endl;
        cout << p << endl;
        ::delete []p;
    }
    void* operator new[](size_t sz) {
        trace << "Widget::new[]: "
        << sz << " bytes" << endl;
        return ::new char[sz];
        }
    void operator delete[](void* p) {
        trace << "Widget::delete[]" << endl;
        cout << p << endl;
        ::delete []p;
    }
};

int main() {
    trace << "new Widget" << endl;
    Widget* w = new Widget;
    cout << endl << w << endl;
    trace << "delete Widget" << endl;
    delete w;
    trace << "new Widget[25]" << endl;
    Widget* wa = new Widget[25];
    cout << endl << wa << endl;
    trace << "delete []Widget" << endl;
    delete []wa;
} ///:~
//new Widget
//Widget::new: 40 bytes
//*
//0x1001054c0
//delete Widget
//~Widget::delete
//0x1001054c0
//new Widget[25]
//Widget::new[]: 1008 bytes
//*************************
//0x100200008
//delete []Widget
//~~~~~~~~~~~~~~~~~~~~~~~~~Widget::delete[]
//0x100200000
/**********************************************************/
//: C13:NoMemory.cpp
// Constructor isn't called if new fails
#include <iostream>
#include <new> // bad_alloc definition
using namespace std;

class NoMemory {
public:
    NoMemory() {
        cout << "NoMemory::NoMemory()" << endl;
    }
    void* operator new(size_t sz) throw(bad_alloc){
        cout << "NoMemory::operator new" << endl;
        throw bad_alloc(); // "Out of memory"
    }
};

int main() {
    NoMemory* nm = 0;
    try {
        nm = new NoMemory;
    } catch(bad_alloc) {
        cerr << "Out of memory exception" << endl;
    }
    cout << "nm = " << nm << endl;
} ///:~
//NoMemory::operator new
//Out of memory exception
//nm = 0x0
/**********************************************************/
//: C13:PlacementOperatorNew.cpp
// Placement with operator new()
#include <cstddef> // Size_t
#include <iostream>
using namespace std;
        
class X {
    int i;
public:
    X(int ii = 0) : i(ii) {
        cout << "this = " << this << endl;
    }
    ~X() {
        cout << "X::~X(): " << this << endl;
    }
    void* operator new(size_t, void* loc) {
        return loc;
    }
};

int main() {
    int l[10];
    cout << "l = " << l << endl;
    X* xp = new(l) X(47); // X at location l
    xp->X::~X(); // Explicit destructor call
    // ONLY use with placement!
} ///:~
//l = 0x7fff5fbff7e0
//this = 0x7fff5fbff7e0
//X::~X(): 0x7fff5fbff7e0
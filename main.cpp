#include <iostream>
#include <memory>
#include "mempory_pool_mutex.h"
using namespace std;

class Person{
public:
    int id_;
    string name_;
public:
    Person(int id,string name):id_(id),name_(name){
        cout<<"构造person"<<endl;
    }
    ~Person(){
        cout<<"析构person"<<endl;
    }
};
void test01(){

    init_MemoryPool();
    cout<<"start test~~~~"<<endl;
    shared_ptr<Person> sp(newElement<Person>(11,"xiaoling"),deleteElement<Person>);
}
int main() {
    std::cout << "Hello, World!" << std::endl;
    test01();
    return 0;
}

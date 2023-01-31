#include <cstdio>
#include <iostream>
#include <ostream>
#include <sstream>
#include <list>
#include <map>

int main() {
    using namespace std;

    list<int> numbers_list({1,10,100,1000});
    list<const char *> vocals_list( {"a","e","i","o","u"} );
    list<std::string> word_list( {"test", "words", "make", "my", "list"} );

    numbers_list.push_front(0);             //insert in the beginning
    numbers_list.push_back(3000);           //insert in the end
    vocals_list.push_front("i");
    word_list.push_back("pushed back");

    for (int val : numbers_list)             // Loop
        cout << val << "  ";                 // Print function


    for (const char* val : vocals_list)           // Loop
        cout << val << "  ";                 // Print function

    for(std::string word : word_list)
        std::cout << word << " ";

    std::list<int> new_list({5,50,500});

    numbers_list.insert(numbers_list.end(),new_list.begin(),new_list.end());

    for (int val : numbers_list)
        std::cout << val << " ";

    std::map<std::string,int> girls_dictionary;
    girls_dictionary["Dolores"] = 30;
    girls_dictionary["Maeve"] = 27;
    girls_dictionary["Theresa"] = 6;
    girls_dictionary["Clementine"] = 11;

    for (auto item : girls_dictionary)
        std::cout << item.first << " appears in " << item.second << " episodes\n";


    int a = 42;
    printf("Value a is %i \n",a);            // Print an integer
    float b = 3.1415;
    printf("Value a is %f \n",b);            // Print a float
    std::string word = "Hey you!";
    std::cout << word;
    std::cout << word << std::endl;
    printf("- %s \n",word.c_str());           // Print a string

//     std::string name;
//     std::cout << "What's your name? ";
//     getline(std::cin, name);
// //     std::cin >> name;
//     std::cout << "Nice to meet you " << name << "!";

    // a has been declared above as an int so I mutated the value only.
    a = 4;
    int c = 3;

    cout << a+c << endl;
    cout << a-c << endl;
    cout << a*c << endl;
    cout << a/c << endl;
    cout << a%c << endl;
    return 0;
}
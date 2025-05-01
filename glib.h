#pragma once
#include <string>
#include <vector>

std::string getFunction(std::string substr);
std::string getNumber(const std::string &substr);
std::string getVariable(const std::string& substr);
int getPriority(const std::string &op);
void debugPrintNodes();

struct Glib
{
    std::string data;
    enum Type {
        Number = 1,
        Variable = 2, 
        Function = 3, 
        Operator = 4
    } type;
    int priority; // 0 for numbers, variables, functions; >0 for operators

    Glib* parent = nullptr;
    Glib* left = nullptr;
    Glib* right = nullptr;

    // Constructor
    Glib(const std::string& d, Type t, Glib* p = nullptr, Glib* l = nullptr, Glib* r = nullptr)
        : data(d), type(t), left(l), right(r), parent(p) {priority = (type == 4) ? getPriority(d) : 0;}
};

Glib* buildTree(const std::string& eq,
    Glib*              current   = nullptr,
    bool               parenthesis = false);
double compute(Glib *tree, std::vector<std::pair<std::string, double>> variables);
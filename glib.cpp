#include "glib.h"
#include <_ctype.h>
#include <string>
#include <iostream>
#include <vector>
#include <cmath>

std::vector<Glib*> g_allNodes;   // This variable contains all created nodes' addresses. For debug purpose only.

std::string getFunction(std::string substr) {
    int i = 0;
    std::string func;
    while(i < substr.length() && i < 4 && substr.at(i) != '(') {
        func += substr.at(i);
        i++;
    }

    // If it's not a known function, return ""
    if(!(func == "sin" || func == "cos" || func == "tan")) return "";
    return func;
}

/**
 * @brief Checks if the given string starts with a number or a dot, 
 * and if so, returns the numeric prefix. Otherwise, returns an empty string.
 * 
 * @param str The string to process.
 * @return The numeric prefix if found, or an empty string otherwise.
 */
std::string getNumber(const std::string &str) {
    size_t i = 0;
    while (i < str.size() && (isdigit(str[i]) || str[i] == '.')) {
        i++;
    }
    if (i == 0) return ""; // No number at start
    return str.substr(0, i);
}

std::string getOperator(const std::string &str) {
    char op = str[0];
    if(op == '*' || op == '/' || op == '+' || op == '-') return str.substr(0, 1);
    return "";
}

std::string getVariable(const std::string& str)
/* Returns the leading identifier when the first character is a letter
   and the next character is *not* a letter (so we don’t steal “sin”).   */
{
    if (!str.empty() && std::isalpha(str[0])) {
        if (str.size() == 1 || !std::isalpha(str[1]))
            return str.substr(0, 1);            // e.g. "x"
    }
    return "";
}

int getPriority(const std::string &op) {
    if(op.length() > 1) {
        throw std::invalid_argument("ERROR: " + op + " is not an operator");
        return 0;
    }
    if(op == "^") return 3;
    else if(op == "*" || op == "/") return 2;
    else if(op == "+" || op == "-") return 1;
    return 0;
}

// helper: climb to the top‑most parent so the caller always receives the real root
static Glib* getRoot(Glib* n)
{
    while (n && n->parent) n = n->parent;
    return n;
}

Glib* buildTree(const std::string& eq,
                Glib* current,
                bool parenthesis
)
{
    if (eq.empty()) return getRoot(current);

    // If we were inside a parenthesis pair and found a closing, we get out of them
    if (parenthesis && eq[0] == ')')
        return buildTree(eq.substr(1), getRoot(current), true);

    if (eq[0] == '(') // if we find an opening parenthesis, we enter the parenthesis
        return buildTree(eq.substr(1), current, true);

    std::string func = getFunction(eq);
    if (!func.empty()) {
        std::cout << "Function " << func << " found!\n";
        Glib *funcNode = new Glib(func, Glib::Type::Function, current);
        g_allNodes.push_back(funcNode);
        if(!current->left) {
            current->left = funcNode;
            funcNode->parent = current;
            return buildTree(eq.substr(func.size() + 1), funcNode, true);
        }
        else if(!current->right) {
            current->right = funcNode;
            funcNode->parent = current;
            return buildTree(eq.substr(func.size() + 1), funcNode, true);
        }
        else throw std::runtime_error("buildTree: current->left and current->right are already set");
        
        //return buildTree(eq.substr(func.size() + 1), funcNode, true);
    }

    std::string num = getNumber(eq);
    if (!num.empty()) {
        std::cout << "Number " << num << " found!\n";
        Glib* numNode = new Glib(num, Glib::Type::Number);
        g_allNodes.push_back(numNode);

        if (current) {
            /* attach to an operator expecting its left / right operand */
            if (current->type == Glib::Type::Operator) {
                if (!current->left)  { current->left  = numNode; numNode->parent = current; }
                else                 { current->right = numNode; numNode->parent = current; }
                /* continue parsing from the *operator* (not the new number) */
                return buildTree(eq.substr(num.size()), numNode, parenthesis);
            }
            /* attach as the argument of a function such as sin(...) */
            if (current->type == Glib::Type::Function && !current->left) {
                current->left = numNode;
                numNode->parent = current;
                return buildTree(eq.substr(num.size()), numNode, parenthesis);
            }
        }
        /* otherwise this number starts the expression */
        return buildTree(eq.substr(num.size()), numNode, parenthesis);
    }

    std::string var = getVariable(eq);
    if (!var.empty()) {
        Glib* varNode = new Glib(var, Glib::Type::Variable);
        g_allNodes.push_back(varNode);

        if (current) {
            if (current->type == Glib::Type::Operator) {
                if (!current->left)  { current->left  = varNode; varNode->parent = current; }
                else                 { current->right = varNode; varNode->parent = current; }
                return buildTree(eq.substr(var.size()), varNode, parenthesis);
            }
            if (current->type == Glib::Type::Function && !current->left) {
                current->left = varNode;
                varNode->parent = current;
                return buildTree(eq.substr(var.size()), varNode, parenthesis);
            }
        }
        return buildTree(eq.substr(var.size()), varNode, parenthesis);
    }

    std::string op = getOperator(eq);
    if (!op.empty()) {
        std::cout << "Operator " << op << " found!\n";
        if (!current) throw std::runtime_error("buildTree: expression cannot start with an operator");

        /* new operator becomes parent of the subtree rooted at `current` */
        Glib* parentNode = current->parent;
        Glib* opNode = new Glib(op, Glib::Type::Operator, parentNode, current);
        g_allNodes.push_back(opNode);

        /* splice opNode into the parent */
        if (parentNode) {
            if (parentNode->left  == current) parentNode->left  = opNode;
            if (parentNode->right == current) parentNode->right = opNode;
        }

        current->parent = opNode;

        return buildTree(eq.substr(op.size()), opNode, parenthesis);
    }

    throw std::runtime_error("buildTree: unexpected token near \"" + eq + "\"");
}

void debugPrintNodes()
{
    std::cout << "=== All Glib nodes (" << g_allNodes.size() << ") ===\n";
    for (const Glib* n : g_allNodes) {
        std::cout << n
                  << "  [" << n->data << "]  "
                  << "L:" << n->left
                  << " R:" << n->right
                  << " P:" << n->parent
                  << '\n';
    }
    std::cout << "=============================================\n";
}

double compute(Glib *tree, std::vector<std::pair<std::string, double>> variables) {
    if(tree->type == Glib::Type::Number) {
        return std::stod(tree->data);
    }
    else if(tree->type == Glib::Type::Variable) {
        // 1. Find the variable in the vector
        auto it = std::find_if(variables.begin(), variables.end(),
            [&tree](const std::pair<std::string, double>& var) {
                return var.first == tree->data;
            });
        // 2. If found, return its value
        if(it != variables.end()) {
            return it->second;
        }
        // 3. If not found, throw an error
        else {
            throw std::runtime_error("Variable " + tree->data + " not found in the provided variables.");
        }
    }
    else if(tree->type == Glib::Type::Function) {
        double arg = compute(tree->left, variables);
        // Convert degrees to radians
        arg = arg * M_PI / 180.0;
        if(tree->data == "sin") return sin(arg);
        else if(tree->data == "cos") return cos(arg);
        else if(tree->data == "tan") return tan(arg);
    }
    else if(tree->type == Glib::Type::Operator) {
        double left = compute(tree->left, variables);
        double right = compute(tree->right, variables);
        if(tree->data == "+") return left + right;
        else if(tree->data == "-") return left - right;
        else if(tree->data == "*") return left * right;
        else if(tree->data == "/") return left / right;
    }
}

void printTree(Glib *root, std::string prefix, bool isLeft) {
    if (root == nullptr) return; // If we've reached the end
    
    std::cout << prefix; // Print the prefix
    std::cout << (isLeft ? "├── " : "└── "); // Then print the graphic element depending on child location
    std::cout << "[" << root->data << "]" << std::endl; // And print the data

    // Make new prefix for children
    std::string newPrefix = prefix + (isLeft ? "│   " : "    ");

    // Recursively print left and right children
    if (root->left || root->right) { // Only print if at least one child exists
        printTree(root->left, newPrefix, true); 
        printTree(root->right, newPrefix, false);
    }
}
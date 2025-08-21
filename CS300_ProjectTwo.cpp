// ProjectTwo.cpp
// Author: Adryan Knight
// Course: CS 300 - ABCU Advising Assistance Program
//
// Description:
// Command-line C++ program that loads course data from a CSV file and supports:
//   1) Load the data into a binary search tree (BST)
//   2) Print an alphanumeric (A-Z/0-9) course list
//   3) Print information for one course (title + prerequisites)
//   9) Exit
//
// Notes:
// * Single CPP file as required. Uses only standard headers.
// * CSV format per assignment: courseNum,courseTitle[,prereq1,prereq2,...]
// * Input is case-insensitive for lookups; internal storage uses uppercase for IDs.

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <limits>

using namespace std;

// ---------- Utilities ----------

static inline string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static inline string toUpper(const string& s) {
    string out = s;
    transform(out.begin(), out.end(), out.begin(),
              [](unsigned char c){ return static_cast<char>(toupper(c)); });
    return out;
}

// ---------- Data Model ----------

struct Course {
    string id;                 // e.g., "CSCI200"
    string title;              // e.g., "Data Structures"
    vector<string> prereqs;    // e.g., {"CSCI101"}
};

struct Node {
    Course course;
    Node* left;
    Node* right;
    explicit Node(const Course& c) : course(c), left(nullptr), right(nullptr) {}
};

class CourseBST {
public:
    CourseBST() : root(nullptr) {}
    ~CourseBST() { clear(root); }

    // Disable copying to avoid accidental double-frees.
    CourseBST(const CourseBST&) = delete;
    CourseBST& operator=(const CourseBST&) = delete;

    // Minimal move support (not strictly needed, but safe if used).
    CourseBST(CourseBST&& other) noexcept : root(other.root) { other.root = nullptr; }
    CourseBST& operator=(CourseBST&& other) noexcept {
        if (this != &other) {
            clear(root);
            root = other.root;
            other.root = nullptr;
        }
        return *this;
    }

    void clearAll() {
        clear(root);
        root = nullptr;
    }

    void insert(const Course& c) {
        root = insertRec(root, c);
    }

    const Course* search(const string& id) const {
        Node* cur = root;
        string key = toUpper(id);
        while (cur != nullptr) {
            if (key == cur->course.id) return &(cur->course);
            if (key < cur->course.id) cur = cur->left;
            else cur = cur->right;
        }
        return nullptr;
    }

    void printInOrder() const {
        if (!root) {
            cout << "No courses loaded.\n";
            return;
        }
        inOrderRec(root);
    }

    bool empty() const { return root == nullptr; }

private:
    Node* root;

    static Node* insertRec(Node* node, const Course& c) {
        if (node == nullptr) {
            return new Node(c);
        }
        if (c.id == node->course.id) {
            node->course = c; // update if duplicate
        } else if (c.id < node->course.id) {
            node->left = insertRec(node->left, c);
        } else {
            node->right = insertRec(node->right, c);
        }
        return node;
    }

    static void inOrderRec(Node* node) {
        if (!node) return;
        inOrderRec(node->left);
        cout << node->course.id << ", " << node->course.title << '\n';
        inOrderRec(node->right);
    }

    static void clear(Node* node) {
        if (!node) return;
        clear(node->left);
        clear(node->right);
        delete node;
    }
};

// ---------- File Loading ----------

bool loadCoursesFromCSV(const string& filePath, CourseBST& bst, size_t& count) {
    ifstream infile(filePath);
    if (!infile) return false;

    count = 0;
    string line;
    while (getline(infile, line)) {
        line = trim(line);
        if (line.empty()) continue;

        vector<string> tokens;
        string token;
        stringstream ss(line);
        while (getline(ss, token, ',')) tokens.push_back(trim(token));

        if (tokens.size() < 2) continue; // require id and title

        Course c;
        c.id = toUpper(tokens[0]);
        c.title = tokens[1];
        for (size_t i = 2; i < tokens.size(); ++i) {
            if (!tokens[i].empty()) c.prereqs.push_back(toUpper(tokens[i]));
        }
        bst.insert(c);
        ++count;
    }
    return true;
}

void printCourse(const CourseBST& bst, const string& idInput) {
    string key = toUpper(trim(idInput));
    const Course* c = bst.search(key);
    if (!c) {
        cout << key << " was not found.\n";
        return;
    }

    cout << c->id << ", " << c->title << '\n';
    cout << "Prerequisites: ";
    if (c->prereqs.empty()) {
        cout << "None\n";
    } else {
        for (size_t i = 0; i < c->prereqs.size(); ++i) {
            cout << c->prereqs[i];
            if (i + 1 < c->prereqs.size()) cout << ", ";
        }
        cout << '\n';
    }
}

// ---------- Menu Loop ----------

void showMenu() {
    cout << '\n';
    cout << "1. Load Data Structure.\n";
    cout << "2. Print Course List.\n";
    cout << "3. Print Course.\n";
    cout << "9. Exit\n\n";
    cout << "What would you like to do? ";
}

int main() {
    cout << "Welcome to the course planner.\n";

    CourseBST bst;
    bool dataLoaded = false;

    while (true) {
        showMenu();

        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please type 1, 2, 3, or 9.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear rest of line

        if (choice == 1) {
            cout << "Enter the name of the data file: ";
            string filePath;
            getline(cin, filePath);

            bst.clearAll(); // reset any prior data

            size_t count = 0;
            if (loadCoursesFromCSV(filePath, bst, count)) {
                dataLoaded = !bst.empty();
                if (dataLoaded) {
                    cout << "Successfully loaded " << count << " line(s).\n";
                } else {
                    cout << "File contained no courses.\n";
                }
            } else {
                cout << "Error: could not open file '" << filePath << "'.\n";
            }
        } else if (choice == 2) {
            if (!dataLoaded) {
                cout << "Please load the data structure first (option 1).\n";
                continue;
            }
            cout << "Here is a sample schedule:\n\n";
            bst.printInOrder();
        } else if (choice == 3) {
            if (!dataLoaded) {
                cout << "Please load the data structure first (option 1).\n";
                continue;
            }
            cout << "What course do you want to know about? ";
            string id;
            getline(cin, id);
            printCourse(bst, id);
        } else if (choice == 9) {
            cout << "Thank you for using the course planner!\n";
            break;
        } else {
            cout << choice << " is not a valid option.\n";
        }
    }

    return 0;
}

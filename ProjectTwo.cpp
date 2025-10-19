/***************************************************************************
 * Author: Matthew
 * Date: 10/15/25
 * ABCU Course planner.
 * CLI Menu for the user to load and parse the data into the data structure.
 * It uses a hash table with chaining and dynamic resize.
 * Offers the option to print all of the data sorted or
 * search for a specific course and its prerequisites.
 ***************************************************************************/
// C++ headers
#include <algorithm>
#include <fstream> // ifstream, ofstream, fstream | file input/output
#include <iostream> // cin,cout,cerr,clog | input/output from console
#include <limits>
#include <set> 
#include <sstream> // for parsing string input
#include <string> 
#include <vector>
// C headers
#include <cctype> // char conversion; isdigit, letter, whitespace, etc.
#include <climits> // UINT_MAX sentinel, can be replaced.
#include <ctime> // clock

using namespace std;

//============================================================================
// Global helper utilities in unnamed namespace. Limited to this files scope for classes and methods. 
//============================================================================

namespace
{
    // default table size, prime number.
    constexpr unsigned int DEFAULT_SIZE = 31;

    // trim whitespace from text
    string trim(const string& text)
    {
        // remove leading spaces
        // finds index of first not a \tab, not a \newline, or a carriage \return
        size_t first = text.find_first_not_of(" \t\n\r");
        // if none, string is all whitespace, return empty string -- npos = sentinel for not found
        if (first == string::npos) return "";
        // remove trailing spaces, last non-whitespace
        size_t last = text.find_last_not_of(" \t\n\r");
        // return trimmed text -- the middle slice.
        return text.substr(first, (last - first + 1));
    }

    // split CSV line into fields
    vector<string> splitCSV(const string& line)
    {
        vector<string> result;
        // streaming string for formatting & parsing data
        stringstream parse(line);
        string field;

        // parse the line at commas into parts
        while (getline(parse, field, ','))
        {
            // for each part, trim it
            result.push_back(trim(field));
        }
        // return vector of trimmed parts
        return result;
    }

    // check if course string contains digit
    bool containsDigit(const string& course)
    {
	    // for each character in course
		for (char character : course)
		{
            if (isdigit(character)) return true;
            // if character is digit ('0' through '9')
                // return true
		}
        return false;
    }

	/**
	 * isPrime
	*
	* Checks if a number is prime.
	* @param num is the number it's checking.
	* @return true if it is, else false.
	*/
    bool isPrime(unsigned int num)
    {
        if (num <= 1) return false; // 0,1 aren't prime
        if (num <= 3) return true; // 2 and 3 are prime
        // next checking for numbers 4+, modulo, no remainder means not prime
        if (num % 2 == 0 || num % 3 == 0) return false;

        // starting at 5, check 6i +- 1 up to sqrt(num)
        // ex: i+=6 means i walks 5,11,17,23
        for (size_t i = 5; i <= num / i; i += 6)
        {
            // for each i, test i and i+2 (6k-1 and 6k+1) any prime > 3 is one of those.
            if (num % i == 0 || num % (i + 2) == 0) return false;
        }
        return true;
    }
    /**
     * nextPrime
     *
     * Picks the next prime number.
     * @param num is the number it's checking.
     * @return returns a prime number, 2 and 3,
     * otherwise it increments it to make it odd and calls isPrime.
     */
     // find next prime number >= num
    unsigned int nextPrime(unsigned int num)
    {
        if (num <= 2) return 2;
        // now for 3+, if it's even, increment it to make it odd.
        if (num % 2 == 0) num++;
        // while isPrime hasn't returned true, only check odd numbers.
        while (!isPrime(num))
        {
            num += 2;
        }
        return num; // returns the next prime
    }
}

/**
    * Structure for course data
    */
struct Course
{
    string courseNumber; // unique identifier -- key for hashing
    string name; // the course title/name
    vector<string> prerequisites; // list of prerequisite course numbers

    // default constructor
    Course() = default;
};

//============================================================================
// Hash Table class definition
//============================================================================

/**
 * Hash Table Class
 *
 * Contains the Node structure, initializes the vector containing the buckets.
 * Initializes the table size for the buckets, tracking of the total elements, and max chain length.
 * It has methods for loading the course data, parsing it, storing it in the data structure.
 * Then searching for a specific course, printing all courses, or resizing the table size if the bucket chain
 * becomes too long.
 */
class CourseHashTable
{
private:
	// Structure for node chaining
    struct Node
    {
        Course course; // local object for loading course data
        unsigned int key; // hash key value
        Node* next; // pointer to the next node in the chain
        // default constructor
        Node()
        {
            key = UINT_MAX;
            next = nullptr;
        }
        // constructor for resizing
        Node(Course aCourse, unsigned int aKey)
        {
            course = aCourse;
            key = aKey;
            next = nullptr;
        }
    };

    vector<Node> buckets;
    // these are set in the constructors. 
    size_t tableSize; // DEFAULT_SIZE is 31 for a small dataset.
	size_t numElements; // track total elements

    unsigned int maxChainLength = 4; // threshold for resizing

    unsigned int hash(const string& courseNumber) const;
    void reSize(); // dynamic resizing when chains get too long

public:
    CourseHashTable(); // default constructor
    CourseHashTable(unsigned int size); // constructor for resizing
    ~CourseHashTable(); // destructor

    void Insert(const Course& course);
    Course searchCourse(const string& courseNumber) const; 
    void printAll() const;
    void Clear(); 
    size_t Size() const { return numElements; }
};

/**
 * Default constructor
 * Creates a hash table with DEFAULT_SIZE (31) buckets.
 * And sets the numElements for counting.
 */
CourseHashTable::CourseHashTable() : tableSize(DEFAULT_SIZE), numElements(0)
{
    buckets.resize(tableSize);
}

/**
 * Size constructor
 * For resizing to a new table if too many chained nodes. (max chain length is 4 to cap worst cases)
 */
CourseHashTable::CourseHashTable(unsigned int size) : tableSize(size), numElements(0)
{
    buckets.resize(tableSize);
}

/**
 * Destructor
 * Frees all dynamically allocated memory in the chains
 */
CourseHashTable::~CourseHashTable() {
    for (size_t i = 0; i < tableSize; i++)
    {
        // start with the first chained node
        Node* current = buckets[i].next;
        // delete all linked nodes in the chain
        while (current != nullptr) {
            Node* temp = current;
            current = current->next;
            delete temp;
        }
    }
}

/**
 * Hash function
 * 
 * @param string courseNumber is passed, then it evaluates a new hash value
 * @return the new hash value
 */
unsigned int CourseHashTable::hash(const string& courseNumber) const
{
    // simple polynomial string hash works better to avoid issues like 101 being used for multiple courses
    unsigned int hashValue = 0;
    for (char currChar : courseNumber) {
        hashValue = hashValue * 31 + currChar;
    }
    return hashValue % tableSize;
}

/**
 * reSize
 * To automatically resize the hash table when chain length is too long.
 */

void CourseHashTable::reSize()
{
    cout << "Resizing hash table from " << tableSize << " to ";
    // decide the new size of the hash table
    unsigned int newSize = nextPrime(tableSize * 2);
    cout << newSize << " buckets.\n";
    // create the new table with new size
    CourseHashTable* newTable = new CourseHashTable(newSize);

    // rehash all elements
    for (size_t i = 0; i < tableSize; i++)
    {
	    if (buckets[i].key != UINT_MAX)
	    {
            newTable->Insert(buckets[i].course);
            Node* current = buckets[i].next;
            while (current != nullptr)
            {
                newTable->Insert(current->course);
                current = current->next;
            }
	    }
    }
    // swap the internals
    swap(buckets, newTable->buckets);
    swap(tableSize, newTable->tableSize);
    swap(numElements, newTable->numElements);
    // clean up old data that's now swapped
    delete newTable;
}


/**
 * Insert
 *
 * This is for loading data and inserting it into the data structure.
 * It checks the buckets and inserts it, traverses the chain until it finds an empty one.
 * 
 * @param It passes the course to hash the courseNumber and insert it.
 * @return Upon inserting the course, it returns. It increases the chain length and checks for resizing too.
 */

void CourseHashTable::Insert(const Course& course)
{
    unsigned int key = hash(course.courseNumber);
    // retrieve bucket location using hash key
    Node* node = &buckets.at(key);

    // if the head bucket/node is empty
    if (node->key == UINT_MAX)
    {
        // First course in this bucket direct insert 
        node->key = key;
        node->course = course; 
        node->next = nullptr;
        numElements++;
        return;
    }

    // update existing course
    if (node->course.courseNumber == course.courseNumber)
    {
        node->course = course;
        return;
    }
    // traverse chain
    unsigned int chainLength = 1; 
    while (node->next != nullptr)
    {
        chainLength++;
        node = node->next;
        if (node->course.courseNumber == course.courseNumber)
        {
            node->course = course;
            return;
        }
    }
    // add new node at end
    node->next = new Node(course, key);
    numElements++;
    chainLength++;

    // check if resize is needed
    if (chainLength > maxChainLength)
    {
        cout << "Chain length " << chainLength << " exceeds threshold.\n";
        reSize();
    }

}

/**
 * searchCourse
 *
 * Searches for a specific course, along with the prerequisites.
 * 
 * @param courseNumber to look up.
 * @return if found, the course, otherwise emptyCourse data, not found.
 */

Course CourseHashTable::searchCourse(const string& courseNumber) const
{
    Course emptyCourse; // empty course data to return if not found

    unsigned int key = hash(courseNumber);
    const Node* node = &buckets.at(key);

    // check if bucket is empty
    if (node->key == UINT_MAX)
    {
        // not found
        return emptyCourse;
    }
    // search through chain
    while (node != nullptr)
    {
	    if (node->course.courseNumber == courseNumber)
	    {
            // found
            return node->course;
	    }
        // go next
        node = node->next;
    }
    // not found
    return emptyCourse;
}

/**
 * printAll
 *
 * Prints all the courses in sorted order.
 *
 * @return
 */

void CourseHashTable::printAll() const
{
	// print all courses header
    cout << "\nCourse List:\n";
    cout << "============\n";
    // collect all courses
    vector<Course> allCourses;

    // iterate through all buckets
    for (size_t i = 0; i < tableSize; ++i)
    {
        // if the bucket key isn't empty
	    if (buckets[i].key != UINT_MAX)
	    {
            // add main node course
            allCourses.push_back(buckets[i].course);

            // add chained nodes
            const Node* node = buckets[i].next;
            while (node != nullptr)
            {
                allCourses.push_back(node->course);
                node = node->next;
            }

	    }
    }
    // sort courses by courseNumber, using std::sort with begin() and end()
    // then the predicate as the third argument to define the bool comparison
    sort(allCourses.begin(), allCourses.end(), [](const Course& a, const Course& b)
        {
            return a.courseNumber < b.courseNumber;
        });
    // print sorted courses
    // for each course in allCourses
	for (const Course& course: allCourses)
	{
        cout << course.courseNumber << ", " << course.name << endl;
	}
    cout << "\nTotal courses: " << allCourses.size() << endl;
}

/**
 * Clear
 *
 * Used to clear all data.
 */

void CourseHashTable::Clear()
{
	for (size_t i = 0; i < tableSize; ++i)
	{
        Node* current = buckets[i].next;
        while (current != nullptr)
        {
            Node* temp = current;
            current = current->next;
            delete temp;
        }
        buckets[i].key = UINT_MAX;
        buckets[i].course = Course();
        buckets[i].next = nullptr;
	}
    numElements = 0;
}


//============================================================================
// unnamed namespace continued for loadCourses and displayInformation
// Could be moved up top but requires forward declarations. I prefer this. 
//============================================================================

namespace 
{
	/**
	 * Load a CSV file containing course information into a container
	 *
	 * @param filePath the path to the CSV file to load, ht is a pointer to the hash table class.
	 * @return a container holding all the courses
	 */

    bool loadCourses(const string& filePath, CourseHashTable* ht)
    {
        set<string> courseNumbers; // track valid courses
        vector<Course> courses; // vector to track what's inserted for validation
        int lineNumber = 0; // track line numbers

        // open filePath for reading
        ifstream file(filePath);
        // if open fails
        if (!file.is_open())
        {
            cout << "Error: Could not open file " << filePath << endl;
            return false;
        }

        // print "Loading file path " + filePath
        cout << "Loading courses from " << filePath << endl;
        // start clean, clear existing data if there is any.
        ht->Clear();

        // read and parse lines and build course objects
        // for each line in file
        string line;
        while (getline(file, line))
        {
            lineNumber++;
            // if line is empty, continue to next line
            if (line.empty()) continue;

            vector<string> fields = splitCSV(line);

            // validation for minimum fields and non-empty fields (courseNumber and name)
            if (fields.size() < 2)
            {
                cout << "Error line " << lineNumber << ": Missing course number or name." << endl;
                file.close();
                ht->Clear();
                return false;
            }
            if (fields[0].empty() || fields[1].empty())
            {
                cout << "Error line " << lineNumber << ": Empty course number or name." << endl;
                file.close();
                ht->Clear();
                return false;
            }
            // build course object
            Course course;
            course.courseNumber = fields[0];
            course.name = fields[1];

            // add prerequisites
            for (size_t i = 2; i < fields.size(); i++)
            {
                const string& prereq = fields[i];
                if (prereq.empty()) continue;
                // validate prereqs format
                if (prereq.length() < 4 || !containsDigit(prereq))
                {
                    cout << "Error line " << lineNumber << ": Invalid prerequisite format '" << prereq << "'" << endl;
                    file.close();
                    ht->Clear();
                    return false;
                }
                course.prerequisites.push_back(prereq);
            }
            // check for duplicates
            if (courseNumbers.count(course.courseNumber) > 0)
            {
                cout << "Error line " << lineNumber << ": Duplicate course " << course.courseNumber << endl;
                file.close();
                ht->Clear();
                return false;
            }

            courseNumbers.insert(course.courseNumber);
            courses.push_back(course);
            ht->Insert(course);
        }
        file.close();

        // validate prerequisites exist
        for (const Course& course : courses)
        {
            for (const string& prereq : course.prerequisites)
            {
                if (courseNumbers.count(prereq) == 0)
                {
                    cout << "Error: Unknown prerequisite " << prereq << " for course " << course.courseNumber << endl;
                    ht->Clear();
                    return false;
                }
            }
        }

        cout << "Successfully loaded " << courseNumbers.size() << " courses.\n";
        return true;


    }

    /**
     * displayCourse information is used in case 3 from the main menu
     * to format output so it looks nice.
     *
     * @param course data to display.
     * @return the course information displayed properly.
     */

    void displayCourse(const Course& course) {
        cout << course.courseNumber << ", " << course.name << endl;
        if (!course.prerequisites.empty()) {
            cout << "Prerequisites: ";
            for (size_t i = 0; i < course.prerequisites.size(); i++) {
                cout << course.prerequisites[i];
                if (i < course.prerequisites.size() - 1) cout << ", ";
            }
            cout << endl;
        }
        else {
            cout << "No prerequisites" << endl;
        }
    }
}



//============================================================================
// Main function
//============================================================================

int main()
{
    CourseHashTable* courseTable = new CourseHashTable();
    string csvPath, courseNumber;
    clock_t ticks;

    cout << "Welcome to the Course Planner.\n";

    int choice = 0;
    while (choice != 9)
    {
        cout << "\n 1. Load Data Structure\n";
        cout << " 2. Print Course List\n";
        cout << " 3. Search and Print Course\n";
        cout << " 9. Exit\n";
        cout << "Enter your choice: \n";
        cin >> choice;
        if (cin.fail())
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please try again." << endl;
            continue;
        }
        cin.ignore();  // clear newline

        // get user choice from input
        switch (choice) {
        case 1: {
        		cout << "Enter CSV file path (or press Enter for default): ";
        		getline(cin, csvPath);

        		if (csvPath.empty()) {
        			csvPath = "CS 300 ABCU_Advising_Program_Input.csv";
        			cout << "Using default file: " << csvPath << endl;
        		}

        		ticks = clock();

        		bool loaded = loadCourses(csvPath, courseTable);
        		if (!loaded) {
        			cout << "Trying default file." << endl;
        			loaded = loadCourses("CS 300 ABCU_Advising_Program_Input.csv", courseTable);
        		}
        		if (loaded) {
        			ticks = clock() - ticks;
        			cout << "Data structure loaded." << endl;
        			cout << "Time: " << ticks << " clock ticks" << endl;
        			cout << "Time: " << ticks * 1.0 / CLOCKS_PER_SEC << " seconds" << endl;
        		}
        		else {
        			cout << "Failed to load courses." << endl;
        		}
        		break;
        }

        case 2:
            if (courseTable->Size() == 0) {
                cout << "No courses loaded. Please load data first." << endl;
            }
            else {
                courseTable->printAll();
            }
            break;

        case 3: {
        		if (courseTable->Size() == 0) {
        			cout << "No courses loaded. Please load data first." << endl;
        			break;
        		}

        		cout << "What course do you want to know about? ";
        		getline(cin, courseNumber);

        		if (courseNumber.empty()) {
        			cout << "Invalid input." << endl;
        			break;
        		}

        		transform(courseNumber.begin(), courseNumber.end(), courseNumber.begin(), ::toupper);
        		ticks = clock();
        		Course course = courseTable->searchCourse(courseNumber);
        		ticks = clock() - ticks;

        		if (!course.courseNumber.empty()) {
        			displayCourse(course);
        		}
        		else {
        			cout << "Course '" << courseNumber << "' not found." << endl;
        		}

        		cout << "Time: " << ticks << " clock ticks" << endl;
        		cout << "Time: " << ticks * 1.0 / CLOCKS_PER_SEC << " seconds" << endl;
        		break;
        }

        case 9:
            cout << "Thank you for using the course planner!" << endl;
            break;

        default:
            cout << choice << " is not a valid option, please try again." << endl;
            break;
        }
    }

    delete courseTable;
    return 0;
}
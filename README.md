# ProjectTwo

First Project was pseudocode + runtime analysis: [here](https://github.com/Matt0x90/ProjectTwo/blob/master/PseudocodeProject1.pdf).

The purpose of project two was to choose one of our pseudocode examples, then build a CLI course planner that intakes a CSV file of courses, stores them in a data structure, prints a sorted catalogue, and supports fast lookup of a course with its prerequisites included. Deciding which data structure to go with affects various facets, such as implementation difficulty, the runtime build speed, the method/function speed on how long it might take to load or print courses. All of this is greatly impacted by it.

I specifically went with a hash table because it offers 𝑂(1) average search, insertion, and deletion, which is great for the intended utility. It handles frequent look ups well. It has dynamic resizing to maintain performance and can scale well with larger course databases. It suits the project use case.

When deciding on how to implement it, I had to add a few things I missed/forgot about when doing the pseudocode compared to implementing it in C++, this included changing, or in most cases simplifying by using libraries like std in almost every method/helper.

I wrote a few helpers such as trim, splitCSV, containsDigit, isPrime, and nextPrime. When implementing trim and splitCSV I had to figure out how to do this with the string library. I was also able to simplify containsDigit and re-use isPrime and nextPrime from my previous assignment. I decided to structure my helpers in an unnamed namespace, which is continued in a section above main instead of all at top to avoid additional forward declarations.

I changed how I did the hash because if I simply trim the text, and courses have the same numbers like 101, it's not really solving anything, so a polynomial hash works better. I added an additional helper method called Clear() to delete all data, which I use when loading the courses, in case you want to load a different csv file or the file fails to parse properly. There's also displayCourse, which could be inside case 3, but doing it this way is cleaner. 

For printall, I simplified the sort by using std::sort. And also used std::transform so the course search is case insensitive inside the menu, that way it doesn't matter how the user types it in.

The biggest roadblock was CSV parsing details, trim and splitCSV required some digging into library functionality to get it working. I also know that it's not a proper CSV parser since it's going to have issues with quotations, but works for the data set. As previously mentioned, I changed how hashing was done since the previous method was overcomplicated and not helpful. 

If I were to re do this, I'd choose the simpler data structure setup that still meets the intended functionality and usage, which would be a basic vector and unordered map.

I've considered how I can improve this project, for example using std library more. Forward_list is a singly linked list I can use for each node, zero manual new/delete, works well for per bucket chains. Each bucket is a `forward_list<Course>`. Alternatively, ownership + unique/shared pointers instead of new/delete, no memory leak possibilities. Just use unique_ptr and make_unique, no destructor code to free chains, it happens automatically.

I could also use std::optional for searchCourse instead of returning a blank Course object sentinel, std::chrono instead of clock(), .reserve before collecting for sort, emplace_back instead of push_back, etc. C++ offers quite a lot of ways to do things and I'm still quite new to exploring it. 

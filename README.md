This project provides a macro-based implementation of a generic list (dynamic array) in C. The list supports various operations such as appending, inserting, sorting, slicing, and more.

## Table of Contents

- [Overview](#overview)
- [Importing the List](#importing-the-list)
- [Functionality](#functionality)
  - [Creating a New List](#creating-a-new-list)
  - [Appending Elements](#appending-elements)
  - [Accessing Elements](#accessing-elements)
  - [Inserting Elements](#inserting-elements)
  - [Removing Elements](#removing-elements)
  - [Sorting and Shuffling](#sorting-and-shuffling)
  - [Slicing the List](#slicing-the-list)
  - [Clearing and Destroying the List](#clearing-and-destroying-the-list)
- [License](#license)

## Overview

The generic list is implemented using macros that define the required types and functions for any data type. This allows you to create lists of any type without writing repetitive code.

## Importing the List

To use the list in your project, include the `list.h` header file and use the `IMPORT_LIST` macro to define a list for your desired type.

### Example:

```c
#include "list.h"

// Define a list of integers
IMPORT_LIST(int, intList)
```
This creates an `intList` type that is a pointer to a structure managing an array of integers.

## Functionality
### Creating a New List
To create a new list, use the `NEW` macro:
```c
intList my_list = NEW(intList);
```

### Appending Elements
To append an element to the list:
```c
if (my_list->append(my_list, 42) == 0) {
    printf("Appending was successful\n");
}
```

### Accessing Elements
To access an element at a specific index there are two methods:
```c
// Directly accessing the underlying array.
int i = 0;
int value = my_list->data[i];

// Using the supplied method.
int value;
if (my_list->get(my_list, 0, &value) == 0)
	printf("Element at index 0 is %d\n", value);
```
Setting elements is the same however it uses the `set` method.

### Inserting Elements
To insert an element:
```c
// Insert the element 99 at index 1.
if (my_list->insert(my_list, 1, 99) == 0) {
    printf("Insertion successful\n");
}
```

### Removing Elements
To remove (pop) an element at a specified index:
```c
int removed;
if (my_list->pop(my_list, 0, &remove) == 0) {
    printf("Removed the element %d\n", removed);
}
```

### Sorting and Shuffling
#### Sorting
```c
// Define a comparison function
int compare_ints(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

// Sort the values between the range 0 to the end of the list. 
// Both endpoints are inclusive.
my_list->sort(my_list, 0, my_list->len(my_list) - 1, compare_ints);
```

#### Shuffling
```c
// Shuffle the array between 0 and the end (inclusive).
my_list->shuffle(my_list, 0, my_list->len(my_list) - 1);
```

### List Slicing
Create a new list from a slice of the existing list.
```c
// slice(src_list, start, stop, end), endpoints are inclusive.
int_list sliced_list = my_list->slice(my_list, 1, 3, 1);
```

### Clearing and Destroying 
To clear the contents of the list:
```c
my_list->clear(my_list);
```
And to destroy the list and free all memory:
```c
my_list->destroy(my_list);
```

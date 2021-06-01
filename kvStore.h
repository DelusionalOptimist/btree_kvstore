#include "btree.h"
#include <cstdio>
#include <utility>
using namespace std;

// class for providing convenience while using BTrees for key value pairs
template <typename T1, typename T2> class kvStore {
public:
  // t is the minimum degree of the Btree which will be used for storing pairs.
  // compare is the comparison function used for managing elements within the
  // tree. printK is a function that prints keys.
  kvStore(unsigned t, bool (*)(pair<T1, T2>, pair<T1, T2>),
          void (*)(pair<T1, T2>) = 0);

  // Inserts a new pair into the tree
  // Logarithmic time
  void set(pair<T1, T2> k) { tree->insert(k); }

  // Prints all the key-value pairs stored in memory
  // Linear time
  void print() { tree->print(); }

  // Get a pair corresponding to the given key
  // Catches an exception if key not found
  // Logarithmic time
  pair<T1, T2> get(T1 k) {
    pair<T1, T2> value;
    try {
      value = tree->searchKey(pair<T1, T2>(k, NULL));
    } catch (...) {
      cout << "Entry not found";
    }
    return value;
  }

  // Remove a key value pair from memory
  // Returns false if could not remove successfully
  // Logarithmic time
  bool del(pair<T1, T2> k) {
    try {
      tree->remove(k);
      return true;
    } catch (char BTREE_EXCEPTION) {
      return false;
    }
  }

private:
  // Pointer to the comparison function for a pair of T1, T2
  bool (*lessThan)(pair<T1, T2>, pair<T1, T2>);

  // Pointer to the function for printing a key value pair
  void (*printKey)(pair<T1, T2>);

  // The degree of the BTree which will be used for creating the store
  unsigned degree;

  // The tree for storing Key-value pairs
  BTree<pair<T1, T2>> *tree;
};

template <typename T1, typename T2>
kvStore<T1, T2>::kvStore(unsigned t,
                         bool (*compare)(pair<T1, T2>, pair<T1, T2>),
                         void (*printK)(pair<T1, T2>)) {
  lessThan = compare;
  printKey = printK;
  degree = t;
  tree = new BTree<pair<T1, T2>>(t, lessThan, printKey);
}

// Template specialization when the "value" is of type string
template <typename T1> class kvStore<T1, string> {
public:
  // t is the minimum degree of the Btree which will be used for storing pairs
  // compare is the comparison function used for managing elements within the
  // tree. printK is a function that prints keys.
  kvStore(unsigned t, bool (*)(pair<T1, string>, pair<T1, string>),
          void (*)(pair<T1, string>) = 0);

  // Inserts a new pair into the tree
  void set(pair<T1, string> k) { tree->insert(k); }

  // Prints all the key-value pairs stored in memory
  void print() { tree->print(); }

  // Get a pair corresponding to the given key
  pair<T1, string> get(T1 k) {
    pair<T1, string> value;
    try {
      value = tree->searchKey(pair<T1, string>(k, ""));
    } catch (...) {
      cout << "Entry not found" << endl;
    }
    return value;
  }

  // Remove a key value pair from memory
  bool del(pair<T1, string> k) {
    try {
      tree->remove(k);
      return true;
    } catch (BTREE_EXCEPTION) {
      return false;
    }
  }

private:
  // Pointer to the comparison function for a pair of T1, string
  bool (*lessThan)(pair<T1, string>, pair<T1, string>);

  // Pointer to the function for printing a key value pair
  void (*printKey)(pair<T1, string>);

  // The degree of the BTree which will be used for creating the store
  unsigned degree;

  // The tree for storing Key-value pairs
  BTree<pair<T1, string>> *tree;
};

template <typename T1>
kvStore<T1, string>::kvStore(unsigned t,
                             bool (*compare)(pair<T1, string>,
                                             pair<T1, string>),
                             void (*printK)(pair<T1, string>)) {
  lessThan = compare;
  printKey = printK;
  degree = t;
  tree = new BTree<pair<T1, string>>(t, lessThan, printKey);
}

#include <iostream>
using namespace std;

#define SEARCH_KEY_NOT_FOUND 's'
#define REMOVE_KEY_NOT_FOUND 'r'
#define NEW_ROOT 2
#define MODIFIED_NOT_ROOT 1
#define NOT_MODIFIED 0

// struct for nodes of a b tree
template <typename T> struct BNode {
  BNode<T> **child; // Array of pointers to children.
  T *key;           // Array of keys.
  unsigned size;    // Number of keys.
  bool leaf;        // Whether the node is a leaf.
};

typedef char BTREE_EXCEPTION;

// class for b trees.
template <typename T> class BTree {
public:
  // Constructor
  // First parameter is the minimum degree of the tree.
  // Second parameter is the tree's key-comparison function.
  // Third parameter is a function that prints keys.
  // Constant time.
  BTree(unsigned, bool (*)(T, T), void (*)(T) = 0);

  // Destructor.
  // Linear time.
  ~BTree<T>();

  // Inserts a key into the tree.
  // Logorithmic time.
  void insert(T);

  // Removes a key from the tree.
  // Throws a BTREE_EXCEPTION if no item was found to remove.
  // Logorithmic time.
  T remove(T);

  // Function to find a key in the tree.
  // returnValue.first is the node the item is in.
  // returnValue.second is the correct index in that node's key array
  // Logorithmic time.
  pair<BNode<T> *, unsigned> search(T);

  // Uses search but just returns the key rather than the whole node.
  // Useful when T is a key value pair and lessThan only looks at the key.
  // Throws a BTREE_EXCEPTION if no item matching the parameter is found
  // Logorithmic time.
  T searchKey(T);

  // Prints the tree.
  // Linear time
  void print();

private:
  // Used for initializing nodes.
  void initializeNode(BNode<T> *);

  // Recursive function called by destructor.
  void freeNode(BNode<T> *);

  // Finds the index of a key in a node.
  unsigned findIndex(BNode<T> *, T);

  // Inserts a key into a node.
  unsigned nodeInsert(BNode<T> *, T);

  // Deletes the key at a given index from a node.
  T nodeDelete(BNode<T> *, unsigned);

  // Function for splitting nodes that are too full.
  void splitChild(BNode<T> *, int);

  // Merges two children of a node at a given index into one child.
  char mergeChildren(BNode<T> *, unsigned);

  // Makes sure the child of a node at a specified index has >= minDegree items.
  char fixChildSize(BNode<T> *, unsigned);

  // Recursively prints a subtree.
  void printNode(BNode<T> *);

  // Root node.
  BNode<T> *root;

  // Comparison function used for managing element placement.
  bool (*lessThan)(T, T);

  // Function used to print items in the tree.
  void (*printKey)(T);

  // Minimum degree of the tree.
  unsigned minDegree;
};
// Constructor for b tree.
// t is the minimum degree of the tree.
// compare is the comparison function used for managing elements within the
// tree. printK is a function that prints keys.
template <typename T>
BTree<T>::BTree(unsigned t, bool (*compare)(T, T), void (*printK)(T)) {
  minDegree = t;
  lessThan = compare;
  root = new BNode<T>;
  initializeNode(root);
  root->leaf = true;
  printKey = printK;
}

// Destructor.
template <typename T> BTree<T>::~BTree<T>() { freeNode(root); }

// Inserts the key k into the tree.
template <typename T> void BTree<T>::insert(T k) {

  // Grow upwards if the root is full.
  if (root->size == 2 * minDegree - 1) {
    BNode<T> *newRoot = new BNode<T>;
    initializeNode(newRoot);
    newRoot->leaf = false;
    newRoot->child[0] = root;
    root = newRoot;
    splitChild(newRoot, 0);
  }

  // Work down the tree.
  BNode<T> *curr = root;
  while (!curr->leaf) {

    // Find the proper child to go to.
    int index = curr->size - 1;
    while (index >= 0 && lessThan(k, curr->key[index])) {
      index--;
    }
    index++;

    // Split child if full.
    if (curr->child[index]->size == 2 * minDegree - 1) {
      splitChild(curr, index);
      if (lessThan(curr->key[index], k)) {
        index++;
      }
    }
    curr = curr->child[index];
  }

  nodeInsert(curr, k);
}

// Removes k from the tree. Returns the removed key.
// Throws a BTREE_EXCEPTION if key is not found.
template <typename T> T BTree<T>::remove(T k) {
  BNode<T> *curr = root;
  while (true) {
    unsigned i = findIndex(curr, k);

    // If the item to be deleted has been found.
    if (i < curr->size &&
        !(lessThan(curr->key[i], k) || lessThan(k, curr->key[i]))) {
      T toReturn = curr->key[i];

      // If at a leaf, just delete it.
      if (curr->leaf) {
        nodeDelete(curr, i);
      }

      // Otherwise replace with predecessor/successor or merge children.
      else {
        BNode<T> *leftKid = curr->child[i];
        BNode<T> *rightKid = curr->child[i + 1];

        // Replace with predecessor.
        if (leftKid->size >= minDegree) {
          while (!(leftKid->leaf)) {
            fixChildSize(leftKid, leftKid->size);
            leftKid = leftKid->child[leftKid->size];
          }
          curr->key[i] = nodeDelete(leftKid, leftKid->size - 1);
        }

        // Replace with successor
        else if (rightKid->size >= minDegree) {
          while (!(rightKid->leaf)) {
            fixChildSize(rightKid, 0);
            rightKid = rightKid->child[0];
          }
          curr->key[i] = nodeDelete(rightKid, 0);
        }

        // Merge children and move down the tree.
        else {
          mergeChildren(curr, i);
          curr = leftKid;
          continue;
        }
      }
      return toReturn;
    }

    // If the item has not been found, move down the tree.
    else {

      // If at a leaf, then the item isn't present.
      if (curr->leaf) {
        throw(BTREE_EXCEPTION) REMOVE_KEY_NOT_FOUND;
      }

      // Adjust curr and move down the tree.
      char result = fixChildSize(curr, i);
      if (result == NEW_ROOT) {
        curr = root;
      } else {
        curr = curr->child[findIndex(curr, k)];
      }
    }
  }
}

// Function to find a key in the tree.
// returnValue.first is the node the item is in.
// returnValue.second is the correct index in that node's key array
template <typename T> pair<BNode<T> *, unsigned> BTree<T>::search(T k) {

  // Start at root.
  BNode<T> *x = root;

  // Work down the tree.
  while (true) {

    // Find the proper index in the current node's array.
    unsigned i = findIndex(x, k);

    // Found it!
    if (i < x->size && !(lessThan(k, x->key[i]) || lessThan(x->key[i], k))) {
      return pair<BNode<T> *, unsigned>(x, i);
    }

    // Hit the bottom of the tree.
    else if (x->leaf) {
      return pair<BNode<T> *, unsigned>(NULL, 0);
    }

    // Keep going.
    else {
      x = x->child[i];
    }
  }
}

// Function to find a key in the tree.
// Returns the key.
// If the item was not found an exception is thrown.
template <typename T> T BTree<T>::searchKey(T k) {
  pair<BNode<T> *, unsigned> node = search(k);
  if (node.first == NULL) {
    throw(BTREE_EXCEPTION) SEARCH_KEY_NOT_FOUND;
  }
  return node.first->key[node.second];
}

// Function for printing a tree.
template <typename T> void BTree<T>::print() {
  if (printKey != NULL && root != NULL) {
    printNode(root);
    cout << "\n";
  }
}

// Initialize a b tree node.
// x is a pointer to the node
// t is the minimum degree of the tree.
template <typename T> void BTree<T>::initializeNode(BNode<T> *x) {
  x->size = 0;
  x->key = (T *)malloc((2 * minDegree - 1) * sizeof(T));
  x->child = (BNode<T> **)malloc(2 * minDegree * sizeof(BNode<T> *));
}

// Recursively deletes the subtree rooted at x.
// Does the dirty work for the destructor.
template <typename T> void BTree<T>::freeNode(BNode<T> *x) {
  if (!x->leaf) {
    for (unsigned i = 0; i <= x->size; i++) {
      freeNode(x->child[i]);
    }
  }
  free(x->child);
  free(x->key);
  free(x);
}

// Finds the index of k in x->key.
// If k is not present, returns the index of the subtree
// that could contain k in x->child.
template <typename T> unsigned BTree<T>::findIndex(BNode<T> *x, T k) {
  unsigned i = 0;
  while (i < x->size && lessThan(x->key[i], k)) {
    i++;
  }
  return i;
}

// Inserts k into x.
// Returns the index of k in x->key.
template <typename T> unsigned BTree<T>::nodeInsert(BNode<T> *x, T k) {
  int index;

  // Make room for k.
  for (index = x->size; index > 0 && lessThan(k, x->key[index - 1]); index--) {
    x->key[index] = x->key[index - 1];
    x->child[index + 1] = x->child[index];
  }

  // Insert k.
  x->child[index + 1] = x->child[index];
  x->key[index] = k;
  x->size++;

  return index;
}

// Deletes the indexth element from x->key.
// Returns deleted key.
template <typename T> T BTree<T>::nodeDelete(BNode<T> *x, unsigned index) {

  T toReturn = x->key[index];

  x->size--;
  while (index < x->size) {
    x->key[index] = x->key[index + 1];
    x->child[index + 1] = x->child[index + 2];
    index++;
  }
  return toReturn;
}

// Function for splitting nodes that are too full.
// x points to the parent of the node to splits.
// i is the index in x's child array of the node to split.
template <typename T> void BTree<T>::splitChild(BNode<T> *x, int i) {

  // z is the new node and y is the node to split.
  BNode<T> *toSplit = x->child[i];
  BNode<T> *newNode = (BNode<T> *)malloc(sizeof(BNode<T>));
  ;
  initializeNode(newNode);
  newNode->leaf = toSplit->leaf;
  newNode->size = minDegree - 1;

  // Copy the second half of y's keys and children into z.
  for (unsigned j = 0; j < minDegree - 1; j++) {
    newNode->key[j] = toSplit->key[j + minDegree];
  }
  if (!toSplit->leaf) {
    for (unsigned j = 0; j < minDegree; j++) {
      newNode->child[j] = toSplit->child[j + minDegree];
    }
  }
  toSplit->size = minDegree - 1;

  nodeInsert(x, toSplit->key[minDegree - 1]);
  x->child[i + 1] = newNode;
}

// Merges the (i + 1)th child of parent with the ith child of parent.
// Returns an indicator of whether the change affected the root.
template <typename T>
char BTree<T>::mergeChildren(BNode<T> *parent, unsigned i) {

  BNode<T> *leftKid = parent->child[i];
  BNode<T> *rightKid = parent->child[i + 1];

  // Move item from parent to left child.
  leftKid->key[leftKid->size] = nodeDelete(parent, i);
  unsigned j = ++(leftKid->size);

  // Move everything from rightKid into leftKid
  for (unsigned k = 0; k < rightKid->size; k++) {
    leftKid->key[j + k] = rightKid->key[k];
    leftKid->child[j + k] = rightKid->child[k];
  }
  leftKid->size += rightKid->size;
  leftKid->child[leftKid->size] = rightKid->child[rightKid->size];

  // Free the memory used by rightChild
  free(rightKid->child);
  free(rightKid->key);
  free(rightKid);

  // If parent is empty, than it must have been the root.
  if (parent->size == 0) {
    root = leftKid;
    free(parent->child);
    free(parent->key);
    free(parent);
    return NEW_ROOT;
  }

  return MODIFIED_NOT_ROOT;
}

// Makes sure parent->child[index] has at least minDegree items.
// If it doesn't, then things are changed to make sure it does.
// Returns a code indicating what action was taken.
template <typename T>
char BTree<T>::fixChildSize(BNode<T> *parent, unsigned index) {
  BNode<T> *kid = parent->child[index];

  // If things need fixed.
  if (kid->size < minDegree) {

    // Borrow from left sibling if possible.
    if (index != 0 && parent->child[index - 1]->size >= minDegree) {
      BNode<T> *leftKid = parent->child[index - 1];

      // When there are numerous equivalent keys,
      // nodeInsert can insert into an index other than 0.
      // The for loop fixed child pointers if that happens.
      for (unsigned i = nodeInsert(kid, parent->key[index - 1]); i != 0; i--) {
        kid->child[i] = kid->child[i - 1];
      }
      kid->child[0] = leftKid->child[leftKid->size];
      parent->key[index - 1] = nodeDelete(leftKid, leftKid->size - 1);
    }

    // Borrow from right sibling if possible
    else if (index != parent->size &&
             parent->child[index + 1]->size >= minDegree) {
      BNode<T> *rightKid = parent->child[index + 1];
      // Move curr->key[i] into kid->key
      nodeInsert(kid, parent->key[index]);
      kid->child[kid->size] = rightKid->child[0];
      rightKid->child[0] = rightKid->child[1];
      // Move rightKid->key[0] into curr->key
      parent->key[index] = nodeDelete(rightKid, 0);
    }

    // If borrowing is not possible, then merge.
    else if (index != 0) {
      return mergeChildren(parent, index - 1);
    } else {
      return mergeChildren(parent, index);
    }
    return MODIFIED_NOT_ROOT;
  }

  // If things don't need fixed.
  return NOT_MODIFIED;
}

// Recursive function for printing a tree or subtree.
// node is the root of the subtree to be printed.
template <typename T> void BTree<T>::printNode(BNode<T> *node) {

  unsigned int i;
  for (i = 0; i < node->size; i++) {
    if (!node->leaf) {
      printNode(node->child[i]);
    }
    printKey(node->key[i]);
    cout << "\n";
  }

  // Print all child nodes.
  if (!node->leaf) {
    printNode(node->child[i]);
  }
}

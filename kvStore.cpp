#include "kvStore.h"
#include <bits/stdc++.h>
#include <fstream>
using namespace std;

// function for comparing two string kv pairs
bool compare(pair<string, string> k1, pair<string, string> k2);

// function for printing a string kv pair
void print(pair<string, string> k) { cout << k.first << "\t\t\t" << k.second; }

// create a new file to store the tree
fstream createFile(string filename);

// read an existing database file and store it into a tree
void readFile(string filename, kvStore<string, string> *store);

// delete the given record from the database file
fstream deleteFromFile(string filename, pair<string, string> record);

int main() {

  // csv file for storing entries
  string filename;
  cout << "Enter filename for using as persistent storage (Leave blank to use "
          "default file): ";
  getline(cin, filename);
  if (filename.empty()) {
    cout << "Using default file..." << endl;
    filename = "default.csv";
  }

  // pointer to the data store file
  fstream fileOut = createFile(filename);

  // Create a BTree
  kvStore<string, string> store(3, compare, print);

  // read from file
  cout << "Reading entries from file..." << endl;
  readFile(filename, &store);

  // the MENU for interacting with the store
  int choice;
  string line;
  // Loop for interacting with the store
  while (true) {
    cout << endl << "Press 1 for adding an entry to the store" << endl;
    cout << "Press 2 for getting an entry by its key" << endl;
    cout << "Press 3 for deleting an existing entry" << endl;
    cout << "Press 4 for printing all the entries in the store" << endl;
    cout << "Press 5 for exiting" << endl;
    cout << "Enter your choice: ";
    getline(cin, line);
    stringstream linestream(line);
    linestream >> choice;

    switch (choice) {
      // Get the entry from user and store it
    case 1: {
      string key, value;
      cout << "\nEnter the key: ";
      getline(cin, key);
      cout << "Enter the value: ";
      getline(cin, value);
      cout << "New entry added..." << endl;
      store.set(pair<string, string>(key, value));
      fileOut << key << "," << value << endl;
      break;
    }

      // Search a key in the store. Handles exceptions
    case 2: {
      string key;
      cout << "\nEnter the key for the entry to look for: ";
      getline(cin, key);
      pair<string, string> value;
      value = store.get(key);
      cout << "KEY"
           << "\t\t\t"
           << "VALUE" << endl;
      print(value);
      cout << "\n";
      break;
    }

      // Delete a key from the store. Handles exceptions
    case 3: {
      string key;
      string value;
      bool deleted;
      cout << "\nEnter the key for the entry to delete: ";
      getline(cin, key);
      cout << "Enter the value for the entry to delete: ";
      getline(cin, value);
      deleted = store.del(pair<string, string>(key, value));
      if (!deleted) {
        cout << "Entry not found!" << endl;
      } else {
        cout << "Entry deleted..." << endl;
      }
      fileOut.close();
      fileOut = deleteFromFile(filename, pair<string, string>(key, value));
      break;
    }

      // Print all the data present in the store
    case 4: {
      cout << "\nKEY"
           << "\t\t\t"
           << "VALUE" << endl;
      store.print();
      break;
    }

      // Exist peacefully
    case 5: {
      cout << "Exiting..." << endl;
      fileOut.close();
      return 0;
    }

      // Prompt if the user enters invalid choice
    default: {
      cout << "Please enter a valid choice" << endl;
    }
    }
  }

  fileOut.close();
  return 0;
}

bool compare(pair<string, string> k1, pair<string, string> k2) {
  if (k1.first < k2.first)
    return true;
  return false;
}

fstream createFile(string filename) {
  // file pointer
  fstream fout;

  // opens an existing csv file or creates a new file.
  fout.open(filename, ios::out | ios::app);
  return fout;
}

void readFile(string filename, kvStore<string, string> *store) {

  // File pointer
  fstream fin;

  // Open an existing file
  fin.open(filename, ios::in);
  if (!fin) {
    cout << "Error while reading from file" << endl;
    exit(1);
  }

  // Read the Data from the file
  // as String Vector
  string line, key, value, temp;

  while (fin << temp) {

    // read an entire row and
    // store it in a string variable 'line'
    getline(fin, line);

    // used for breaking words
    stringstream s(line);

    // if line is empty break out
    if (line == "") {
      break;
    }

    // read key and value seperated with a ',' from the given stringstream
    getline(s, key, ',');
    getline(s, value, ',');

    // store in the tree
    store->set(pair<string, string>(key, value));
  }
  fin.close();
}

fstream deleteFromFile(string filename, pair<string, string> record) {

  // close file for old pointer

  // Open File pointers
  fstream fin, fout;

  // Open the existing file
  fin.open(filename, ios::in);

  // Create a new file to store the non-deleted data
  fout.open("temp.csv", ios::out);

  int i, count = 0;
  string line, word;
  vector<string> row;

  // Check if this record exists
  // If exists, leave it and
  // add all other data to the new file
  while (!fin.eof()) {

    row.clear();
    getline(fin, line);
    stringstream s(line);

    while (getline(s, word, ',')) {
      row.push_back(word);
    }

    int row_size = row.size();

    // writing all records,
    // except the record to be deleted,
    // into the new file 'temp.csv'
    // using fout pointer
    if (row[0] != record.first || row[1] != record.second) {
      if (!fin.eof()) {
        for (i = 0; i < row_size - 1; i++) {
          fout << row[i] << ",";
        }
        fout << row[row_size - 1] << endl;
      }
    } else {
      count = 1;
    }
    if (fin.eof())
      break;
  }
  if (count == 1)
    cout << "Record deleted" << endl;
  else
    cout << "Record not found" << endl;

  // Close the pointers
  fin.close();
  fout.close();

  // removing the existing file
  if (remove((filename).c_str()) != 0) {
    cout << "Error while deleting" << endl;
  }

  // renaming the new file with the existing file name
  if (rename("temp.csv", filename.c_str()) != 0) {
    cout << "Error while deleting" << endl;
  }

  // reopen the file and return a pointer to it
  fout.open(filename, ios::out | ios::app);
  return fout;
}

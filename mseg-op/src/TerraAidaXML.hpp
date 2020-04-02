#ifndef TerraAidaXML_hpp
#define TerraAidaXML_hpp

#include "OperatorsBaseDefines.h"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <set>

#define NIL ""

using namespace std;

typedef enum{TAG, ATTRIBUTE, TEXT}TAXMLNodeType;
typedef struct
{
  TAXMLNodeType  type; // type of the node (tag, attribute, text, ...)
  unsigned  level, // level in the tree
      position; // if the parentS has more than one son, this variable contains de number of the son
  string  name, // name of the tag, attribute, text, ...
    value,
    parent; // if the name is attribute, this variable has the value
}TAXMLNode;

class TerraAidaXML
{
  public:
    TerraAidaXML();
    ~TerraAidaXML();

// Load disk xml to memory struct
    bool LoadFromDisk();
// Save disk xml from memory struct
    bool SaveToDisk();
// Set input and output file names
    void SetInputFile(string inputfile);
    void SetOutputFile(string outputfile);

    /**
      * @brief Insert a new node.
      * @param t Node type.
      * @param l level.
      * @param p Position.
      * @param n Name.
      * @param v Value.
      * @param pt Parent.
      * @return true if OK, false on errors.
      */
    bool InsertNode(TAXMLNodeType t, unsigned l, unsigned p, string n, 
      string v, string pt);
    
// Some searchs
    set<string> FindChildren(string parent);
    vector<string> FindNode(string parent, string name);
    vector<TAXMLNode>::iterator GetNode(string name);

// Print input xml file on screen
    void Print();
// Print memory struct on screen    
    void ListTree();
  private:
    bool ExtractValues();
    bool ModifyLastNode(string v);

    string  _inputfile,
      _outputfile,
      _inputline;
    unsigned  _pointer;

    vector<TAXMLNode> _tree;
};

#endif

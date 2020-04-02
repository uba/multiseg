#include "TerraAidaXML.hpp"

TerraAidaXML::TerraAidaXML()
{
  _inputfile = NIL;
  _outputfile = NIL;
  _inputline = NIL;
  _pointer = 0;
}

TerraAidaXML::~TerraAidaXML()
{
}

bool TerraAidaXML::LoadFromDisk()
{
  ifstream xml(_inputfile.c_str());

  if (xml.is_open())
  {
    string  tmp_inputline,
      tmp_inputline_ant;
    while (!xml.eof())
    {
      xml >> tmp_inputline;
      if (tmp_inputline_ant != tmp_inputline)
      {
        tmp_inputline_ant = tmp_inputline;
        _inputline += tmp_inputline + " ";
      }
    }
    xml.close();
    return ExtractValues();
  }
  else
    return false;
}

bool TerraAidaXML::SaveToDisk()
{
  ofstream xml(_outputfile.c_str());

  if (xml.is_open())
  {
    vector<TAXMLNode>::iterator it;
    stack<string> parents;
    string last_tag;
    int dlevel = 0;
    bool  is_attribute = false,
      is_tag = false;
  
  
    for(it = _tree.begin(); it != _tree.end(); ++it)
    {
      if (it->type == TAG)
      {
        if (is_tag)
        {
          xml << ">" << endl;
          parents.push(last_tag);
        }
        if (is_attribute)
        {
          xml << "/>" << endl;
          is_attribute = false;
        }
  
        if (((int) it->level - dlevel) < 0)
        {
          for (unsigned i = 0; i < 3*it->level; i++)
            xml << " ";
          xml << "</" << parents.top() << ">" << endl;
          parents.pop();
        }
  
        for (unsigned i = 0; i < 3*it->level; i++)
          xml << " ";
  
        xml << "<" << it->name;
        last_tag = it->name;
        is_tag = true;
      }
  
      if (it->type == ATTRIBUTE)
      {
        xml << " " << it->name << "=\"" << it->value << "\"";
        is_attribute = true;
        is_tag = false;
      }
  
      dlevel = it->level;
    }
    if (is_attribute)
    {
      xml << "/>" << endl;
      is_attribute = false;
    }
  
    int size = (int)parents.size();
    for (int i = 0; i < size; i++)
    {
      xml << endl << "</" << parents.top() << ">" << endl;
      parents.pop();
    }

    return true;
  }
  else
    return false;
}

void TerraAidaXML::SetInputFile(string inputfile)
{
  _inputfile = inputfile;
}

void TerraAidaXML::SetOutputFile(string outputfile)
{
  _outputfile = outputfile;
}

bool TerraAidaXML::ExtractValues()
{
  _pointer = 0;
  vector<char> ends;
  stack<string> parents;
  string  name = NIL,
    parent = NIL;
  bool  is_attribute = false,
    is_tag = false,
    is_value = false;
  int  level = -1;

  while (_pointer < _inputline.length())
  {
    switch(_inputline[_pointer])
    {
      case '<':
        is_tag = true;
        name = NIL;
        break;
      case ' ':
        if (is_tag)
        {
          level++;
          parent = (parents.size()==0?NIL:parents.top());
          InsertNode(TAG, level, 0, name, NIL, parent);
          parents.push(name);
          name = NIL;
          is_tag = false;
          is_attribute = true;
        }
        break;
      case '=':
        if (is_attribute)
        {
          parent = (parents.size()==0?NIL:parents.top());
          InsertNode(ATTRIBUTE, level, 0, name, NIL, parent);
          name = NIL;
          is_attribute = false;
        }
        break;
      case '"':
        if (is_value)
        {
          ModifyLastNode(name);
          name = NIL;
          is_value = false;
          is_attribute = true;
        }
        else
          is_value = true;
        break;

      case '/':
        if (!is_value)
        {
          level--;
          parents.pop();
          is_tag = false;
          is_attribute = false;
          name = NIL;
        }
        else
          name += _inputline[_pointer];
        break;

      case '>':
        if (is_tag)
        {
          level++;
          parent = (parents.size()==0?NIL:parents.top());
          InsertNode(TAG, level, 0, name, NIL, parent);
          parents.push(name);
          name = NIL;
          is_tag = false;
        }
        break;

      default:
        name += _inputline[_pointer];
        break;
      
    }
    _pointer++;
  }

  if (level != -1)
    return false;
  else
    return true;
}

bool TerraAidaXML::InsertNode(TAXMLNodeType t, unsigned l, unsigned p, string n, string v, string pt)
{
  TAXMLNode  node;
  vector<TAXMLNode>::reverse_iterator it;

  node.type = t;
  node.level = l;
  for(it = _tree.rbegin(); it != _tree.rend(); ++it)
    if (it->type == TAG)
    {
      if (it->name == n && it->parent == pt)
        p = it->position + 1;
      else
        if (it->name == pt)
          p = it->position;
      break;
    }
  node.position = p;
  node.name = n;
  node.value = v;
  node.parent = pt;
  _tree.push_back(node);

  return  true;
}

bool TerraAidaXML::ModifyLastNode(string v)
{
  vector<TAXMLNode>::reverse_iterator it;
  for(it = _tree.rbegin(); it != _tree.rend(); ++it)
    if (it->type == ATTRIBUTE)
    {
      it->value = v;
      break;
    }

  return true;
}

vector<string> TerraAidaXML::FindNode(string parent, string name)
{
  vector<TAXMLNode>::iterator it = _tree.begin();
  vector<string> values;

  for (it = _tree.begin(); it != _tree.end(); ++it)
  {
    if (it->parent == parent && it->name == name)
      values.push_back(it->value);
  }

  return values;
}

set<string> TerraAidaXML::FindChildren(string parent)
{
  vector<TAXMLNode>::iterator it = _tree.begin();
  set<string> children;

  for (it = _tree.begin(); it != _tree.end(); ++it)
  {
    if (it->parent == parent)
      children.insert(it->name);
  }

  return children;
}

vector<TAXMLNode>::iterator TerraAidaXML::GetNode(string name)
{
  vector<TAXMLNode>::iterator it = _tree.begin();

  for (it = _tree.begin(); it != _tree.end(); ++it)
    if (it->name == name)
      return it;

  return _tree.end();
}

void TerraAidaXML::ListTree()
{
  vector<TAXMLNode>::iterator it;
  for(it = _tree.begin(); it != _tree.end(); ++it)
    cout << it->type << "|" << it->level << "|" << it->position << "|" << it->name << "|" << it->value << "|" << it->parent << endl;
}

void TerraAidaXML::Print()
{
  cout << "*" << _inputline << "*" << endl;

}

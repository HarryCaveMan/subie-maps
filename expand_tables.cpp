#include <iostream>     /* cout */
#include <fstream>      /* ifstream */
#include "rapidxml-1.13/rapidxml.hpp" /* namespace rapidxml */
#include "rapidxml-1.13/rapidxml_utils.hpp" /* rapidxml::file */

using namespace std;
using namespace rapidxml;

const string HEX_BASE_MAP("0123456789ABCDEF",16*sizeof(char));

const static void print_formatted_hex_string(const char& byte) {
  unsigned int big_end;
  unsigned int little_end;
  string hex_string("0x",3*sizeof(char));
  big_end = (byte&0xf0)>>4;
  hex_string += HEX_BASE_MAP.at(big_end);
  little_end = byte&0x0f;
  hex_string += HEX_BASE_MAP.at(little_end);
  cout << hex_string << " ";
}

static long get_file_size(ifstream& fs) {
  // set cursor to tail of file for tellg
  fs.seekg (0, fs.end);
  //calculate buffer length
  long length = fs.tellg();
  //reset cursor to head of file
  fs.seekg (0, fs.beg);
  return length;
}

const static char* get_bytes_vector(ifstream& fs,long& length) {  
  //allocate buffer on heap (not good for large files)
  //for large files, use stream_bytes
  char* buff = new char [length];
  //populate buffer and release file handle
  fs.read(buff,length);
  return buff;  
}

const static void stream_bytes(ifstream& fs,long& length, const void (*callback)(const char& byte)) {
  for (long i=0 ; i<length ; i++) {    
    const char byte = fs.seekg(i,fs.beg).get();
    //worked like a charm first try
    callback(byte);
  }
}

int main(int argc, char **argv) {
  const static string map_base_dir(argv[1]);
  const static string hex_path = map_base_dir+"/"+map_base_dir+".hex";
  const static string xml_path = map_base_dir+"/"+map_base_dir+".xml";
  //can't use const on the ifstream because no constructor supports
  //setting exceptions attribute on initilaization, so I must mutate
  //rapidxml also requires lots of mutation up front
  //might make config functions for my classes later
  static ifstream hex_fs(hex_path,ifstream::binary);
  static file<> xml_file(xml_path.c_str());
  static xml_document<> dom;
  dom.parse<0>(xml_file.data());  
  static xml_node<> * ecu_definition(dom.first_node("rom"));
  const static xml_node<> * definition_metadata(ecu_definition->first_node("romid"));
  for (xml_node<> * metadata_node = definition_metadata->first_node(); metadata_node; metadata_node = metadata_node->next_sibling())
	{
    cout << metadata_node->name() << ": " << metadata_node->value() << endl;
  }
  //mutating needlessly makes me sad
  hex_fs.exceptions(ifstream::failbit | ifstream::badbit);
  //const static char* bytes;
  static long length;
  try {
    length = 100; //get_file_size(hex_fs);
    stream_bytes(hex_fs,length,print_formatted_hex_string);
  }
  // catch fs error and print message but then bubble up anyway
  catch (ifstream::failure& fs_ex) {
    cout << "File I/O error checking length!";
    throw fs_ex;
  }
  
}
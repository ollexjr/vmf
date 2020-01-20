#pragma once

#include "vmf.h"
#include <vector>

struct version_info {

};

struct world_info {

};

class VMFFile {
private:
	friend class VMFFileLineReader;
	std::string _filename;
	std::vector<VMFObject *> _object_map;
	std::vector<VMFObject *> _solid_map; // Solid map of non entity world brushes
	std::vector<VMFObject *> _entity_map;
	//version_info vinfo;
public:
	std::string get_filename();
	std::vector<VMFObject *> * get_entities();
	std::vector<VMFObject *> * get_brush_list();
	VMFObject * get_class(std::string key);
	VMFObject * add_object(std::string name);
	VMFFile * add_object_brush(VMFObject * v);
	VMFFile * add_object_entity(VMFObject * v);
	VMFObject * get_world();
	VMFObject * get_version_info();
	//VMFObject * get_version_infov();
	
	void write(std::ofstream & f);
	void dump();
};


#pragma once
#include "vmf_exporter.h"
#include "vmf.h"
#include <string>
#include <vector>

std::string VMFFile::get_filename()
{
	return this->_filename;
}

std::vector<VMFObject *> * VMFFile::get_entities(){
	/*for (int i = 0; i < this->_object_map.size(); i++) {
		if (_object_map[i]->get_name() == "entity") {

		}
	}*/

	return &this->_entity_map;
}

std::vector<VMFObject*> * VMFFile::get_brush_list()
{
	return &this->_solid_map;
	//return this->get_world()->get_object("solid");
	// ->get_brushes("");
}

VMFObject * VMFFile::get_class(std::string name){
	for (int i = 0; i < this->_object_map.size(); i++) {
		if (this->_object_map[i]->get_name() == name) {
			return this->_object_map[i];
		}
	}
	return nullptr;
}

VMFObject * VMFFile::add_object(std::string name) {
	VMFObject * f = new VMFObject(name, 0);
	//if (name == "entity") {
	//	this->_entity_map.push_back(f);
	//}
	
	this->_object_map.push_back(f);
	return f;
}
VMFFile * VMFFile::add_object_brush(VMFObject * v)
{
	this->_solid_map.push_back(v);
	return this;
}
VMFFile * VMFFile::add_object_entity(VMFObject * v)
{
	this->_entity_map.push_back(v);
	return this;
}
VMFObject * VMFFile::get_world()
{
	return this->get_class("world");
}
VMFObject * VMFFile::get_version_info()
{
	return this->get_class("versioninfo");
}
/*VMFObject * VMFFile::get_version_infov()
{
	return this->_object_map[0];
}*/
void VMFFile::write(std::ofstream & f) {
	for (size_t i = 0; i < _object_map.size(); i++) {
		this->_object_map[i]->write(f, 0);
	}
}
void VMFFile::dump() {
	printf("vmf_FILE : %d",
		this->_object_map.size());
	for (size_t i = 0; i < this->_object_map.size(); i++) {
		this->_object_map[i]->dump_table(0);
	}
}


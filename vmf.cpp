#pragma once

#include "vmf.h"
#include <fstream>
#include <iostream>
#include <algorithm>


/*enum object_id {
	OBJECT_NULL = 0,
	VINT32 = 1,
	VFLOAT = 2,
	STRING = 3,
	VEC2 = 4,
	VEC3 = 5,
	AXIS = 6,
	PLANE = 7,
	OBJECT = 10,
};
struct object_map {
	object_id id;
	std::string name;
	void * data;
};*/

void fill_indentf(std::ofstream & f, int indent) {
	for (size_t i = 1; i < indent; i++) {
		f << "\t";
	}
}
void fill_indent(int indent) {
	for (size_t i = 1; i < indent; i++) {
		putchar('-');
		putchar('-');
	}
}

bool VMFObject::get_wobject(object_id id, std::string name, void ** optr) {
	for (int i = 0; i < this->_map.size(); i++) {
		object_map * v = &this->_map[i];
		if ((v->name == name) && (v->id == id)) {
			*optr = v->data;
			return true;
		}
	}
	//printf("[VMFObject] key does not exist\n");
	return false;
}

bool VMFObject::_try_cache(std::string & key) {
	if (this->_cache_key == key) {
		return true;
	}
	return false;
}

int VMFObject::get_depth()
{
	return this->_depth;
}

bool VMFObject::delete_object(const std::string & key)
{
	for (int i = 0; i < this->_object_map.size(); i++) {
		VMFObject * v = this->_object_map[i];
		if (v->get_name() == key) {
			this->_object_map.erase(this->_object_map.begin() + i);
			return true;
		}
	}

	return false;
}

VMFObject * VMFObject::add_object(const std::string & name) {
	VMFObject * f = new VMFObject(name, this->_depth + 1);

	if (name.size() == 5 &&
		(name[0] == 's')
		&& (name[1] == 'o')
		&& (name == "solid")) {
		//this->_solid_map.push_back(f);
	}

	this->_object_map.push_back(f);
	return f;
}
VMFObject * VMFObject::add_plane(const std::string & key, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {

	glm::vec3 * p = new glm::vec3[3];
	p[0] = v1;
	p[1] = v2;
	p[2] = v3;

	//if (v1.x > this->_mx) {
	//	std::min(_mx, v.x);
	//}
	struct object_map om;
	om.id = object_id::PLANE;
	om.name = key;
	om.data = (void*)p;

	this->_map.push_back(om);
	return this;
}
VMFObject * VMFObject::add_plane_s(const std::string & key, plane f) {

	this->add_plane(key, f[0], f[1], f[2]);
	return this;
}
VMFObject * VMFObject::add_axis_s(std::string key, axis & f) {
	char buf[100];


	axis * a = new axis;
	*a = f;

	struct object_map om;

	om.id = object_id::AXIS;
	om.name = key;
	om.data = (void*)a;

	this->_map.push_back(om);
	return this;
	//sprintf_s(buf, 100, "[%d %d %d %d] %f", f.x, f.y, f.z, f.a, f._f);
	//return this->add_string(key, buf);
}
VMFObject * VMFObject::add_axis(std::string key, int x, int y, int z, int dec, float dec2) {
	char buf[100];
	sprintf_s(buf, 100, "[%d %d %d %d] %f", x, y, dec, dec2);


	axis a;
	a.x = x;
	a.y = y;
	a.z = z;
	a.a = dec;
	a._f = dec2;
	add_axis_s(key, a);
	return this;
}
VMFObject * VMFObject::add_int(std::string key, int val) {
	struct object_map om;
	om.id = object_id::VINT32;
	om.name = key;
	om.data = new int;
	*(int*)(om.data) = val;// = 1;

	this->_map.push_back(om);
	return this;
}
std::string VMFObject::get_name()
{
	return this->_name;
}
axis * VMFObject::get_axis(const std::string & key) {
	for (int i = 0; i < this->_map.size(); i++) {
		if ((this->_map[i].id == AXIS) && (this->_map[i].name == key)) {
			return (axis*)this->_map[i].data;
		}
	}
	return nullptr;
}

int VMFObject::get_int(std::string key)
{
	int * int32 = nullptr;
	if (!this->get_wobject(object_id::VINT32, key, (void**)&int32)) {
		assert(false);
	}
	return (*int32);
}
std::string * VMFObject::get_string(std::string key) {
	_try_cache(key);
	std::string * str = nullptr;
	if (!this->get_wobject(object_id::STRING, key, (void**)&str)) {
		//assert(false);
		return nullptr;
	}
	return (str);
}
/*std::string VMFObject::get_string(std::string key)
{
	return *get_string(key);
}*/
VMFObject * VMFObject::get_object(std::string key)
{
	for (int i = 0; i < this->_object_map.size(); i++) {
		VMFObject * v = this->_object_map[i];
		if (v->get_name() == key) {
			return v;
		}
	}
	return nullptr;
}
bool VMFObject::get_objects(std::string key, std::vector<VMFObject*> * out) {
	int found = 0;
	for (int i = 0; i < this->_object_map.size(); i++) {
		VMFObject * v = this->_object_map[i];
		if (v->get_name() == key) {
			out->push_back(v);
			found++;
		}
	}
	return found > 0;
}
plane * VMFObject::get_plane()
{
	for (int i = 0; i < this->_map.size(); i++) {
		if (this->_map[i].id == PLANE) {
			return (plane*)this->_map[i].data;
		}
	}
	return nullptr;
}
/*std::vector<VMFObject*>* VMFObject::get_brushes()
{
	return &this->_solid_map;
}*/
bool VMFObject::has_child_classes()
{
	return this->_object_map.size() > 0;
}
bool VMFObject::has_keys()
{
	return this->_object_map.size() > 0;
}
void VMFObject::dump_table(int indent) {
	//fill_indent(indent);
	indent++;
	printf("vmf_object : %s | #Variables: %d | #Objects: %d\n",
		this->_name.c_str(),
		this->_map.size(),
		this->_object_map.size());
	for (size_t i = 0; i < this->_map.size(); i++) {
		struct object_map * m = &this->_map[i];
		fill_indent(indent);
		switch (this->_map[i].id) {
		case object_id::VINT32:
			printf("(int32) %s : %d\n", m->name.c_str(), *(int*)m->data);
			break;
		case object_id::STRING:
			printf("(string) %s : %s\n", m->name.c_str(), (*(std::string*)m->data).c_str());
			break;
		case object_id::PLANE:
			/* effort to write this tbh*/
			break;
		}
	}
	for (size_t i = 0; i < this->_object_map.size(); i++) {
		this->_object_map[i]->dump_table(indent);
	}
}
VMFObject * VMFObject::add_string(std::string key, std::string value) {
	struct object_map om;
	om.name = key;
	om.id = object_id::STRING;
	om.data = new std::string;
	*(std::string*)(om.data) = value;// = 1;

	this->_map.push_back(om);
	return this;
}
std::string * VMFObject::get_string_ptr(const std::string & key) {
	//_try_cache(key);
	std::string * str = nullptr;
	if (!this->get_wobject(object_id::STRING, key, (void**)&str)) {
		//assert(false);
		return nullptr;
	}
	return str;
}
#define NEWLINE "\n"
#define SPACE " "
int VMFObject::write(std::ofstream & f, int indent) {
	indent++;
	fill_indentf(f, indent);
	f << this->_name << NEWLINE;

	fill_indentf(f, indent);
	f << "{" NEWLINE;
	//#define ENQUOTE(x) ("\""  << x << "\"\n")

	indent++;
	for (size_t i = 0; i < this->_map.size(); i++) {
		struct object_map * m = &this->_map[i];
		switch (this->_map[i].id) {
		case object_id::VINT32:
			//printf("(int32) %s : %d\n", m->name.c_str(), *(int*)m->data);
			fill_indentf(f, indent);
			f << "\"" << m->name << "\" \"" << *(int*)m->data << "\"" << NEWLINE;
			break;
		case object_id::STRING:
			//printf("(string) %s : %s\n", m->name.c_str(), (*(std::string*)m->data).c_str());
			fill_indentf(f, indent);
			f << "\"" << m->name << "\" \"" << (*(std::string*)m->data) << "\"" << NEWLINE;
			break;
		case object_id::AXIS: {
			axis * ax = ((axis*)m->data);
			fill_indentf(f, indent);
			f << "\"" << m->name << "\" \"[";
			f << ax->x << " ";
			f << ax->y << " ";
			f << ax->z << " ";
			f << ax->a << "] ";
			f << ax->_f << "\"" << NEWLINE;
			// << (*(std::string*)m->data) << 
		}break;
		case object_id::PLANE:
			/* effort to write this tbh*/
			glm::vec3 * ary = ((glm::vec3*)m->data);
			ary[0].x;
			ary[0].y;
			ary[0].z;

			ary[1].x;
			ary[1].y;
			ary[1].z;


			ary[2].x;
			ary[2].y;
			ary[3].z;

			fill_indentf(f, indent);
			f << "\"" << m->name << "\" \"";
			f << "(" << ary[0].x << SPACE << ary[0].y << SPACE << ary[0].z << ")" SPACE;
			f << "(" << ary[1].x << SPACE << ary[1].y << SPACE << ary[1].z << ")" SPACE;
			f << "(" << ary[2].x << SPACE << ary[2].y << SPACE << ary[2].z << ")\"" NEWLINE;
			break;
		}
	}
	indent--;
	for (size_t i = 0; i < this->_object_map.size(); i++) {
		fill_indentf(f, indent);
		f << NEWLINE;
		this->_object_map[i]->write(f, indent);
	}
	fill_indentf(f, indent);
	f << "}" NEWLINE;
	return 0;
}
VMFObject::VMFObject(std::string name, int depth) {
	this->_name = name;
	this->_depth = depth;
}
VMFObject::~VMFObject() {
	for (size_t i = 0; i < this->_map.size(); i++) {
		struct object_map * m = &this->_map[i];
		switch (m->id) {
		case object_id::VINT32:
			m->name;
			delete (int*)m->data;
			break;
		case object_id::STRING:
			//(*(std::string*)m->data);
			delete (std::string*)m->data;
			break;
		case object_id::PLANE:
			//glm::vec3 * ary = ((glm::vec3*)m->data);
			delete[](glm::vec3*)m->data;
		default:
			printf("memory leak in dealloc, debug this\n");
			break;
		}
		//delete this->_map[i].data;
	}
}

VMFPrefab::VMFPrefab()
{
}

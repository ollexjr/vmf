#pragma once
#include "vmf.h"
#include "vmf_import.h"
#include "vmf_exporter.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <chrono>

#include "util.h"

void VMFFileLineReader::strip_escape_codes(std::string & buf)
{
	for (int i = 0; i < buf.size(); i++) {
		if (
			buf[i] == '\n' ||
			buf[i] == '\r' ||
			buf[i] == '\t') {
			switch (buf[i]) {
			case 10://'\n':
				//printf("n");
				break;
			case '\r':
				//printf("r");
				break;
			case '\t':
				//printf("t");
				break;
			}
			buf.erase(buf.begin() + i);
			i--;
		}
	}
}

int VMFFileLineReader::_findClass(int start, svclass * f) {
	bool fClassName = false;
	int openBrace = 0;
	int openIndex = 0;

	int closeBrace = 0;
	int closeIndex = 0;


	const int sz = this->_lineBuf.size();
	for (int i = start; i < sz; i++) {
		std::string * v = &this->_lineBuf[i];
		if (!fClassName) {
			if (this->_isClassName(i)) {
				f->name = (*v);
				fClassName = true;
				//Iprintf(this->_depth, "[_findClass: %s]\n", name.c_str());
			}
			continue;
		}
		if (fClassName) {
			if (v->length() == 1) {
				if ((*v)[0] == '{') {
					if (openBrace == 0) {
						openIndex = i;
					}
					openBrace++;
				}
				else if ((*v)[0] == '}') {
					closeBrace++;
					closeIndex = i;
				}
			}
			if (openBrace > 0 && (openBrace == closeBrace)) {
				/* found end of class!*/
				f->start = openIndex + 1;
				f->end = closeIndex - 1;
				//f->name = name;
				return 1;
			}
		}
	}
	if (fClassName || openBrace > 0 | closeBrace > 0) {
		/* failed to find ending brace*/
		printf("Failed to find class: EOF\n");
		return -1;
	}
	return 0;
}

int strip_non_numeric(std::string * s) {
	for (int i = 0; i < s->size(); i++) {
		if ((*s)[i] < '0' || (*s)[i] > '9' && ((*s)[i] != '-' || (*s)[i] != '.' || (*s)[i] != ' ')) {
			(*s)[i] = ' ';
			s->erase(s->begin() + i);
			i--;
		}
	}
	return 0;
}

// Scan string to assess if it's a plane, contains a number of optimisations to quickly skip obviously incorrect input.
bool VMFFileLineReader::_parse_plane(const std::string & src, plane * f) {

	int cidx = 0;
	int ridx = 0;
	int ceat = 0;
	for (int i = 0; i < src.size(); i++) {
		const char c = src[i];
		if ((c >= '0' && c <= '9') || (c == '.') || (c == '-')) {
			if (ceat++ == 0) {
				cidx = i;
			}
		}
		if ((c == ' ' || c == '(' || c == ')' || (i == (src.size() - 1))) && ceat > 0) {
			if (ceat > 0) {
				std::string sub = src.substr(cidx, ceat);
				switch (ridx) {
				case 0:
					(*f)[0].x = atoi(sub.c_str());
					break;
				case 1:
					(*f)[0].y = atoi(sub.c_str());
					break;
				case 2:
					(*f)[0].z = atoi(sub.c_str());
					break;

					//plane 2
				case 3:
					(*f)[1].x = atoi(sub.c_str());
					break;
				case 4:
					(*f)[1].y = atoi(sub.c_str());
					break;
				case 5:
					(*f)[1].z = atoi(sub.c_str());
					break;

					//plane 3
				case 6:
					(*f)[2].x = atoi(sub.c_str());
					break;
				case 7:
					(*f)[2].y = atoi(sub.c_str());
					break;
				case 8:
					(*f)[2].z = atoi(sub.c_str());
					break;
				}
			}
			ceat = 0;
			cidx = 0;
			ridx++;
		}
	}
	if (ridx != 9) {
		printf("warning plane! invalid amount of values\n");
		return false
	}
	return true;
}

bool VMFFileLineReader::_parse_axis(const std::string & src, axis * a){
	int cidx = 0;
	int ridx = 0;
	int ceat = 0;
	for (int i = 0; i < src.size(); i++) {
		const char c = src[i];
		if ((c >= '0' && c <= '9') || (c == '.') || (c == '-')) {
			if (ceat++ == 0) {
				cidx = i;
			}

		}
		if ((c == ' ' || c == '[' || c == ']' || (i == (src.size() - 1))) && ceat > 0) {
			if (ceat > 0) {
				std::string f = src.substr(cidx, ceat);
				switch (ridx) {
				case 0:
					a->x = atoi(f.c_str());
					break;
				case 1:
					a->y = atoi(f.c_str());
					break;
				case 2:
					a->z = atoi(f.c_str());
					break;
				case 3:
					a->a = atoi(f.c_str());
					break;
				case 4:
					a->_f = atof(f.c_str());
					break;
				}
			}
			ceat = 0;
			cidx = 0;
			ridx++;
		}
	}
	return true;
}

bool VMFFileLineReader::_isClassName(int index) {
	std::string * v = &this->_lineBuf[index];
	if (v->size() < 2 || (*v)[0] == '\"') {
		return false;
	}
	for (int i = 0; i < v->size(); i++) {
		if ((*v)[i] < 'a' || (*v)[i] > 'z') {
			//classnames are lowercase and the string musts be between or equal a to z
			return false;
		}
	}
	return true;
}

bool VMFFileLineReader::_isKey(int index) {
	std::string * v = &this->_lineBuf[index];
	if ((v->size() > 0) && (v->size() >= 7) && (*v)[0] == '\"') {
		return true;
	}
	return false;
}

int VMFFileLineReader::_getKey(int index, VMFObject * o) {
	std::string * v = &this->_lineBuf[index];

	//minimum keyvalue pair is 7 characters because the smallest pair is: "a" "b"
	if ((v->size() < 7) || ((*v)[0] != '\"') || ((*v)[v->size() - 1] != '\"')) {
		printf("invalid key/value!: %s\n", v->c_str());
		return -1;
	}

	int openTagKey = -1;
	int closeTagKey = -1;
	int openTagValue = -1;
	int closeTagValue = -1;

	bool notNumeric = false;

	//regex is slow, do a #highperformance scan
	//|| closeTagValue < 0;
	for (int i = 0; i < v->size(); i++) {
		if ((*v)[i] == '\"') {
			//openTag++;
			if (openTagKey < 0)
				openTagKey = i + 1;
			else if (openTagKey > -1 && closeTagKey < 0)
				closeTagKey = i;
			else if (closeTagKey > -1 && openTagValue < 0)
				openTagValue = i + 1;
			else if (openTagValue > -1 && closeTagValue < 0)
				closeTagValue = i;
			continue;
		}
		if (openTagValue > -1
			//&& ((*v)[i] != '\"') 
			&& ((*v)[i] < '0' || (*v)[i] > '9')) {
			//flag that it is not a single integer, (contains non numeric)
			notNumeric = true;
		}
	}
	if (openTagKey > -1
		&& closeTagKey > -1
		&& openTagValue > -1
		&& closeTagValue > -1) {
		std::string key = v->substr(openTagKey, closeTagKey - openTagKey);
		std::string value = v->substr(openTagValue, closeTagValue - openTagValue);

		if (key.size() == 0 || value.size() == 0) {
			printf("invalid key/value!: %s\n", v->c_str());
			return -1;
		}
		if ((key)[0] == 'p' && (key) == "plane") {
			plane f = {};
			this->_parse_plane(value, &f);
			o->add_plane_s(key, f);
		}
		else if ((key)[0] == 'v' && (key) == "vaxis") {
			//o->add_axis(key, value);
			axis a;
			_parse_axis(value, &a);
			o->add_axis_s("vaxis", a);
		}
		else if ((key)[0] == 'u' && (key) == "uaxis") {
			axis a;
			_parse_axis(value, &a);
			o->add_axis_s("uaxis", a);
		}
		else {
			if (!notNumeric) {
				o->add_int(key, atoi(value.c_str()));
			}
			else {
				o->add_string(key, value);
			}
		}
	}
	else {
		printf("invalid key/value!: %s\n", v->c_str());
		return false;
	}
	return -1;
}

int VMFFileLineReader::_parse_class(svclass * c, VMFObject * f) {
	this->_depth++;

	for (int i = c->start; i < c->end; i++) {
		if (_isKey(i)) {
			_getKey(i, f);
		}
		if (_isClassName(i)) {
			svclass cc = {};
			if (this->_findClass(i++, &cc) < 0) {
				/* fatal error, class tag but something illegal
					(presumably to do with mismatched braces) was found
				*/
				_depth--;
				return -1;
			}
			VMFObject * v = f->add_object(cc.name);
			if (this->_parse_class(&cc, v) < 0) {
				_depth--;
				return -1;
			}
			if (cc.name == "solid" && f->get_name() != "entity") {
				this->_vmf->add_object_brush(v);
			}
			i = cc.end;
		}
	}
	_depth--;
	return 0;
}

int VMFFileLineReader::_parse_top(std::vector<std::string> buf, VMFFile * f) {
	/* parse top level of file*/
	std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
	printf("reading file... %d\n", buf.size());
	for (int i = 0; i < buf.size(); i++) {
		int ret = 0;
		svclass c = {};

		switch (ret = _findClass(i, &c)) {
		case 1: {
			if (c.name == "entity") {
				VMFObject * v = f->add_object(c.name);
				_parse_class(&c, v);
				std::string * t = v->get_string("classname");
				f->add_object_entity(v);
				if (t && *t == "func_detail") {
					f->add_object_brush(v);
				}
				
			}
			else {
				VMFObject * v = f->add_object(c.name);
				_parse_class(&c, v);
			}

			//set the index to the end index of the class,
			//because parse_class has eaten it
			i = c.end;
		}break;
		case -1:
			return -2;
		case 0:
			printf("\tEOF\n");
			buf[i];
			break;
		}
	}
	//continue until the file ends! :D
	std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
	std::chrono::milliseconds time_mill = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	printf("\timport time: %dms\n", time_mill.count());

	return 0;
}

VMFFile * VMFFileLineReader::get_file_tree() {
	return this->_vmf;
}

bool VMFFileLineReader::open(std::string name)
{
	this->_filename = name;
	this->_file.open(name);
	if (!this->_file.is_open())
		return false;
	return true;
}

bool VMFFileLineReader::is_open()
{
	return this->_file.is_open();
}

void VMFFileLineReader::close()
{
	this->_file.close();
	this->_lineBuf.clear();
}

bool VMFFileLineReader::parse(VMFFile * f) {
	if (!this->_file.is_open()) {
		assert(false);
		return false;
	}
	f->_filename = this->_filename;
	this->_vmf = f;

	//reserve some space in the buffer
	std::string buf;
	buf.reserve(200);

	//files are usually about 2000 lines, some can be huge
	//it's mainly down to the number of brushes in the solid section
	//so to avoid agressive reallocs in the below loop, size it up a bit to a generous size

	this->_lineBuf.reserve(10000);
	while (std::getline(this->_file, buf)) {
		strip_escape_codes(buf);
		this->_lineBuf.push_back(buf);
	}
	if (this->_lineBuf.size() > 0) {
		this->_parse_top(this->_lineBuf, this->_vmf);
	}
	return true;
}

VMFFileLineReader::~VMFFileLineReader() {
	if (this->_file.is_open())
		this->_file.close();
	if (this->_regexPlane)
		delete this->_regexPlane;
}


void VMFFileLineReaderPrefab::strip_escape_codes(std::string & buf)
{
	for (int i = 0; i < buf.size(); i++) {
		if (
			buf[i] == '\n' ||
			buf[i] == '\r' ||
			buf[i] == '\t') {
			switch (buf[i]) {
			case 10:
				break;
			case '\r':
				break;
			case '\t':
				break;
			}
			buf.erase(buf.begin() + i);
			i--;
		}
	}
}

int VMFFileLineReaderPrefab::_findClass(int start, svclass * f) {
	bool fClassName = false;
	int openBrace = 0;
	int openIndex = 0;

	int closeBrace = 0;
	int closeIndex = 0;

	const int sz = this->_lineBuf.size();
	for (int i = start; i < sz; i++) {
		std::string * v = &this->_lineBuf[i];
		if (!fClassName) {
			if (this->_isClassName(i)) {
				f->name = (*v);
				fClassName = true;
				//Iprintf(this->_depth, "[_findClass: %s]\n", name.c_str());
			}
			continue;
		}
		if (fClassName) {
			if (v->length() == 1) {
				if ((*v)[0] == '{') {
					if (openBrace == 0) {
						openIndex = i;
					}
					openBrace++;
				}
				else if ((*v)[0] == '}') {
					closeBrace++;
					closeIndex = i;
				}
			}
			if (openBrace > 0 && (openBrace == closeBrace)) {
				/* found end of class!*/
				f->start = openIndex + 1;
				f->end = closeIndex - 1;
				//f->name = name;
				return 1;
			}
		}
	}
	if (fClassName || openBrace > 0 | closeBrace > 0) {
		/* failed to find ending brace*/
		printf("Failed to find class: EOF\n");
		return -1;
	}
	return 0;
}

bool VMFFileLineReaderPrefab::_parse_plane(const std::string & src, plane * f) {
	//TODO: 
	assert(false)
	
	/*for (int i = 0; i < src.size(); i++) {
		const char c = src[i];
	}*/
	return false;
}

bool VMFFileLineReaderPrefab::_isClassName(int index) {
	std::string * v = &this->_lineBuf[index];
	if (v->size() < 2 || (*v)[0] == '\"') {
		return false;
	}
	for (int i = 0; i < v->size(); i++) {
		if ((*v)[i] < 'a' || (*v)[i] > 'z') {
			//classnames are lowercase and the string musts be between or equal a to z
			return false;
		}
	}
	return true;
}

bool VMFFileLineReaderPrefab::_isKey(int index) {
	std::string * v = &this->_lineBuf[index];
	if ((v->size() > 0) && (v->size() >= 7) && (*v)[0] == '\"') {
		return true;
	}
	return false;
}

int VMFFileLineReaderPrefab::_getKey(int index, VMFObject * o) {
	std::string * v = &this->_lineBuf[index];

	//minimum keyvalue pair is 7 characters because the smallest pair is: "a" "b"
	if ((v->size() < 7) || ((*v)[0] != '\"') || ((*v)[v->size() - 1] != '\"')) {
		printf("invalid key/value!: %s\n", v->c_str());
		return -1;
	}

	int openTagKey = -1;
	int closeTagKey = -1;
	int openTagValue = -1;
	int closeTagValue = -1;
	bool notNumeric = false;

	//regex is slow, do a #highperformance scan
	for (int i = 0; i < v->size(); i++) {
		if ((*v)[i] == '\"') {
			//openTag++;
			if (openTagKey < 0)
				openTagKey = i + 1;
			else if (openTagKey > -1 && closeTagKey < 0)
				closeTagKey = i;
			else if (closeTagKey > -1 && openTagValue < 0)
				openTagValue = i + 1;
			else if (openTagValue > -1 && closeTagValue < 0)
				closeTagValue = i;
			continue;
		}
		if (openTagValue > -1
			//&& ((*v)[i] != '\"') 
			&& ((*v)[i] < '0' || (*v)[i] > '9')) {
			//flag that it is not a single integer, (contains non numeric)
			notNumeric = true;
		}
	}
	if (openTagKey > -1
		&& closeTagKey > -1
		&& openTagValue > -1
		&& closeTagValue > -1) {
		std::string key = v->substr(openTagKey, closeTagKey - openTagKey);
		std::string value = v->substr(openTagValue, closeTagValue - openTagValue);

		if (key.size() == 0 || value.size() == 0) {
			printf("invalid key/value!: %s\n", v->c_str());
			return -1;
		}
		if ((key)[0] == 'p' && (key) == "plane") {
			plane f = {};
			this->_parse_plane(value, &f);
			o->add_plane_s(key, f);
		}
		else if ((key)[0] == 'v' && (key) == "vaxis") {
			o->add_string(key, value);
		}
		else {
			if (!notNumeric) {
				o->add_int(key, atoi(value.c_str()));
			}
			else {
				o->add_string(key, value);
			}
		}
	}
	else {
		printf("invalid key/value!: %s\n", v->c_str());
		return false;
	}
	return -1;
}

int VMFFileLineReaderPrefab::_parse_class(svclass * c, VPrefab * f) {
	this->_depth++;

	for (int i = c->start; i < c->end; i++) {
		if (_isKey(i)) {
			//_getKey(i, f);
		}
		if (_isClassName(i)) {
			svclass cc = {};
			if (this->_findClass(i++, &cc) < 0) {
				/* fatal error, class tag but something illegal
				(presumably to do with mismatched braces) was found*/
				_depth--;
				return -1;
			}
			//VMFObject * v = f->add_object(cc.name);
			if (this->_parse_class(&cc, f) < 0) {
				_depth--;
				return -1;
			}
			i = cc.end;
		}
	}
	_depth--;
	return 0;
}

int VMFFileLineReaderPrefab::_parse_top(std::vector<std::string> buf, VPrefab * f) {
	/* parse top level of file*/
	std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
	printf("reading file... %d\n", buf.size());
	for (int i = 0; i < buf.size(); i++) {
		int ret = 0;
		svclass c = {};

		switch (ret = _findClass(i, &c)) {
		case 1: {


			if (c.name == "entity") {
				VMFObject * v = new VMFObject("entity", this->_depth + 1);
				//f->add_object(c.name);
				_parse_class(&c, f);
				std::string * t = v->get_string("classname");
				if (t && *t == "portal") {

				}
				if (t && *t == "funcdetail") {
					//add copy to brush set
					//f->
				}
			}
			else if (c.name == "world") {

			}
			else {
				i = c.end;
				_parse_class(&c, f);
				//set the index to the end index of the class,
				//because parse_class has eaten it
			}

		}break;
		case -1:
			return -2;
		case 0:
			printf("\tEOF\n");
			break;
		}
	}
	std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
	std::chrono::milliseconds time_mill = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	printf("\timport time: %dms\n", time_mill.count());
	return 0;
}


bool VMFFileLineReaderPrefab::open(std::string name)
{
	this->_file.open(name);
	if (!this->_file.is_open())
		return false;
	return true;
}

bool VMFFileLineReaderPrefab::is_open()
{
	return this->_file.is_open();
}

void VMFFileLineReaderPrefab::close()
{
	this->_file.close();
	this->_lineBuf.clear();
}

bool VMFFileLineReaderPrefab::parse(VPrefab * f) {
	if (!this->_file.is_open()) {
		assert(false);
		return false;
	}

	this->_regexPlane = new std::regex(("\\(([-0-9]+) ([-0-9]+) ([-0-9]+)\\) \\(([-0-9]+) ([-0-9]+) ([-0-9]+)\\) \\(([-0-9]+) ([-0-9]+) ([-0-9]+)\\)"));

	//reserve some space in the buffer
	std::string buf;
	buf.reserve(200);
	this->_lineBuf.reserve(10000);
	while (std::getline(this->_file, buf)) {
		strip_escape_codes(buf);
		this->_lineBuf.push_back(buf);
	}
	if (this->_lineBuf.size() > 0) {
		this->_parse_top(this->_lineBuf, f);
	}
	return true;
}

VMFFileLineReaderPrefab::~VMFFileLineReaderPrefab() {
	if (this->_file.is_open())
		this->_file.close();
	if (this->_regexPlane)
		delete this->_regexPlane;
}

void print_vec3(glm::vec3 & in)
{
	printf("vec3: %f %f %f\n", in.x, in.y, in.x);
}

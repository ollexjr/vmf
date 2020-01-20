/* Copyright 2019 Orlando Crawford */
#pragma once
#include "vmf.h"
#include "vmf_import.h"
#include "vmf_exporter.h"


#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include <glm\glm.hpp>
#include <regex>

#include "prefab.h"

/* 
	fairly fast recursive parser for the "Valve Map Format" format
	!compile with release and full optimisation!
*/
class VMFFileLineReader {
private:
	int _depth = 0;
	struct svclass {
		std::string name;
		int start, end;
	};
	std::ifstream _file;
	std::string _filename;
	VMFFile * _vmf;
	std::vector<std::string> _lineBuf;

	std::smatch match;
	//std::regex * _regexKeyPair;
	std::regex * _regexPlane;

	void strip_escape_codes(std::string & buf);
	int _findClass(int start, svclass * f);
	bool _parse_plane(const std::string & key, plane * f);
	bool _parse_axis(const std::string & src, axis * a);

	bool _isClassName(int index);
	bool _isKey(int index);
	int _getKey(int index, VMFObject * o);
	int _parse_class(svclass * c, VMFObject * f);
	int _parse_top(std::vector<std::string> buf, VMFFile * f);
public:
	VMFFile * get_file_tree();
	//VMFFile * get_file_tree_ptr();
	bool open(std::string name);
	bool is_open();
	void close();
	bool parse(VMFFile * f);
	//VMFFileLineReader();
	~VMFFileLineReader();
};

class VMFFileLineReaderPrefab {
private:
	int _depth = 0;
	struct svclass {
		std::string name;
		int start, end;
	};
	std::ifstream _file;
	std::vector<std::string> _lineBuf;
	
	std::smatch match;
	std::regex * _regexPlane;

	void strip_escape_codes(std::string & buf);
	int _findClass(int start, svclass * f);
	bool _parse_plane(const std::string & key, plane * f);

	bool _isClassName(int index);
	bool _isKey(int index);
	int _getKey(int index, VMFObject * o);
	int _parse_class(svclass * c, VPrefab * f);
	int _parse_top(std::vector<std::string> buf, VPrefab * f);
public:
	//VMFFile * get_file_tree_ptr();
	bool open(std::string name);
	bool is_open();
	void close();
	bool parse(VPrefab * f);
	//VMFFileLineReader();
	~VMFFileLineReaderPrefab();
};

void print_vec3(glm::vec3 & in);
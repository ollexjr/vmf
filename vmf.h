#pragma once
#include <string>
#include <vector>
#include <map>

#include <glm\glm.hpp>
enum object_id {
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
};

typedef glm::vec3 plane[3];

struct axis {
	int x; //ignore the names, idk what they should be
	int y;
	int z;
	int a;

	//glm::vec4 _axis;
	float _f;
};

class VMFObject {
private:
	std::string _name;
	//std::string _class;
	std::vector<object_map> _map;
	std::vector<VMFObject *> _object_map;
	//std::vector<VMFObject *> _solid_map;
	int _depth = 0;

	//float min_x;
	//float max_x;
	std::string _cache_key;

	bool get_wobject(object_id id, std::string name, void ** out);
	bool _try_cache(std::string & key);
public:
	int get_depth();
	bool   delete_object(const std::string & name);
	VMFObject * add_object(const std::string & name);
	VMFObject * add_plane(const std::string & key, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3);
	VMFObject * add_plane_s(const std::string & key, plane f);

	VMFObject * add_axis_s(std::string key, axis & f);
	VMFObject * add_axis(std::string key, int x, int y, int z, int dec, float dec2);
	VMFObject * add_int(std::string key, int val);
	VMFObject * add_string(std::string key, std::string value);
	std::string * get_string_ptr(const std::string & key);
	std::string get_name();

	axis * get_axis(const std::string & key);

	int get_int(std::string key);

	//std::string get_string(std::string key);
	std::string * get_string(std::string key);
	VMFObject * get_object(std::string key);
	bool get_objects(std::string key, std::vector<VMFObject*> * out);
	plane * get_plane();
	//std::vector<VMFObject *> * get_brushes();

	bool has_child_classes();
	bool has_keys();

	void dump_table(int indent);

#define NEWLINE "\n"
	int write(std::ofstream & f, int indent);
	VMFObject(std::string name, int depth);
	~VMFObject();
};


class VMFObjectM {
private:
	std::string _name;
	std::string _class;

	std::map<std::string, int> _map_int32;
	std::map<std::string, int> _map_string;
	std::map<std::string, int> _map_plane;
	std::map<std::string, int> _map_axis;
	std::vector<VMFObjectM *> _object_map;

	int _depth = 0;

	bool get_wobject(object_id id, std::string name, void * out);
public:
	int get_depth();
	VMFObjectM * add_object(std::string name);
	VMFObjectM * add_plane(std::string key, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3);
	VMFObjectM * add_axis(std::string key, int x, int y, int dec, float dec2);
	VMFObjectM * add_int(std::string key, int val);
	VMFObjectM * add_string(std::string key, std::string value);

	std::string get_name();
	int get_int(std::string key);
	std::string get_string(std::string key);
	VMFObjectM * get_object(std::string key);

	bool has_child_classes();
	bool has_keys();

	void dump_table(int indent);

#define NEWLINE "\n"
	int write(std::ofstream & f, int indent);
	VMFObjectM(std::string name, int depth);
	~VMFObjectM();

};
class VMFPrefab {
	std::string name;


	VMFPrefab();
};
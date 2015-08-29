#include "../include/Scene.h"
#include "../external_libraries/common_include/pugixml.h"

struct transform_traverser: pugi::xml_tree_walker
{
	glm::mat4* transform;
	Scene* scene;
	virtual bool for_each(pugi::xml_node& node);
};

struct scene_traverser: pugi::xml_tree_walker
{
	Scene* scene;
    virtual bool for_each(pugi::xml_node& node);
};
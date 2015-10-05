#include "../include/xmlTraverser.h"

bool transform_traverser::for_each(pugi::xml_node& node)
{
    if(std::strncmp(node.name(),"transform",9) == 0)
    {
        if (std::strncmp(node.attribute("type").value(),"scale",5) == 0)
        {
            float x = std::stof(node.child("v").attribute("x").value());
            float y = std::stof(node.child("v").attribute("y").value());
            float z = std::stof(node.child("v").attribute("z").value());

            *transform = glm::scale(glm::mat4(), glm::vec3(x, y, z)) * *transform;
        }
        else if (std::strncmp(node.attribute("type").value(),"orientation",11) == 0)
        {
            float normal_x = std::stof(node.child("normal").attribute("x").value());
            float normal_y = std::stof(node.child("normal").attribute("y").value());
            float normal_z = std::stof(node.child("normal").attribute("z").value());

            float up_x = std::stof(node.child("up").attribute("x").value());
            float up_y = std::stof(node.child("up").attribute("y").value());
            float up_z = std::stof(node.child("up").attribute("z").value());

            *transform = glm::orientation(
                glm::vec3(normal_x, normal_y, normal_z),
                glm::vec3(up_x, up_y, up_z)) * *transform;
        }
        else if (std::strncmp(node.attribute("type").value(),"rotate",6) == 0)
        {
            float x = std::stof(node.child("v").attribute("x").value());
            float y = std::stof(node.child("v").attribute("y").value());
            float z = std::stof(node.child("v").attribute("z").value());

            float angle = std::stof(node.attribute("angle").value());

            *transform = glm::rotate(angle, glm::vec3(x, y, z)) * *transform;
        }
        else if (std::strncmp(node.attribute("type").value(),"translate",9) == 0)
        {
            float x = std::stof(node.child("v").attribute("x").value());
            float y = std::stof(node.child("v").attribute("y").value());
            float z = std::stof(node.child("v").attribute("z").value());

            *transform = glm::translate(
                glm::mat4(),
                glm::vec3(x, y, z)) * *transform;
        }
    }
    return true; // Continue traversal
}

bool scene_traverser::for_each(pugi::xml_node& node)
{
    if(std::strncmp(node.name(),"material",8) == 0)
    {
        std::string id = node.attribute("id").value();

        float reflectance = std::stof(node.attribute("reflectance").value());
        float specular_reflectance = std::stof(node.attribute("specular_reflectance").value());
        
        float transmissivity = std::stof(node.attribute("transmissivity").value());
        float refraction_index = std::stof(node.attribute("refraction_index").value());
        float diffuse_roughness = std::stof(node.attribute("diffuse_roughness").value());
        
        SpectralDistribution color_diffuse;
        SpectralDistribution color_specular;
        color_diffuse[0] = std::stof(node.child("color_diffuse").attribute("r").value());
        color_diffuse[1] = std::stof(node.child("color_diffuse").attribute("g").value());
        color_diffuse[2] = std::stof(node.child("color_diffuse").attribute("b").value());
        color_specular[0] = std::stof(node.child("color_specular").attribute("r").value());
        color_specular[1] = std::stof(node.child("color_specular").attribute("g").value());
        color_specular[2] = std::stof(node.child("color_specular").attribute("b").value());

        Material* m = new Material;

        m->color_diffuse = color_diffuse;
        m->color_specular = color_specular;
        m->reflectance = reflectance;
        m->specular_reflectance = specular_reflectance;
        
        m->transmissivity = transmissivity;
        m->refraction_index = refraction_index;
        m->diffuse_roughness = diffuse_roughness;

        scene->materials_.insert(std::pair<std::string, Material*>(id, m));
    }
    else if(std::strncmp(node.name(),"object3D",8) == 0)
    {
        std::string material_id = node.attribute("material_id").value();
        std::string type = node.attribute("type").value();
        Object3D* object;

        if (type == "plane")
        {
            glm::vec3 P0(
                std::stof(node.child("P0").attribute("x").value()),
                std::stof(node.child("P0").attribute("y").value()),
                std::stof(node.child("P0").attribute("z").value()));
            glm::vec3 P1(
                std::stof(node.child("P1").attribute("x").value()),
                std::stof(node.child("P1").attribute("y").value()),
                std::stof(node.child("P1").attribute("z").value()));
            glm::vec3 P2(
                std::stof(node.child("P2").attribute("x").value()),
                std::stof(node.child("P2").attribute("y").value()),
                std::stof(node.child("P2").attribute("z").value()));
            object = new Plane(P0, P1, P2, scene->materials_[material_id]);
        }
        if (type == "sphere")
        {
            glm::vec3 position(
                std::stof(node.child("position").attribute("x").value()),
                std::stof(node.child("position").attribute("y").value()),
                std::stof(node.child("position").attribute("z").value()));
            float radius = std::stof(node.attribute("radius").value());
            object = new Sphere(position, radius, scene->materials_[material_id]);
        }
        if (type == "mesh")
        {
            std::string file_path = node.attribute("file_path").value();

            // Compute the transform matrix by traversing current node
            glm::mat4 mesh_transform;
            transform_traverser walker;
            walker.transform = &mesh_transform;
            node.traverse(walker);

            object = new Mesh(mesh_transform, file_path.c_str(), scene->materials_[material_id]);
        }
        scene->objects_.push_back(object);
    }
    else if(std::strncmp(node.name(),"light_source",12) == 0)
    {
        float flux = std::stof(node.attribute("flux").value());
        std::string type = node.attribute("type").value();
        LightSource* lamp;

        if (type == "lamp_plane")
        {
            glm::vec3 P0(
                std::stof(node.child("P0").attribute("x").value()),
                std::stof(node.child("P0").attribute("y").value()),
                std::stof(node.child("P0").attribute("z").value()));
            glm::vec3 P1(
                std::stof(node.child("P1").attribute("x").value()),
                std::stof(node.child("P1").attribute("y").value()),
                std::stof(node.child("P1").attribute("z").value()));
            glm::vec3 P2(
                std::stof(node.child("P2").attribute("x").value()),
                std::stof(node.child("P2").attribute("y").value()),
                std::stof(node.child("P2").attribute("z").value()));
            SpectralDistribution color;
            color[0] = std::stof(node.child("color").attribute("r").value());
            color[1] = std::stof(node.child("color").attribute("g").value());
            color[2] = std::stof(node.child("color").attribute("b").value());

            lamp = new LightSource(P0, P1, P2, flux, color);
        }
        scene->lamps_.push_back(lamp);
    }
    return true; // Continue traversal
}
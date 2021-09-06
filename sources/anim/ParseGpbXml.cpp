#include "ParseGpbXml.h"

#include "pugixml.hpp"
#include <iostream>
#include <sstream>

std::vector<std::string> tokenize_operator(const std::string& data) {
	std::vector<std::string> result;
	std::string token;
	std::stringstream ss(data);

	while (ss >> token) {
		result.push_back(token);
	}
	return result;
}

glm::vec3 to_vec3(const std::vector<std::string>& data) {
    assert(data.size() == 3);
    glm::vec3 v;
    v.x = std::stof(data[0]);
    v.y = std::stof(data[1]);
    v.z = std::stof(data[2]);
    return v;
}

std::vector<float> to_float(const std::vector<std::string>& data) {
    std::vector<float> result(data.size());
    for (size_t i = 0; i < data.size(); i++)
        result[i] = std::stof(data[i]);
    return result;
}

std::vector<uint32_t> to_int(const std::vector<std::string>& data) {
    std::vector<uint32_t> result(data.size());
    for (size_t i = 0; i < data.size(); i++)
        result[i] = std::stoi(data[i]);
    return result;
}

bool GpbMesh::parse(pugi::xpath_node& node) {
    pugi::xml_node mesh = node.node();

    id = mesh.attribute("id").value();

    for (pugi::xml_node element : mesh.children("VertexElement")) {
        element.child("usage").text().get();
        element.child("size").text().get();
    }
    pugi::xml_node vertices = mesh.child("vertices");
    size_t vcount = vertices.attribute("count").as_int();
    std::string vertices_str = vertices.text().get();
    std::istringstream input(vertices_str);
    std::string line;
    std::getline(input, line);

    positions.reserve(vcount);
    normals.reserve(vcount);
    texcoords.reserve(vcount);

    while (std::getline(input, line)) {
        if (line.rfind("// position", 0) == 0) {
            glm::vec3 p;
            input >> p.x >> p.y >> p.z;
            positions.push_back(p);
        } else if (line.rfind("// normal", 0) == 0) {
            glm::vec3 n;
            input >> n.x >> n.y >> n.z;
            normals.push_back(n);
        } else if (line.rfind("// texCoord", 0) == 0) {
            glm::vec2 t;
            input >> t.x >> t.y;
            texcoords.push_back(t);
        }
        input.ignore();
    }

    pugi::xml_node bounds = mesh.child("bounds");
    min = to_vec3(tokenize_operator(bounds.child("min").text().get()));
    max = to_vec3(tokenize_operator(bounds.child("max").text().get()));
    center = to_vec3(tokenize_operator(bounds.child("center").text().get()));
    radius = std::stof(bounds.child("radius").text().get());

    pugi::xml_node part = mesh.child("MeshPart");
    pugi::xml_node indices_node  = part.child("indices");
    size_t icount = indices_node.attribute("count").as_int();
    indices = to_int(tokenize_operator(indices_node.text().get()));

    return true;
}

bool ParseGpbXml::parse(const std::string& filename) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filename.c_str());
    if (!result)
        return false;
        
    pugi::xpath_node_set mesh_node = doc.select_nodes("/root/Mesh");
    
    for (pugi::xpath_node node: mesh_node) {
        GpbMesh mesh;
        mesh.parse(node);
        meshes.push_back(mesh);
    }
    return true;
}

#include "include/plonk/scene.h"

Scene::Scene() {}
Scene::~Scene() {}

ResourceId Scene::add_mesh(Mesh mesh) {
	auto id = ResourceId::next();
	insert_mesh(id, mesh);
	return id;
}

void Scene::insert_mesh(ResourceId id, Mesh mesh) {
	meshes[id] = mesh;
}

bool Scene::has_mesh(ResourceId id) {
	return meshes.contains(id);
}

std::optional<std::reference_wrapper<Mesh>> Scene::get_mesh(ResourceId id) {
	auto it = meshes.find(id);
	if (it != meshes.end()) {
		return it->second;
	}
	return std::nullopt;
}

std::optional<Mesh> Scene::remove_mesh(ResourceId id) {
	auto it = meshes.find(id);
	if (it != meshes.end()) {
		auto mesh = std::move(it->second);
		meshes.erase(id);
		return mesh;
	}
	return std::nullopt;
}

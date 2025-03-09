#pragma once

#include "context.h"
#include "mesh.h"
#include "resource_id.h"

class Scene
{
public:
	Scene();
	~Scene();

	/**
	 * Add a new mesh to the scene
	 *
	 * @param mesh Mesh to add
	 * @return Auto-generated ID for the mesh
	 */
	ResourceId add_mesh(Mesh mesh);

	/**
	 * Insert or replace a mesh in the scene
	 *
	 * @param id Unique ID for the mesh
	 * @param mesh Mesh to add
	 * @return Auto-generated ID for the mesh
	 */
	void insert_mesh(ResourceId id, Mesh mesh);

	/**
	 * Test if a mesh exists
	 *
	 * @param id Unique ID for the mesh
	 * @return `true` if a mesh with that ID exists
	 */
	bool has_mesh(ResourceId id);

	/**
	 * Get a reference to a Mesh, if it exists
	 *
	 * @param id Unique ID for the mesh
	 * @return A reference to the Mesh if it exists, or none if not.
	 */
	std::optional<std::reference_wrapper<Mesh>> get_mesh(ResourceId id);

	/**
	 * Remove a Mesh from the scene
	 *
	 * @param id Unique ID for the mesh
	 * @return A reference to the Mesh if it existed, or none if not.
	 */
	std::optional<Mesh> remove_mesh(ResourceId id);

protected:
	ContextPtr ctx;
	std::unordered_map<ResourceId, Mesh> meshes;
};

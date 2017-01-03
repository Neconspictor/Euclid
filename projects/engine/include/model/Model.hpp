#pragma once
#include <vector>
#include <mesh/mesh.hpp>
#include <shader/Shader.hpp>

class Model
{
public:
	explicit Model(std::vector<Mesh*> meshes);
	Model(const Model& o);
	Model(Model&& o);
	Model& operator=(const Model& o);
	Model& operator=(Model&& o);

	virtual ~Model();

	void draw(Shader* shader);

	virtual std::vector<Mesh*> getMeshes() const;
	
protected:
	std::vector<Mesh*> meshes;
};

#pragma once
#include <vector>
#include <mesh/mesh.hpp>
#include <shader/Shader.hpp>

class Model
{
public:
	explicit Model(std::vector<std::reference_wrapper<Mesh>> meshReferences);
	Model(const Model&) = delete;

	Model(Model&& o);
	Model& operator=(Model&& o);
	Model& operator=(const Model& o) = delete;

	virtual ~Model();

	void draw(Shader* shader);

	virtual const std::vector<std::reference_wrapper<Mesh>>& getMeshes() const;
	
protected:
	std::vector<std::reference_wrapper<Mesh>> meshReferences;
};
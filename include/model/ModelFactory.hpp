#ifndef MODEL_FACTORY
#define MODEL_FACTORY

#include "model/Model.hpp"
#include <memory>

class ModelFactory
{
public:
	static std::shared_ptr<Model> texturedCube();
	static std::shared_ptr<Model> simpleLitCube();

private:
	static void modelRelease(Model* model);
};
#endif
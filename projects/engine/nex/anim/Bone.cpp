#include <nex/anim/Bone.hpp>

bool nex::Bone::operator==(const Bone& bone)
{
	return bone.mName == mName;
}

const std::string& nex::Bone::getName() const
{
	return mName;
}

void nex::Bone::setName(const std::string& name)
{
	mName = name;
}

const glm::mat4& nex::Bone::getBoneToParentTrafo() const
{
	return mBoneToParentTrafo;
}

void nex::Bone::setBoneToParentTrafo(const glm::mat4& mat)
{
	mBoneToParentTrafo = mat;
}

bool nex::Bone::isRoot() const
{
	return mParent == nullptr;
}

const nex::Bone* nex::Bone::getParent() const
{
	return mParent;
}

nex::Bone* nex::Bone::getParent()
{
	return mParent;
}

const std::vector<nex::Bone>& nex::Bone::getChildren() const
{
	mChildren;
}

std::vector<nex::Bone>& nex::Bone::getChildren()
{
	mChildren;
}
#pragma once

namespace tinygltf { class Model; }

class BasicModel{
private:
	std::string srcFile;


public:
	BasicModel();
	~BasicModel();

	void load(const char* assetFileName);
}
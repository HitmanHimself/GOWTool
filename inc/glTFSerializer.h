#pragma once
#include "pch.h"
#include "Mesh.h"
#include "Rig.h"
void WriteGLTF(const std::filesystem::path& path, const vector<RawMesh>& expMeshes, const Rig& Armature);
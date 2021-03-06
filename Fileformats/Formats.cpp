#pragma once
#include <cstdint>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <3DStructs.h>
#include <Mesh.h>

class Lodpack
{
	
public:
	uint32_t groupCount;
	uint32_t* groupStartOff;
	uint64_t* groupHash;
	uint32_t* groupBlockSize;

	
	uint32_t TotalmembersCount;
	uint32_t* memberGroupIndex;
	uint32_t* memberOffsetter;
	uint64_t* memberHash;
	uint32_t* memberBlockSize;
	
	void ReadLodpack(std::string filename)
	{
		std::ifstream file;

		file.open(filename, std::ios::in | std::ios::binary);
		file.read((char *) &groupCount, sizeof(uint32_t));

		groupStartOff = new uint32_t[groupCount];
		groupHash = new uint64_t[groupCount];
		groupBlockSize = new uint32_t[groupCount];

		file.seekg(16);
		for (uint32_t i = 0; i < groupCount; i++)
		{
			file.read((char *) &groupStartOff[i], sizeof(uint32_t));
			file.seekg(4, file.cur);
		//	std::cout << "\noffset = " << file.tellg() << " index = " << i;
			file.read((char*) &groupHash[i], sizeof(uint64_t));
			file.read((char*) &groupBlockSize[i], sizeof(uint32_t));
			file.seekg(4, file.cur);
		//	std::cout << "\noffset = " << file.tellg() << " index = " << i;
		}

		file.seekg(4);
		file.read((char*) &TotalmembersCount, sizeof(uint32_t));

		memberGroupIndex = new uint32_t[TotalmembersCount];
		memberOffsetter = new uint32_t[TotalmembersCount];
		memberHash = new uint64_t[TotalmembersCount];
		memberBlockSize = new uint32_t[TotalmembersCount];
		
		uint32_t offset = 24 * groupCount + 16;
		file.seekg(offset);
		
		for (uint32_t e = 0; e < TotalmembersCount; e++)
		{
			file.read((char*) &memberGroupIndex[e], sizeof(uint32_t));
			file.read((char*) &memberOffsetter[e], sizeof(uint32_t));
			file.read((char*) &memberHash[e], sizeof(uint64_t));
			file.read((char*) &memberBlockSize[e], sizeof(uint32_t));
			file.seekg(4, std::ios::cur);
		}

		file.close();
	}
};
class MGDefinition
{
public:
	uint16_t defCount;
	uint32_t* defOffsets;

	uint32_t meshCount;
	std::vector<uint32_t> indicesOffsetter;
	std::vector<uint32_t> verticesOffsetter;
	std::vector<uint32_t> indicesCount;
	std::vector<uint32_t> verticesCount;
	std::vector<Vec3> meshScale;
	std::vector<Vec3> meshMin;
	std::vector<uint32_t> vertexBlockOffsetter;
	std::vector<uint64_t> meshHash;
	std::vector<uint16_t> buffCount;
	std::vector<uint16_t> CompCount;
	std::vector<std::vector<MeshComp>> Components;

	std::vector<uint16_t> LODlvl;
	std::vector<uint16_t> boneAssociated;
	void ReadMG(std::string filename)
	{
		meshCount = 0;
		std::ifstream file(filename, std::ios::in | std::ios::binary);
		file.seekg(56);
		file.read((char*)&defCount, sizeof(uint16_t));

		defOffsets = new uint32_t[defCount];

		file.seekg(76);
		for (uint16_t i = 0; i < defCount; i++)
		{
			file.read((char*)&defOffsets[i], sizeof(uint32_t));
		}

		for (uint16_t i = 0; i < defCount; i++)
		{
			uint32_t offset = defOffsets[i];
			
			file.seekg((uint64_t)offset);

			uint16_t boneAsso = UINT16_MAX;
			file.read((char*)&boneAsso, sizeof(uint16_t));
			uint8_t subMeshCount = UINT8_MAX;
			file.read((char*)&subMeshCount, sizeof(uint8_t));

			for (uint8_t e = 0; e < subMeshCount; e++)
			{
				uint32_t subMeshOffsetter = UINT32_MAX;
				file.seekg((uint64_t)offset + 60 + (uint64_t)e * 4);
				file.read((char*)&subMeshOffsetter, sizeof(uint32_t));

				uint32_t partCount = UINT32_MAX;
				file.seekg((uint64_t)offset + (uint64_t)subMeshOffsetter);
				file.read((char*)&partCount, sizeof(uint32_t));

				if (partCount == 0)
					continue;

				uint32_t partOffset = (uint32_t)file.tellg() + 8;
				for (uint32_t c = 0; c < partCount; c++)
				{
					LODlvl.push_back((uint16_t)e);
					boneAssociated.push_back(boneAsso);
					file.seekg((uint64_t)partOffset + (uint64_t)c * 4);
					uint32_t off = UINT32_MAX;
					file.read((char*)&off, sizeof(uint32_t));
					off += partOffset + c * 4;

					uint32_t smBaseOff = off;
					file.seekg((uint64_t)smBaseOff + 48);

					uint32_t indOff = UINT32_MAX;
					file.read((char*)&indOff, sizeof(uint32_t));
					indicesOffsetter.push_back(indOff);

					file.seekg(4, std::ios::cur);
					uint32_t vertOff = UINT32_MAX;
					file.read((char*)&vertOff, sizeof(uint32_t));
					verticesOffsetter.push_back(vertOff);

					file.seekg(4, std::ios::cur);
					uint32_t vertC = UINT32_MAX;
					file.read((char*)&vertC, sizeof(uint32_t));
					verticesCount.push_back(vertC);
					uint32_t indC = UINT32_MAX;
					file.read((char*)&indC, sizeof(uint32_t));
					indicesCount.push_back(indC);

					file.seekg((uint64_t)smBaseOff + 16);
					Vec3 extent;
					file.read((char*) &extent.X, sizeof(float));
					file.read((char*) &extent.Y, sizeof(float));
					file.read((char*) &extent.Z, sizeof(float));
					Vec3 origin;
					file.read((char*) &origin.X, sizeof(float));
					file.read((char*) &origin.Y, sizeof(float));
					file.read((char*) &origin.Z, sizeof(float));

					//std::cout << extent.X << " " << extent.Y << " " << extent.Z << " " << origin.X << " " << origin.Y << " " << origin.Z << std::endl;
					Vec3 scale(extent.X * 2, extent.Y * 2, extent.Z * 2);
					Vec3 min(origin.X - extent.X, origin.Y - extent.Y, origin.Z - extent.Z);

					meshScale.push_back(scale);
					meshMin.push_back(min);
					file.seekg((uint64_t)smBaseOff + 84);

					uint32_t vertexBlockInfoOffset = UINT32_MAX;
					file.read((char*)&vertexBlockInfoOffset, sizeof(uint32_t));
					uint32_t vertexBlockOffsetterOff = UINT32_MAX;
					file.read((char*)&vertexBlockOffsetterOff, sizeof(uint32_t));

					vertexBlockOffsetter.push_back(smBaseOff + vertexBlockOffsetterOff);
					//std::cout << vertexBlockInfoOffset << " " << vertexBlockOffOffsetter <<std::endl;
					
					uint64_t Hash = UINT64_MAX;
					file.read((char*)&Hash, sizeof(uint64_t));
					meshHash.push_back(Hash);

					file.seekg((uint64_t)smBaseOff + (uint64_t)vertexBlockInfoOffset - 8);

					uint8_t buffC = UINT8_MAX;
					file.read((char*)&buffC, sizeof(uint8_t));
					file.seekg(2, std::ios::cur);
					uint8_t compC = UINT8_MAX;
					file.read((char*)&compC, sizeof(uint8_t));

					buffCount.push_back(buffC);
					CompCount.push_back(compC);
					//std::cout << file.tellg() << " "<< (uint16_t)buffC << " " << (uint16_t)compC << std::endl;

					file.seekg((uint64_t)smBaseOff + (uint64_t)vertexBlockInfoOffset);
					
					std::vector<MeshComp> tempCompList;
					for (uint16_t d = 0; d < compC; d++)
					{
						uint8_t compID = UINT8_MAX;
						file.read((char*)&compID, sizeof(uint8_t));
						uint8_t dataType = UINT8_MAX;
						file.read((char*)&dataType, sizeof(uint8_t));
						uint8_t elemCount = UINT8_MAX;
						file.read((char*)&elemCount, sizeof(uint8_t));
						uint8_t strider = UINT8_MAX;
						file.read((char*)&strider, sizeof(uint8_t));
						uint8_t index = UINT8_MAX;
						file.read((char*)&index, sizeof(uint8_t));
						file.seekg(3, std::ios::cur);
						
						//std::cout << (uint16_t)compId << " " << (uint16_t)dataType << " " << (uint16_t)elemCount << " " << (uint16_t)strider << " " << (uint16_t)index << std::endl;
						MeshComp tempComp;
						tempComp.compID = (ComponentTypes)compID;
						tempComp.dataType = (uint16_t)dataType;
						tempComp.elemCount = (uint16_t)elemCount;
						tempComp.strider = (uint16_t)strider;
						tempComp.index = (uint16_t)index;

						//std::cout << tempComp.compID << " " << (uint16_t)tempComp.dataType << " " << (uint16_t)tempComp.elemCount << " " << (uint16_t)tempComp.strider << " " << (uint16_t)tempComp.index << std::endl;
						tempCompList.push_back(tempComp);
					}
					Components.push_back(tempCompList);

					meshCount++;
				}
			}
		}
		
		/*
		for (uint32_t i = 0; i < verticesCount.size(); i++)
		{
			//std::cout << indicesOffsetter[i] << " " << verticesOffsetter[i] << " " << indicesCount[i] << " " << verticesCount[i] << std::endl;
			//std::cout << meshHash[i] <<std::endl;
		}
		*/
		file.close();
	}
};
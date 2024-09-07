// proto.cpp : 定义控制台应用程序的入口点。
//

#include "stdint.h"

#include <vector>
#include <iostream>
#include <fstream>
#include<cstdio>

#include <iostream>   
#include <string> 
#include "label.pb.h"
struct MyPoint {
	int x;
	int y;
};

struct labelInfo {
	std::vector<MyPoint> allPoint;
	float score;
	//std::string* name;
	std::string readName;

};
struct AllInfo
{

	float imageHeight;
	float imageWidth;
	std::string *ImageName;
	std::vector<labelInfo> allRegions;
};


void readBinFile(std::string BinName)
{
	labelInfo myLabelInfo;

	std::string filePath = BinName;

	FILE* pFile = nullptr;
	if (fopen_s(&pFile, filePath.c_str(), "rb") != 0)
	{
		perror("Error opening file");
		return;
	}

	// 获取文件大小
	fseek(pFile, 0, SEEK_END);
	long fileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	// 分配内存缓冲区
	char *buffer = new char[fileSize];

	// 读取文件内容到缓冲区
	size_t bytesRead = fread(buffer, sizeof(char), fileSize, pFile);

	if (bytesRead == fileSize)
	{
		cf::glacier::Label msg2;
		{
			if (!msg2.ParseFromArray(buffer, fileSize)) {
				std::cerr << "Failed to parse address book." << std::endl;
				return;
			}
		}


		std::vector<labelInfo> imgAllInfo;
		for (int i = 0; i < msg2.regions().size(); i++) {
			labelInfo tempInfo;
			std::vector<MyPoint> tempVector;
			MyPoint tempPoint;
			auto myPoint = msg2.regions().at(i).polygon().outer().points();

			for (int j = 0; j < myPoint.size(); j++) {
				tempPoint.x = myPoint.at(j).x();
				tempPoint.y = myPoint.at(j).y();

				tempVector.push_back(tempPoint);

			}
			tempInfo.allPoint = tempVector;
			tempInfo.readName = msg2.regions().at(i).name();
			tempInfo.score = msg2.regions().at(i).score();
			imgAllInfo.push_back(tempInfo);

		}

	//	//遍历信息
	//	for (const auto& info : imgAllInfo) {
	//		std::cout << "Score: " << info.score << std::endl;
	//		std::cout << "Name: " << info.readName << std::endl;

	//		std::cout << "Points:" << std::endl;
	//		for (const auto& point : info.allPoint) {
	//			std::cout << "x: " << point.x << ", y: " << point.y << std::endl;
	//		}

	//		std::cout << std::endl;
	//	}
	}


	delete buffer;
	std::cout << "success run" << std::endl;
};

void writeBinFile(AllInfo tempInfo, const char * binPath)
{

	cf::glacier::Label label;

	auto imgSize =  cf::glacier::Size2f();
	imgSize.set_height(tempInfo.imageHeight);
	imgSize.set_width(tempInfo.imageWidth);
	label.set_allocated_img_size(&imgSize);

	//std::string *p = new std::string;
	std::string p = "ttt";
	label.set_allocated_name(&p);
	
	

	for (int i = 0; i < tempInfo.allRegions.size(); i++) {

		labelInfo tempLabelInfo = tempInfo.allRegions.at(i);
		// 添加区域
		auto region = label.add_regions();
		auto outer =  new cf::glacier::Ring();
		auto polygon =  new cf::glacier::Polygon();


		polygon->set_allocated_outer(outer);

		region->set_allocated_polygon(polygon);

		// 设置名字
		//auto flawName = new std::string(tempLabelInfo.readName);
		//region->set_allocated_name(flawName);

		region->set_name(tempLabelInfo.readName.c_str());


		// 设置最小分数
		region->set_score(tempLabelInfo.score);
		
		// 添加点
		for (int j = 0; j < tempLabelInfo.allPoint.size(); j++) {
			auto p2f = outer->add_points();
			p2f->set_x(tempLabelInfo.allPoint.at(j).x);
			p2f->set_y(tempLabelInfo.allPoint.at(j).y);

		}

	}

	size_t sz = label.ByteSizeLong();
	uint8_t *outPb = (uint8_t *)malloc(sz);
	label.SerializeToArray(outPb, sz);

	// 将数据写入文件
	FILE *myFout;
	if (fopen_s(&myFout, binPath, "wb") != 0) {
		std::cerr << "Error: Unable to open file for writing." << std::endl;
		delete myFout;
		
		return;
	}
	fwrite(outPb, sz, 1, myFout);
	fclose(myFout);

	// 释放内存
	label.release_img_size();
	label.release_name();
	for (int i = 0; i < label.regions_size(); i++) {
		auto region = label.mutable_regions(i);
		auto polygon = region->release_polygon();
		auto outer = polygon->release_outer();


		//region->release_polygon();


		delete outer;
		delete polygon;
		//delete region;
		//region->release_name();


	}



	free(outPb);
	
	label.Clear();
	
	google::protobuf::ShutdownProtobufLibrary();
	
	label.clear_regions();  // 清空 regions
};

void mai1n()
{

	AllInfo myInfo;
	myInfo.imageHeight = 10;
	myInfo.imageWidth = 100;
	myInfo.ImageName = new std::string("ttt");


	labelInfo region1;
	region1.score = 0.8;
	region1.readName = "Region 1";


	MyPoint point1;
	point1.x = 50.0;
	point1.y = 100.0;
	region1.allPoint.push_back(point1);

	MyPoint point2;
	point2.x = 60.0;
	point2.y = 120.0;
	region1.allPoint.push_back(point2);


	myInfo.allRegions.push_back(region1);

	for (int i = 0; i < 100000; i++)
	{
		std::string filename = std::to_string(i) + ".bin";
		writeBinFile(myInfo, filename.c_str());

		//readBinFile(filename.c_str());
	}
	system("pause");
}


// proto.cpp : 定义控制台应用程序的入口点。
//

#include "stdint.h"

#include <vector>
#include <iostream>
#include <fstream>
#include<cstdio>
#include "label.pb.h"
struct MyPoint {
	int x;
	int y;
	int pos_x;
	int pos_y;
};

struct labelInfo {
	std::vector<MyPoint> allPoint;
	float score;
	//std::string* name;
	float area;
	float diagonal;
	float width;
	float height;
	float widthAndHeightRatio;
	float grayDiff;
	float display;
	std::string name;

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

				tempPoint.pos_x = myPoint.at(j).pos_x();
				tempPoint.pos_y = myPoint.at(j).pos_y();

				tempVector.push_back(tempPoint);

			}
			tempInfo.allPoint = tempVector;
			tempInfo.name = msg2.regions().at(i).name();
			tempInfo.score = msg2.regions().at(i).score();
			tempInfo.area = msg2.regions().at(i).area();
			tempInfo.diagonal = msg2.regions().at(i).diagonal();
			tempInfo.width = msg2.regions().at(i).width();
			tempInfo.height = msg2.regions().at(i).height();
			tempInfo.widthAndHeightRatio = msg2.regions().at(i).widthandheightratio();
			tempInfo.grayDiff = msg2.regions().at(i).graydiff();
		//	tempInfo.display = msg2.regions().at(i).display();
			imgAllInfo.push_back(tempInfo);

		}

		//遍历信息
		for (const auto& info : imgAllInfo) {
			std::cout << "Score: " << info.score << std::endl;
			std::cout << "Name: " << info.name << std::endl;
			std::cout << "area: " << info.area << std::endl;
			std::cout << "diagonal: " << info.diagonal << std::endl;
			std::cout << "width: " << info.width << std::endl;
			std::cout << "height: " << info.height << std::endl;
			std::cout << "widthAndHeightRatio: " << info.widthAndHeightRatio << std::endl;
			std::cout << "grayDiff: " << info.grayDiff << std::endl;
			std::cout << "display: " << info.display << std::endl;
			std::cout << "Points:" << std::endl;
			for (const auto& point : info.allPoint) {
				std::cout << "x: " << point.x << ", y: " << point.y << "pos_x: " << point.pos_x << ", pos_y: " << point.pos_y << std::endl;
			}

			std::cout << std::endl;
		}
	}



	std::cout << "success run" << std::endl;
};

void writeBinFile(AllInfo tempInfo, const char * binPath)
{

	cf::glacier::Label label;

	auto imgSize = new cf::glacier::Size2f();
	imgSize->set_height(tempInfo.imageHeight);
	imgSize->set_width(tempInfo.imageWidth);
	label.set_allocated_img_size(imgSize);

	std::string *p = new std::string("tt");
	//p = tempInfo.ImageName;
	label.set_allocated_name(tempInfo.ImageName);

	for (int i = 0; i < tempInfo.allRegions.size(); i++) {

		labelInfo tempLabelInfo = tempInfo.allRegions.at(i);
		// 添加区域
		auto region = label.add_regions();
		auto outer = new cf::glacier::Ring();
		auto polygon = new cf::glacier::Polygon();
		polygon->set_allocated_outer(outer);
		region->set_allocated_polygon(polygon);

		// 设置名字
		auto flawName = new std::string(tempLabelInfo.name);
		region->set_allocated_name(flawName);

		std::cout << tempLabelInfo.score << std::endl;
		std::cout << tempLabelInfo.area << std::endl;
		std::cout << tempLabelInfo.name << std::endl;
			
		// 设置最小分数
		region->set_score(tempLabelInfo.score);
		region->set_area(tempLabelInfo.area);
		region->set_diagonal(tempLabelInfo.diagonal);
		region->set_width( tempLabelInfo.width);
		region->set_height(tempLabelInfo.height);
		region->set_widthandheightratio(tempLabelInfo.widthAndHeightRatio);
		region->set_graydiff(tempLabelInfo.grayDiff);
		//region->set_display(tempLabelInfo.display);

		// 添加点
		for (int j = 0; j < tempLabelInfo.allPoint.size(); j++) {
			auto p2f = outer->add_points();
			p2f->set_x(tempLabelInfo.allPoint.at(j).x);
			p2f->set_y(tempLabelInfo.allPoint.at(j).y);
			std::cout << tempLabelInfo.allPoint.at(j).pos_x << std::endl;
			std::cout << tempLabelInfo.allPoint.at(j).pos_y << std::endl;
			p2f->set_pos_x(tempLabelInfo.allPoint.at(j).pos_x);
			p2f->set_pos_y(tempLabelInfo.allPoint.at(j).pos_y);
		}
	}

	size_t sz = label.ByteSizeLong();
	uint8_t *outPb = (uint8_t *)malloc(sz);
	label.SerializeToArray(outPb, sz);

	// 将数据写入文件
	FILE *myFout;
	if (fopen_s(&myFout, binPath, "wb") != 0) {
		std::cerr << "Error: Unable to open file for writing." << std::endl;
		return;
	}
	fwrite(outPb, sz, 1, myFout);
	fclose(myFout);



	free(outPb);
};

void main()
{

	AllInfo myInfo;
	myInfo.imageHeight = 10;
	myInfo.imageWidth = 100;
	myInfo.ImageName = new std::string("ttt");


	labelInfo region1;
	region1.score = 0.8;
	region1.name = "Region 1";
	region1.area = 222.1;
	region1.diagonal = 3333.1;
	region1.width = 5;
	region1.height = 6;
	region1.widthAndHeightRatio = 7;
	region1.grayDiff = 8;
	region1.display = 1;


	MyPoint point1;
	point1.x = 50.0;
	point1.y = 100.0;
	point1.pos_x = 11150.0;
	point1.pos_y = 111100.0;
	region1.allPoint.push_back(point1);

	MyPoint point2;
	point2.x = 60.0;
	point2.y = 120.0;
	point2.pos_x = 22250.0;
	point2.pos_y = 22200.0;
	region1.allPoint.push_back(point2);


	myInfo.allRegions.push_back(region1);

	//printf("1");
	writeBinFile(myInfo, "2.bin");
	
	readBinFile("2.bin");
	system("pause");
}


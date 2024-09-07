#define _CRT_SECURE_NO_WARNINGS
#include "stdint.h"
#include "DetectEngine.h"
#include "DetectEngineDbg.h"
#include "label.pb.h"
#include "thread_adapter.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <string.h>

using namespace cv;




int imgId = 0;
struct MyPoint {
	int x;
	int y;
};

struct labelInfo {
	std::vector<MyPoint> allPoint;
	int score;
	std::string name;
};

int CFInit(int engId)
{

	imgId++;
	int ret = 0;
	//检测引擎初始化
	ret = DetectEngineInit();
	//检测引擎debug
	ret = DetectEngineDebug(DEBUG_CMD_LOG_OUTPUT, 0, LOG_OUTPUT_OPTION_FLOW);
	ret = DetectEngineDebug(DEBUG_CMD_CHANNEL_DEBUG_FLAGS, 0, 0x1);
	//检测引擎是否打开
	ret = DetectEngineOpen(engId);
	if (ret != 0) {
		printf("open failed with error code %d\n", ret);
		return -1;
	}
	//检测引擎加载
	ret = DetectEngineLoad(engId, "E:/glacier/glacier6/glacier/test/ci/resources/weights/yolov5s-coco/config.yml");
	if (ret != 0) {
		printf("load failed with error code %d\n", ret);
		return -1;
	}
	//检测引擎开始
	ret = DetectEngineStart(engId);
	if (ret != 0) {
		printf("start failed with error code %d\n", ret);
		return -1;
	}

	return ret;

}

int cfPredictPic(std::string imgPath, std::vector<labelInfo> imgAllInfo,std::string labelSavePath = "")
{

	Mat src = imread(imgPath);
	// Mat src1 = imread(argv[3]);
	int ret = 0;
	int engId = 0;
	struct Image img = { 0 };
	struct LabelPb labelPb;
	img.height = src.rows;
	img.width = src.cols;
	img.buffer = src.data;
	img.channels = src.channels();
	img.pitch = img.width * img.channels;
	img.id = imgId;
	//检测引擎入队图片
	ret = DetectEngineEnqueueImage(engId, &img);

	//THREAD_Sleep(10000);


	//检测引擎获取标签
	ret = DetectEngineGetLabel(engId, &labelPb);

	//保存到本地
	if (labelSavePath != "")
	{
		FILE *fout = fopen(labelSavePath.c_str(), "wb");
		fwrite(labelPb.data, labelPb.length, 1, fout);
		fclose(fout);
	}

	//从内存读取
	cf::glacier::Label msg;
	if (!msg.ParseFromArray(labelPb.data, labelPb.length)) {
		std::cerr << "Failed to parse address book." << std::endl;
		return 1;
	}
	
	//赋值
	for (int i = 0; i < msg.regions().size(); i++) {
		labelInfo tempInfo;
		std::vector<MyPoint> tempVector;
		MyPoint tempPoint;
		auto myPoint = msg.regions().at(i).polygon().outer().points();

		for (int j = 0; j < myPoint.size(); j++) {
			tempPoint.x = myPoint.at(j).x();
			tempPoint.y = myPoint.at(j).y();

			tempVector.push_back(tempPoint);

		}
		tempInfo.allPoint = tempVector;
		tempInfo.name = msg.regions().at(i).name();
		tempInfo.score = msg.regions().at(i).score();
		imgAllInfo.push_back(tempInfo);

	}

	//检测引擎释放标签
	ret = DetectEngineReleaseLabel(engId, &labelPb);
	return 0;
}

int analysisLabel(std::string labelPath, std::vector<labelInfo> &imgAllInfo)
{

	labelInfo myLabelInfo;

	std::string filePath = labelPath;

	FILE* pFile = nullptr;
	if (fopen_s(&pFile, filePath.c_str(), "rb") != 0)
	{
		perror("Error opening file");
		return 1;
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
				
			}
		}


		
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
			tempInfo.name = msg2.regions().at(i).name();
			tempInfo.score = msg2.regions().at(i).score();
			imgAllInfo.push_back(tempInfo);
			std::cout << tempInfo.name << std::endl;

		}
	}

	return 0;
}

int main1(int argc, char *argv[])
{
	//engId一个eng对应一个模型，多个模型有多个eng
	CFInit(0);
	std::vector<labelInfo> imgAllInfo;

	//cfPredictPic("E:/glacier/glacier6/glacier/test/ci/resources/images/bus.jpg", imgAllInfo, "E:/onnxruntime-main/vsproto/labelBus1.cflabel");

	std::vector<labelInfo> testAllInfo;
	analysisLabel("E:/onnxruntime-main/vsproto/labelBus1.cflabel", testAllInfo);
    (void)argc;
    (void)argv;
    return 0;
}
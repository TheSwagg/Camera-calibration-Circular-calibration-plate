
// circleTargetCameraCalibration.cpp : 定义控制台应用程序的入口点。
#include<opencv2/opencv.hpp>
#include<vector>
#include<iostream>
#include <io.h>
#include <fstream>

using namespace cv;
using namespace std;

void getFilesName(string& File_Directory, string& FileType, vector<string>& FilesName)
{
    string buffer = File_Directory + "/*" + FileType;

    _finddata_t c_file;   // 存放文件名的结构体

    long long hFile;
    hFile = _findfirst(buffer.c_str(), &c_file);   //找第一个文件名

    if (hFile == -1L)   // 检查文件夹目录下存在需要查找的文件
        cout << "No " << FileType << "files in current directory!" << endl;
    else
    {
        string fullFilePath;
        do
        {
            fullFilePath.clear();

            //名字
            fullFilePath = File_Directory + "\\" + c_file.name;

            FilesName.push_back(fullFilePath);

        } while (_findnext(hFile, &c_file) == 0);  //如果找到下个文件的名字成功的话就返回0,否则返回-1
        _findclose(hFile);
    }
}

void m_calibration(vector<string>& FilesName, Size board_size, Size square_size, Mat& cameraMatrix, Mat& distCoeffs, vector<Mat>& rvecsMat, vector<Mat>& tvecsMat)
{
    /***0.定义变量并设置参数***/
    ofstream fout("circle_caliberation_result.txt");                       // 保存标定结果的文件
    int image_count = 0;                                            // 图像张数
    Size image_size;                                                // 图像的尺寸
    vector<Point2f> image_points;                                   // 缓存每幅图像上检测到的角点
    vector<vector<Point2f>> image_points_seq;                       // 保存检测到的所有角点
    //创建SimpleBlobDetector的检测器，设置斑点检测（SimpleBlobDetector）的参数
    SimpleBlobDetector::Params params;
    //params.maxArea = 10e4;		//斑点的最大面积
    params.minThreshold = 30;		//***二值化的起始阈值，即公式1的T1//数值越大偏黑的部分被忽略，数值越大稳定越差？也影响求解速度
    params.maxThreshold = 200;		//最大二值化值
    params.thresholdStep = 10;		//阈值越小检测越精确，计算速度变慢
    params.filterByInertia = true;	//斑点圆度的限制变量，默认是不限制
    params.filterByColor = true;	//斑点颜色的限制变量
    params.blobColor = 0;			//255表示只提取白色斑点，0表示只提取黑色斑点
    params.filterByArea = true;		//斑点面积的限制变量
    params.minArea = 60;			//斑点的最小面积最大取到120
    //最小的斑点距离，不同二值图像的斑点间距离小于该值时，被认为是同一个位置的斑点，否则是不同位置上的斑点
    params.minDistBetweenBlobs = 15;//最大22左右,15比较合适
    Ptr<FeatureDetector> blobDetector = SimpleBlobDetector::create(params);
    //Ptr<SimpleBlobDetector> blobDetector = SimpleBlobDetector::create(params);//也可以这种写法

    /***1.提取所有图片中的圆心点（角点）***/
    cout << "开始提取角点………………" << endl;
    for (int i = 0; i < FilesName.size(); i++)
    {
        image_count++;
        // 用于观察检验输出
        cout << "image_count = " << image_count << endl;
        Mat imageInput = imread(FilesName[i]);
        imshow("qwewqe", imageInput);
        if (image_count == 1)  //读入第一张图片时获取图像宽高信息
        {
            image_size.width = imageInput.cols;
            image_size.height = imageInput.rows;
            cout << "image_size.width = " << image_size.width << endl;
            cout << "image_size.height = " << image_size.height << endl;
        }

        //主要用作调试斑点检测的参数，斑点检测检测圆，获得圆心
        //std::vector<cv::KeyPoint> keyPoints1;
        //cv::Mat displayImg = imageInput.clone();
        //blobDetector->detect(displayImg, keyPoints1);
        //cv::drawKeypoints(displayImg, keyPoints1, displayImg,
        //	cv::Scalar(0, 0, 255), cv::DrawMatchesFlags::DEFAULT);
        //
        //缩小显示
        //cv::Mat dst = Mat::zeros(512, 640, CV_8UC3); //我要转化为512*512大小的
        //resize(displayImg, dst, dst.size());
        //cv::imshow("斑点检测result", dst);
        //cv::waitKey(200);


        /*** 2.提取角点 ***/
        /***********圆形标定板***********/
        //param1:标定图片 param2:标定板的尺寸 param3:输出检测到的圆心坐标
        //param4:标志位，对称图案—cv::CALIB_CB_SYMMETRIC_GRID,非对称图案—cv::CALIB_CB_ASYMMETRIC_GRID
        //param5:检测算法 斑点检测

        //用于圆形标定板
        bool detectorFlag = findCirclesGrid(imageInput, board_size, image_points, CALIB_CB_SYMMETRIC_GRID, blobDetector);

        //用于方格
        //bool detectorFlag = findChessboardCorners(imageInput, board_size, image_points, CV_CALIB_CB_ADAPTIVE_THRESH);//用于棋盘格检测
        if (detectorFlag == false)
        {	//检测失败，显示检测失败的点
            cout << "imgae_points size = " << image_points.size() << std::endl;
            cout << "第" << image_count << "张照片提取圆心失败，请删除后，重新标定！" << endl; //找不到角点
            //drawChessboardCorners(imageInput, board_size, Mat(image_points), true);//在原图中绘制找到的圆心点
            for (size_t i = 0; i < image_points.size(); i++) {
                cv::circle(imageInput, image_points[i], 5, cv::Scalar(0, 0, 255), 3);
            }
            imshow("检测失败照片", imageInput);
            waitKey(200);
        }
        else {
            cout << "!!!!!!!!!!!!!!!!" << endl;
            cout << "!!!!!!!!!!!!!!!!" << endl;
            cout << "!!!!!!!!!!!!!!!!" << endl;
            cout << "!!!!!!!!!!!!!!!!" << endl;
            cout << "!!!!!!!!!!!!!!!!" << endl;
        }
        //检测成功，显示检测到的点
        drawChessboardCorners(imageInput, board_size, Mat(image_points), true);//在原图中绘制找到的圆心点
        cv::Mat dst2 = Mat::zeros(512, 640, CV_8UC3); //我要转化为512*512大小的
        resize(imageInput, dst2, dst2.size());
        imshow("检测完毕", dst2);
        waitKey(200);
        image_points_seq.push_back(image_points);
        //else		//此部分用于棋盘格检测
        //{
        //	Mat view_gray;
        //	cout << "imageInput.channels()=" << imageInput.channels() << endl;
        //	cvtColor(imageInput, view_gray, CV_RGB2GRAY);

        //	/* 亚像素精确化 */
        //	//find4QuadCornerSubpix(view_gray, image_points, Size(5, 5)); //对粗提取的角点进行精确化
        //	cv::cornerSubPix(view_gray, image_points, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 20, 0.01));

        //	image_points_seq.push_back(image_points);  //保存亚像素角点

        //											   /* 在图像上显示角点位置 */
        //	drawChessboardCorners(view_gray, board_size, image_points, true);

        //	//imshow("Camera Calibration", view_gray);//显示图片
        //	//waitKey(100);//暂停0.1S
        //}

    }
    cout << "角点提取完成！！！" << endl;
    cv::destroyAllWindows();//关掉所有的显示窗口

    cout << "提取的坐标点" << endl;
    for (size_t i = 0; i < image_points_seq[0].size(); i++)
    {
        cout << image_points_seq[0][i] << endl;
    }

    /***3.生成标定板三维坐标信息***/
    vector<vector<Point3f>> object_points_seq;      // 三维坐标点的向量的向量（一定是这个格式，后边标定函数格式要求）

    //非对称标定板(圆形)
    //for (int t = 0; t < image_count; t++)
    //{
    //	vector<Point3f> object_points;
    //	for (int i = 0; i < board_size.height; i++)//行数7
    //	{
    //		for (int j = 0; j < board_size.width; j++)//列数7
    //		{
    //			Point3f realPoint;
    //			/* 假设标定板放在世界坐标系中z=0的平面上 */
    //			if (i % 2 == 0) {//为偶数行时
    //				realPoint.x = i * square_size.width;
    //				realPoint.y = (2 * j)*square_size.height;
    //				realPoint.z = 0;
    //			}
    //			else {//为奇数行
    //				realPoint.x = i * square_size.width;
    //				realPoint.y = (2 * j + 1)*square_size.height;
    //				realPoint.z = 0;
    //			}
    //			object_points.push_back(realPoint);
    //		}
    //	}
    //	object_points_seq.push_back(object_points);
    //}

    //对称标定板（圆形）x轴向下，y轴向右,z轴向外
    for (int t = 0; t < image_count; t++)
    {
        vector<Point3f> object_points;
        for (int i = 0; i < board_size.height; i++)//行数7
        {
            for (int j = 0; j < board_size.width; j++)//列数7
            {
                Point3f realPoint;
                /* 假设标定板放在世界坐标系中z=0的平面上 */
                realPoint.x = i * square_size.width;
                realPoint.y = j * square_size.height;
                realPoint.z = 0;
                object_points.push_back(realPoint);
            }
        }
        object_points_seq.push_back(object_points);
    }

    cout << "标定板的3D坐标" << endl;
    for (int i = 0; i < object_points_seq[0].size(); i++)
    {
        cout << object_points_seq[0][i] << endl;
    }

    /*** 4.运行标定函数 ***/
    cout << "标定计算时间较久" << endl;
    double err_first = calibrateCamera(object_points_seq, image_points_seq, image_size, cameraMatrix, distCoeffs, rvecsMat, tvecsMat, CALIB_FIX_K3);
    fout << "标定重投影误差：" << err_first << "像素" << endl << endl;
    cout << "标定完成！！！" << endl;


    /***5.对标定结果进行评价***/
    //对标定结果进行评价的方法是通过得到的像机内外参数，对空间的三维点进行重新投影计算，
    //得到空间三维点在图像上新的投影点的坐标，计算投影坐标和提取的角点坐标之间的偏差，偏差越小，标定结果越好。
    cout << "开始评价标定结果………………";
    double total_err = 0.0;            // 所有图像的平均误差的总和
    double err = 0.0;                  // 每幅图像的平均误差
    double totalErr = 0.0;
    double totalPoints = 0.0;
    vector<Point2f> image_points_pro;     // 保存重新计算得到的投影点

    for (int i = 0; i < image_count; i++)
    {

        projectPoints(object_points_seq[i], rvecsMat[i], tvecsMat[i], cameraMatrix, distCoeffs, image_points_pro);   //通过得到的摄像机内外参数，对角点的空间三维坐标进行重新投影计算

        err = norm(Mat(image_points_seq[i]), Mat(image_points_pro), NORM_L2);

        totalErr += err * err;
        totalPoints += object_points_seq[i].size();

        err /= object_points_seq[i].size();
        fout << "第" << i + 1 << "幅图像的平均误差：" << err << "像素" << endl;
        cout << "第" << i + 1 << "幅图像的平均误差：" << err << "像素" << endl;
        total_err += err;
    }
    fout << "每个点平均误差：" << sqrt(totalErr / totalPoints) << "像素" << endl << endl;//结果同重投影误差
    fout << "每张图片平均误差：" << total_err / image_count << "像素" << endl << endl;
    cout << "每个点平均误差：" << sqrt(totalErr / totalPoints) << "像素" << endl << endl;
    cout << "每张图片平均误差：" << total_err / image_count << "像素" << endl << endl;

    /***6.保存定标结果***/
    cout << "开始保存定标结果………………" << endl;
    Mat rotation_matrix = Mat(3, 3, CV_32FC1, Scalar::all(0)); /* 保存每幅图像的旋转矩阵 */
    fout << "相机内参数矩阵：" << endl;
    fout << cameraMatrix << endl << endl;
    fout << "畸变系数：\n";
    fout << distCoeffs << endl << endl << endl;
    for (int i = 0; i < image_count; i++)
    {
        /* 将旋转向量转换为相对应的旋转矩阵 */
        Rodrigues(rvecsMat[i], rotation_matrix);
        //将结果保存到txt文件
        fout << "第" << i + 1 << "幅图像的旋转向量：" << endl;
        fout << rvecsMat[i] << endl;
        fout << "第" << i + 1 << "幅图像的旋转矩阵：" << endl;
        fout << rotation_matrix << endl;
        fout << "第" << i + 1 << "幅图像的平移向量：" << endl;
        fout << tvecsMat[i] << endl << endl;
        //显示计算结果
        cout << "第" << i + 1 << "幅图像的旋转向量：" << endl;
        cout << rvecsMat[i] << endl;
        cout << "第" << i + 1 << "幅图像的旋转矩阵：" << endl;
        cout << rotation_matrix << endl;
        cout << "第" << i + 1 << "幅图像的平移向量：" << endl;
        cout << tvecsMat[i] << endl << endl;
    }
    cout << "定标结果完成保存！！！" << endl;
    fout << endl;
}


int main()
{

    //若为对称的圆形标定板则只需要改变下面路径即可
    string File_Directory1 = "D:\\desk\\yolov5-face-master\\save_images\\circle";   //相机标定图像所在文件夹
    string FileType = ".jpg";    // 需要查找的文件类型

    vector<string>FilesName1;    //存放文件名的容器

    getFilesName(File_Directory1, FileType, FilesName1);   // 相机标定所用图像文件的路径

    //标定板尺寸
    //1---27*27  10mm
    //Size board_size = Size(27,27);//标定板的规格7*7圆形标定板
    //Size square_size(10, 10);
    //2---7*7  10mm
    Size board_size = Size(7, 7);//标定板的规格7*7 圆形标定板
    Size square_size(20, 20);

    Mat cameraMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0));
    Mat distCoeffs = Mat(1, 5, CV_32FC1, Scalar::all(0));
    vector<Mat> rvecsMat;
    vector<Mat> tvecsMat;

    m_calibration(FilesName1, board_size, square_size, cameraMatrix, distCoeffs, rvecsMat, tvecsMat);
    waitKey(0);

    return 0;
}
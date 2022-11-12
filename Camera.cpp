
// circleTargetCameraCalibration.cpp : �������̨Ӧ�ó������ڵ㡣
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

    _finddata_t c_file;   // ����ļ����Ľṹ��

    long long hFile;
    hFile = _findfirst(buffer.c_str(), &c_file);   //�ҵ�һ���ļ���

    if (hFile == -1L)   // ����ļ���Ŀ¼�´�����Ҫ���ҵ��ļ�
        cout << "No " << FileType << "files in current directory!" << endl;
    else
    {
        string fullFilePath;
        do
        {
            fullFilePath.clear();

            //����
            fullFilePath = File_Directory + "\\" + c_file.name;

            FilesName.push_back(fullFilePath);

        } while (_findnext(hFile, &c_file) == 0);  //����ҵ��¸��ļ������ֳɹ��Ļ��ͷ���0,���򷵻�-1
        _findclose(hFile);
    }
}

void m_calibration(vector<string>& FilesName, Size board_size, Size square_size, Mat& cameraMatrix, Mat& distCoeffs, vector<Mat>& rvecsMat, vector<Mat>& tvecsMat)
{
    /***0.������������ò���***/
    ofstream fout("circle_caliberation_result.txt");                       // ����궨������ļ�
    int image_count = 0;                                            // ͼ������
    Size image_size;                                                // ͼ��ĳߴ�
    vector<Point2f> image_points;                                   // ����ÿ��ͼ���ϼ�⵽�Ľǵ�
    vector<vector<Point2f>> image_points_seq;                       // �����⵽�����нǵ�
    //����SimpleBlobDetector�ļ���������ðߵ��⣨SimpleBlobDetector���Ĳ���
    SimpleBlobDetector::Params params;
    //params.maxArea = 10e4;		//�ߵ��������
    params.minThreshold = 30;		//***��ֵ������ʼ��ֵ������ʽ1��T1//��ֵԽ��ƫ�ڵĲ��ֱ����ԣ���ֵԽ���ȶ�Խ�ҲӰ������ٶ�
    params.maxThreshold = 200;		//����ֵ��ֵ
    params.thresholdStep = 10;		//��ֵԽС���Խ��ȷ�������ٶȱ���
    params.filterByInertia = true;	//�ߵ�Բ�ȵ����Ʊ�����Ĭ���ǲ�����
    params.filterByColor = true;	//�ߵ���ɫ�����Ʊ���
    params.blobColor = 0;			//255��ʾֻ��ȡ��ɫ�ߵ㣬0��ʾֻ��ȡ��ɫ�ߵ�
    params.filterByArea = true;		//�ߵ���������Ʊ���
    params.minArea = 60;			//�ߵ����С������ȡ��120
    //��С�İߵ���룬��ͬ��ֵͼ��İߵ�����С�ڸ�ֵʱ������Ϊ��ͬһ��λ�õİߵ㣬�����ǲ�ͬλ���ϵİߵ�
    params.minDistBetweenBlobs = 15;//���22����,15�ȽϺ���
    Ptr<FeatureDetector> blobDetector = SimpleBlobDetector::create(params);
    //Ptr<SimpleBlobDetector> blobDetector = SimpleBlobDetector::create(params);//Ҳ��������д��

    /***1.��ȡ����ͼƬ�е�Բ�ĵ㣨�ǵ㣩***/
    cout << "��ʼ��ȡ�ǵ㡭����������" << endl;
    for (int i = 0; i < FilesName.size(); i++)
    {
        image_count++;
        // ���ڹ۲�������
        cout << "image_count = " << image_count << endl;
        Mat imageInput = imread(FilesName[i]);
        imshow("qwewqe", imageInput);
        if (image_count == 1)  //�����һ��ͼƬʱ��ȡͼ������Ϣ
        {
            image_size.width = imageInput.cols;
            image_size.height = imageInput.rows;
            cout << "image_size.width = " << image_size.width << endl;
            cout << "image_size.height = " << image_size.height << endl;
        }

        //��Ҫ�������԰ߵ���Ĳ������ߵ�����Բ�����Բ��
        //std::vector<cv::KeyPoint> keyPoints1;
        //cv::Mat displayImg = imageInput.clone();
        //blobDetector->detect(displayImg, keyPoints1);
        //cv::drawKeypoints(displayImg, keyPoints1, displayImg,
        //	cv::Scalar(0, 0, 255), cv::DrawMatchesFlags::DEFAULT);
        //
        //��С��ʾ
        //cv::Mat dst = Mat::zeros(512, 640, CV_8UC3); //��Ҫת��Ϊ512*512��С��
        //resize(displayImg, dst, dst.size());
        //cv::imshow("�ߵ���result", dst);
        //cv::waitKey(200);


        /*** 2.��ȡ�ǵ� ***/
        /***********Բ�α궨��***********/
        //param1:�궨ͼƬ param2:�궨��ĳߴ� param3:�����⵽��Բ������
        //param4:��־λ���Գ�ͼ����cv::CALIB_CB_SYMMETRIC_GRID,�ǶԳ�ͼ����cv::CALIB_CB_ASYMMETRIC_GRID
        //param5:����㷨 �ߵ���

        //����Բ�α궨��
        bool detectorFlag = findCirclesGrid(imageInput, board_size, image_points, CALIB_CB_SYMMETRIC_GRID, blobDetector);

        //���ڷ���
        //bool detectorFlag = findChessboardCorners(imageInput, board_size, image_points, CV_CALIB_CB_ADAPTIVE_THRESH);//�������̸���
        if (detectorFlag == false)
        {	//���ʧ�ܣ���ʾ���ʧ�ܵĵ�
            cout << "imgae_points size = " << image_points.size() << std::endl;
            cout << "��" << image_count << "����Ƭ��ȡԲ��ʧ�ܣ���ɾ�������±궨��" << endl; //�Ҳ����ǵ�
            //drawChessboardCorners(imageInput, board_size, Mat(image_points), true);//��ԭͼ�л����ҵ���Բ�ĵ�
            for (size_t i = 0; i < image_points.size(); i++) {
                cv::circle(imageInput, image_points[i], 5, cv::Scalar(0, 0, 255), 3);
            }
            imshow("���ʧ����Ƭ", imageInput);
            waitKey(200);
        }
        else {
            cout << "!!!!!!!!!!!!!!!!" << endl;
            cout << "!!!!!!!!!!!!!!!!" << endl;
            cout << "!!!!!!!!!!!!!!!!" << endl;
            cout << "!!!!!!!!!!!!!!!!" << endl;
            cout << "!!!!!!!!!!!!!!!!" << endl;
        }
        //���ɹ�����ʾ��⵽�ĵ�
        drawChessboardCorners(imageInput, board_size, Mat(image_points), true);//��ԭͼ�л����ҵ���Բ�ĵ�
        cv::Mat dst2 = Mat::zeros(512, 640, CV_8UC3); //��Ҫת��Ϊ512*512��С��
        resize(imageInput, dst2, dst2.size());
        imshow("������", dst2);
        waitKey(200);
        image_points_seq.push_back(image_points);
        //else		//�˲����������̸���
        //{
        //	Mat view_gray;
        //	cout << "imageInput.channels()=" << imageInput.channels() << endl;
        //	cvtColor(imageInput, view_gray, CV_RGB2GRAY);

        //	/* �����ؾ�ȷ�� */
        //	//find4QuadCornerSubpix(view_gray, image_points, Size(5, 5)); //�Դ���ȡ�Ľǵ���о�ȷ��
        //	cv::cornerSubPix(view_gray, image_points, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 20, 0.01));

        //	image_points_seq.push_back(image_points);  //���������ؽǵ�

        //											   /* ��ͼ������ʾ�ǵ�λ�� */
        //	drawChessboardCorners(view_gray, board_size, image_points, true);

        //	//imshow("Camera Calibration", view_gray);//��ʾͼƬ
        //	//waitKey(100);//��ͣ0.1S
        //}

    }
    cout << "�ǵ���ȡ��ɣ�����" << endl;
    cv::destroyAllWindows();//�ص����е���ʾ����

    cout << "��ȡ�������" << endl;
    for (size_t i = 0; i < image_points_seq[0].size(); i++)
    {
        cout << image_points_seq[0][i] << endl;
    }

    /***3.���ɱ궨����ά������Ϣ***/
    vector<vector<Point3f>> object_points_seq;      // ��ά������������������һ���������ʽ����߱궨������ʽҪ��

    //�ǶԳƱ궨��(Բ��)
    //for (int t = 0; t < image_count; t++)
    //{
    //	vector<Point3f> object_points;
    //	for (int i = 0; i < board_size.height; i++)//����7
    //	{
    //		for (int j = 0; j < board_size.width; j++)//����7
    //		{
    //			Point3f realPoint;
    //			/* ����궨�������������ϵ��z=0��ƽ���� */
    //			if (i % 2 == 0) {//Ϊż����ʱ
    //				realPoint.x = i * square_size.width;
    //				realPoint.y = (2 * j)*square_size.height;
    //				realPoint.z = 0;
    //			}
    //			else {//Ϊ������
    //				realPoint.x = i * square_size.width;
    //				realPoint.y = (2 * j + 1)*square_size.height;
    //				realPoint.z = 0;
    //			}
    //			object_points.push_back(realPoint);
    //		}
    //	}
    //	object_points_seq.push_back(object_points);
    //}

    //�ԳƱ궨�壨Բ�Σ�x�����£�y������,z������
    for (int t = 0; t < image_count; t++)
    {
        vector<Point3f> object_points;
        for (int i = 0; i < board_size.height; i++)//����7
        {
            for (int j = 0; j < board_size.width; j++)//����7
            {
                Point3f realPoint;
                /* ����궨�������������ϵ��z=0��ƽ���� */
                realPoint.x = i * square_size.width;
                realPoint.y = j * square_size.height;
                realPoint.z = 0;
                object_points.push_back(realPoint);
            }
        }
        object_points_seq.push_back(object_points);
    }

    cout << "�궨���3D����" << endl;
    for (int i = 0; i < object_points_seq[0].size(); i++)
    {
        cout << object_points_seq[0][i] << endl;
    }

    /*** 4.���б궨���� ***/
    cout << "�궨����ʱ��Ͼ�" << endl;
    double err_first = calibrateCamera(object_points_seq, image_points_seq, image_size, cameraMatrix, distCoeffs, rvecsMat, tvecsMat, CALIB_FIX_K3);
    fout << "�궨��ͶӰ��" << err_first << "����" << endl << endl;
    cout << "�궨��ɣ�����" << endl;


    /***5.�Ա궨�����������***/
    //�Ա궨����������۵ķ�����ͨ���õ����������������Կռ����ά���������ͶӰ���㣬
    //�õ��ռ���ά����ͼ�����µ�ͶӰ������꣬����ͶӰ�������ȡ�Ľǵ�����֮���ƫ�ƫ��ԽС���궨���Խ�á�
    cout << "��ʼ���۱궨���������������";
    double total_err = 0.0;            // ����ͼ���ƽ�������ܺ�
    double err = 0.0;                  // ÿ��ͼ���ƽ�����
    double totalErr = 0.0;
    double totalPoints = 0.0;
    vector<Point2f> image_points_pro;     // �������¼���õ���ͶӰ��

    for (int i = 0; i < image_count; i++)
    {

        projectPoints(object_points_seq[i], rvecsMat[i], tvecsMat[i], cameraMatrix, distCoeffs, image_points_pro);   //ͨ���õ������������������Խǵ�Ŀռ���ά�����������ͶӰ����

        err = norm(Mat(image_points_seq[i]), Mat(image_points_pro), NORM_L2);

        totalErr += err * err;
        totalPoints += object_points_seq[i].size();

        err /= object_points_seq[i].size();
        fout << "��" << i + 1 << "��ͼ���ƽ����" << err << "����" << endl;
        cout << "��" << i + 1 << "��ͼ���ƽ����" << err << "����" << endl;
        total_err += err;
    }
    fout << "ÿ����ƽ����" << sqrt(totalErr / totalPoints) << "����" << endl << endl;//���ͬ��ͶӰ���
    fout << "ÿ��ͼƬƽ����" << total_err / image_count << "����" << endl << endl;
    cout << "ÿ����ƽ����" << sqrt(totalErr / totalPoints) << "����" << endl << endl;
    cout << "ÿ��ͼƬƽ����" << total_err / image_count << "����" << endl << endl;

    /***6.���涨����***/
    cout << "��ʼ���涨����������������" << endl;
    Mat rotation_matrix = Mat(3, 3, CV_32FC1, Scalar::all(0)); /* ����ÿ��ͼ�����ת���� */
    fout << "����ڲ�������" << endl;
    fout << cameraMatrix << endl << endl;
    fout << "����ϵ����\n";
    fout << distCoeffs << endl << endl << endl;
    for (int i = 0; i < image_count; i++)
    {
        /* ����ת����ת��Ϊ���Ӧ����ת���� */
        Rodrigues(rvecsMat[i], rotation_matrix);
        //��������浽txt�ļ�
        fout << "��" << i + 1 << "��ͼ�����ת������" << endl;
        fout << rvecsMat[i] << endl;
        fout << "��" << i + 1 << "��ͼ�����ת����" << endl;
        fout << rotation_matrix << endl;
        fout << "��" << i + 1 << "��ͼ���ƽ��������" << endl;
        fout << tvecsMat[i] << endl << endl;
        //��ʾ������
        cout << "��" << i + 1 << "��ͼ�����ת������" << endl;
        cout << rvecsMat[i] << endl;
        cout << "��" << i + 1 << "��ͼ�����ת����" << endl;
        cout << rotation_matrix << endl;
        cout << "��" << i + 1 << "��ͼ���ƽ��������" << endl;
        cout << tvecsMat[i] << endl << endl;
    }
    cout << "��������ɱ��棡����" << endl;
    fout << endl;
}


int main()
{

    //��Ϊ�ԳƵ�Բ�α궨����ֻ��Ҫ�ı�����·������
    string File_Directory1 = "D:\\desk\\yolov5-face-master\\save_images\\circle";   //����궨ͼ�������ļ���
    string FileType = ".jpg";    // ��Ҫ���ҵ��ļ�����

    vector<string>FilesName1;    //����ļ���������

    getFilesName(File_Directory1, FileType, FilesName1);   // ����궨����ͼ���ļ���·��

    //�궨��ߴ�
    //1---27*27  10mm
    //Size board_size = Size(27,27);//�궨��Ĺ��7*7Բ�α궨��
    //Size square_size(10, 10);
    //2---7*7  10mm
    Size board_size = Size(7, 7);//�궨��Ĺ��7*7 Բ�α궨��
    Size square_size(20, 20);

    Mat cameraMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0));
    Mat distCoeffs = Mat(1, 5, CV_32FC1, Scalar::all(0));
    vector<Mat> rvecsMat;
    vector<Mat> tvecsMat;

    m_calibration(FilesName1, board_size, square_size, cameraMatrix, distCoeffs, rvecsMat, tvecsMat);
    waitKey(0);

    return 0;
}
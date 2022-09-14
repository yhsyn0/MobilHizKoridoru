#include <opencv2/video/video.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <string>
#include <ultimateALPR-SDK-API-PUBLIC.h>
#include "../alpr_utils.h"
#include <chrono>
#include <iostream>
#include <ctime>
#include <fstream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

using namespace ultimateAlprSdk;
using namespace std;
using namespace cv;

map<string, int> plateCounter;
vector<string> split(string s);
vector<string> getDatas(string str);
void printDatas(string str);
void drawRect(Mat image, vector<string> datas, float fps);
void countPlate(string plate, map<string, int> &plateCounter);
int kbhit(void);

string datas;
class MyUltAlprSdkParallelDeliveryCallback : public UltAlprSdkParallelDeliveryCallback {
public:
	MyUltAlprSdkParallelDeliveryCallback(const string& charset) : m_strCharset(charset) {}
	virtual void onNewResult(const UltAlprSdkResult* result) const override {
		ULTALPR_SDK_ASSERT(result != nullptr);
		const string& json = result->json();
		//datas = getDatas(result->json());
        datas = result->json();
	}
private:
	string m_strCharset;
};

static const char* __jsonConfig =
"{"
	"\"debug_level\": \"info\","
	"\"debug_write_input_image_enabled\": false,"
	"\"debug_internal_data_path\": \".\","
    "\"assets_folder\": \"../../../assets\","
	""
	"\"num_threads\": -1,"
	"\"gpgpu_enabled\": true,"
	"\"openvino_enabled\": true,"
	"\"openvino_device\": \"CPU\","
	""
	"\"detect_roi\": [0, 0, 0, 0],"
	"\"detect_minscore\": 0.1,"
	""
	"\"pyramidal_search_enabled\": false,"
    //"\"pyramidal_search_sensitivity\": 0.28,"
	//"\"pyramidal_search_minscore\": 0.6,"
	//"\"pyramidal_search_min_image_size_inpixels\": 800,"
	""
	"\"recogn_minscore\": 0.75,"
	"\"recogn_score_type\": \"min\""
	"}";

    #if ULTALPR_SDK_OS_ANDROID 
    #	define ASSET_MGR_PARAM() __sdk_android_assetmgr, 
    #else
    #	define ASSET_MGR_PARAM() 
    #endif
/*
	"\"pyramidal_search_sensitivity\": 0.28,"
	"\"pyramidal_search_minscore\": 0.6,"
	"\"pyramidal_search_min_image_size_inpixels\": 800,"
*/

int main()
{
    string pipeline = "nvarguscamerasrc ! video/x-raw(memory:NVMM), width=(int)1920, height=(int)1080, framerate=(fraction)30/1 ! nvvidconv flip-method=0 ! video/x-raw, width=(int)1280, height=(int)720, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";
    UltAlprSdkResult result;
    //VideoCapture cap("/home/nano/Downloads/video5.mp4");
    //VideoCapture cap("nvarguscamerasrc ! video/x-raw(memory:NVMM), width=(int)1920, height=(int)1080,format=(string)NV12, framerate=(fraction)30/1 ! nvvidconv ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink");
	VideoCapture cap(pipeline, cv::CAP_GSTREAMER);
    
    if (!cap.isOpened())
    {
        cout << "Cannot open the video file. \n";
        return -1;
    }
    
    float fps = cap.get(CAP_PROP_FPS);
	//namedWindow("Recognizer",WINDOW_AUTOSIZE);
    //VideoWriter out("home/nano/Desktop/out.avi", VideoWriter::fourcc('M','J','P','G'), fps/3, Size(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT)));
    //cout << fps << endl;

    bool isParallelDeliveryEnabled = true;
    MyUltAlprSdkParallelDeliveryCallback parallelDeliveryCallbackCallback("latin");
	result = UltAlprSdkEngine::init(
        ASSET_MGR_PARAM()
        __jsonConfig,
        isParallelDeliveryEnabled ? &parallelDeliveryCallbackCallback : nullptr);

    UltAlprSdkEngine::warmUp(ULTALPR_SDK_IMAGE_TYPE_BGR24);
	Mat frame;
    cout << "STARTING" << endl;
    auto t1 = chrono::high_resolution_clock::now();
    //while(true)
    while(!kbhit())
    {
		if (!cap.read(frame)) 
		{
			std::cout<<"Capture read error"<<std::endl;
			break;
		}

        result = UltAlprSdkEngine::process(
            ULTALPR_SDK_IMAGE_TYPE_BGR24,
            frame.ptr(),
			cap.get(CAP_PROP_FRAME_WIDTH),
			cap.get(CAP_PROP_FRAME_HEIGHT)
            );
        
        //drawRect(frame, getDatas(datas), fps);
        getDatas(datas);
        //printDatas(datas);
        //out.write(frame);
		//imshow("Recognizer", frame);
        //if(waitKey(10) == 27)
        //   break;
    }
    auto t2 = chrono::high_resolution_clock::now();

    cap.release();
    //out.release();
    destroyAllWindows();

	result = UltAlprSdkEngine::deInit();

    auto ms_int = chrono::duration_cast<chrono::milliseconds>(t2 - t1);
	cout << ms_int.count() << " ms\n";

    map<string, int>::iterator itr;
    cout << "\tKEY\tELEMENT\n";
    for (itr = plateCounter.begin(); itr != plateCounter.end(); ++itr) {
        cout << '\t' << itr->first << '\t' << itr->second
             << '\n';
    }
    cout << endl;

    return 0;
}

vector<string> split(string s)
{
    string delimiter = ",";
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != string::npos) 
    {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

vector<string> getDatas(string str)
{
    int confIndex = 0;
    vector<string> vec;

    while(true)
    {
        confIndex = str.find("\"confidence\"", confIndex+1);
        if(confIndex == -1)
            break;
        
        string sub = string(str.begin()+str.find("warpedBox", confIndex)+strlen("warpedBox")+3, 
                        str.begin()+str.find("]", confIndex));
        vector<string> v = split(sub);
        vec.insert(vec.end(), {v[0], v[1], v[4], v[5]});

        sub = string(str.begin()+str.find("text", confIndex)+strlen("text")+3,
                        str.begin()+str.find("*", confIndex)+1);
        if(sub.length() >= 5)
        {
            cout << sub << endl;
            vec.push_back(sub);
            countPlate(sub, plateCounter);
        }
        sub = string(str.begin()+str.find("warpedBox", str.find("*", confIndex))+strlen("warpedBox")+3,
                        str.begin()+str.find("]", str.find("warpedBox", str.find("*", confIndex)))+strlen("warpedBox")+4);
        v = split(sub);
        vec.insert(vec.end(), {v[0], v[1], v[4], v[5]});
    }
	//for (auto i : vec) cout << i << endl;
    return vec;
}

void printDatas(string str)
{
    if(str.find("text") != -1)
        cout << string(str.begin()+str.find("text")+strlen("text")+3,
                        str.begin()+str.find("*")+1) << endl;
}

void drawRect(Mat image, vector<string> datas, float fps)
{
    if(datas.size() >= 9)
    {
        for(unsigned i = 0; i < datas.size(); i += 9)
        {
            rectangle(image, Point(stof(datas[i]), stof(datas[i+1])), Point(stof(datas[i+2]), stof(datas[i+3])), Scalar(0,0,255), 2, LINE_8);
            Size sizes = getTextSize(datas[i+4], FONT_HERSHEY_COMPLEX_SMALL, 0.625, 1, 0);
            rectangle(image, Point(stof(datas[i+5]), stof(datas[i+6])), Point(stof(datas[i+7]), stof(datas[i+8])), Scalar(0,194,255), 2, LINE_8);
            rectangle(image, Point(stof(datas[i+5]), stof(datas[i+6])-sizes.height), Point(stof(datas[i+5])+sizes.width, stof(datas[i+6])), Scalar(0,194,255), -1, LINE_8);
            putText(image, datas[i+4], Point(stof(datas[i+5]), stof(datas[i+6])), FONT_HERSHEY_COMPLEX_SMALL, 0.625, Scalar(0,0,0), 1, LINE_8);
            //countPlate(datas[i+4], plateCounter);
            
        }
        //putText(image, to_string(fps), Point(20,50), FONT_HERSHEY_SIMPLEX, 2, Scalar(0,0,255), 2, LINE_8);
    }
}

void countPlate(string plate, map<string, int> &plateCounter)
{
    std::map<string, int>::iterator it = plateCounter.find(plate);
    if(it == plateCounter.end())
    {
        plateCounter.insert(make_pair(plate, 1));
    }
    else if(plate.length() >= 5)
    {
        it->second += 1;
        if(it->second == 25)
        {
            time_t curr_time;
            curr_time = time(NULL);
            tm *tm_local = localtime(&curr_time);

            string dateTime = to_string(tm_local->tm_year+1900) + "-" + to_string(tm_local->tm_mon + 1) + 
                "-" + to_string(tm_local->tm_mday) + " " + to_string(tm_local->tm_hour) + ":" + to_string(tm_local->tm_min) + 
                ":" + to_string(tm_local->tm_sec);

            string sqlQuery = "\"INSERT INTO plates_out VALUES ('" + plate + "', '" + dateTime + "', " + to_string(50) + ")\"";
            string command = "~/ultimateALPR-SDK/samples/c++/recognizer/passTerminal " + sqlQuery;

            if(system(command.c_str()) != 0)
            	cout << "SQL Sending Error !" << endl;
        }   
    }
}

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}

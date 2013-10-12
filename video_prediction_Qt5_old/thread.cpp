#include "thread.h"
#include "horrorvideoprediction.h"
#include <QtCore>

#include <iostream>

HorrThread::HorrThread(){

}

void HorrThread::initHorr(const QString& fileName){
    m_fileName = fileName;
    m_stop = false;
    horrEnd = false;
    horrResult = -1;
}

void HorrThread::run()
{
    while(!m_stop){
        if(!processHorr(horrResult)){
            std::cerr << "Sorry, failed to process this horror video. " << std::endl;
        }
        if(horrResult == 0 || horrResult == 1){
            m_stop = true;
        }else{
            std::cerr << "Why, this horror video. " << std::endl;
            m_stop = true;
        }
        emit passHorrValue(QString::number(horrResult));
        break;
    }
}

bool HorrThread::processHorr(int& result)
{

    const std::string file_name = m_fileName.toStdString();
    const std::string normalization_name = "normalization.xml";
    const std::string model_name = "horrorvideo.hvp";
    const std::string normalization_visual_name = "normalization_visual.xml";
    const std::string model_visual_name = "horrorvideo_visual.hvp";

    const unsigned int shot_interval(100);
    const unsigned int shot_num(20);

    unsigned int label_array_length(0);
    if(!video_scene_length(label_array_length, file_name.c_str(), shot_interval, shot_num)){
        std::cerr << "Sorry, failed to extract the length of video scenes form " << file_name << ". " << std::endl;
        return false;
    }

    int* label_array = new int[label_array_length];
    memset(label_array, 0, sizeof(int)*label_array_length);

    if(!hvp_total(label_array, file_name.c_str(), shot_interval, shot_num,
                  normalization_name.c_str(), model_name.c_str(), normalization_visual_name.c_str(), model_visual_name.c_str())){
        std::cerr << "Sorry, failed to predict the result! " << file_name << ". " << std::endl;
        return false;
     }

    result = label_array[0];

    delete[] label_array;
    label_array = nullptr;

    return true;
}



#if !defined HORRORVIDEOPREDICTION_H
#define HORRORVIDEOPREDICTION_H

#pragma comment(lib,"hvp_004.lib")

#include <iostream>
#include <vector>
#include <string>

extern "C" __declspec(dllimport) bool hvp_total(int label_array[], const char* c_file_name, 
												const unsigned int shot_interval, const unsigned int shot_num, 
												const char* c_normalization_name = "normalization.xml", 
												const char* c_model_name = "horrorvideo.hvp",
												const char* c_normalization_visual_name = "normalization_visual.xml", 
												const char* c_model_visual_name = "horrorvideo_visual.hvp");

extern "C" __declspec(dllimport) bool hvp_total_single(int& label, const char* c_file_name, const unsigned int shot_interval, 
													   const char* c_normalization_name = "normalization.xml", 
													   const char* c_model_name = "horrorvideo.hvp",
													   const char* c_normalization_visual_name = "normalization_visual.xml", 
													   const char* c_model_visual_name = "horrorvideo_visual.hvp");

extern "C" __declspec(dllimport) bool video_scene_length(unsigned int& video_scene_length, const char* c_file_name,
														 const unsigned int shot_interval, const unsigned int shot_num);

#endif

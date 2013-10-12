// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the NoPornVideo_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// NoPornVideo_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.


#ifdef NoPornVideo_EXPORTS
#define NoPornVideo_API __declspec(dllexport)
#else
#define NoPornVideo_API __declspec(dllimport)
#endif

#define NOPORNVIDEO_METHOD_FAST 0

#pragma comment(lib,"NoPornVideo.lib")

//// This class is exported from the NoPornVideo.dll
//class NoPornVideo_API CNoPornVideo {
//public:
//	CNoPornVideo(void);
//	// TODO: add your methods here.
//};
//
//extern NoPornVideo_API int nNoPornVideo;
//
//NoPornVideo_API int fnNoPornVideo(void);

extern "C" NoPornVideo_API void *CreatePornVideoDetector(void);
extern "C" NoPornVideo_API float RunPornVideoDetector(void *pPornDetector, const char *strFileName,int method);
extern "C" NoPornVideo_API void DestroyPornVideoDetector(void *pPornDetector);



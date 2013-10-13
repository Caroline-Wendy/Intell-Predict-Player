#ifdef NoPornVideo_EXPORTS
#define NoPornVideo_API __declspec(dllexport)
#else
#define NoPornVideo_API __declspec(dllimport)
#endif

#define NOPORNVIDEO_METHOD_FAST 0

#pragma comment(lib,"NoPornVideo.lib")

extern "C" NoPornVideo_API void *CreatePornVideoDetector(void);
extern "C" NoPornVideo_API float RunPornVideoDetector(void *pPornDetector, const char *strFileName,int method);
extern "C" NoPornVideo_API void DestroyPornVideoDetector(void *pPornDetector);



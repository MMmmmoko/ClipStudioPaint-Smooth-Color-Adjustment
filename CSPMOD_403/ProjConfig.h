#pragma once



//windows‘ÿ»Îø‚Œƒº˛

//win api
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "Imm32.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Opengl32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Shell32.lib")

//nimdui
#ifdef _DEBUG
#pragma comment(lib, "cximage_d.lib")
#pragma comment(lib, "duilib_d.lib")
#pragma comment(lib, "libpng_d.lib")
#pragma comment(lib, "libwebp_d.lib")
#pragma comment(lib, "zlib_d.lib")
#else
#pragma comment(lib, "cximage_s.lib")
#pragma comment(lib, "duilib_s.lib")
#pragma comment(lib, "libpng_s.lib")
#pragma comment(lib, "libwebp_s.lib")
#pragma comment(lib, "zlib_s.lib")
#endif // _DEBUG


//SKIA
#pragma comment(lib, "skia.lib") 
#pragma comment(lib, "bentleyottmann.lib")
#pragma comment(lib, "expat.lib")
#pragma comment(lib, "jsonreader.lib")
#pragma comment(lib, "pathkit.lib")
#pragma comment(lib, "skcms.lib")
#pragma comment(lib, "skottie.lib")
#pragma comment(lib, "skresources.lib")
#pragma comment(lib, "skshaper.lib")
#pragma comment(lib, "svg.lib")
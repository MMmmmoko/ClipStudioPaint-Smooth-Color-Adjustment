// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef PCH_H
#define PCH_H

// 添加要在此处预编译的标头

#include<wrl/client.h>
#include "framework.h"


#pragma comment(lib, "d3d11.lib")       // 核心库:ml-citation{ref="3,5" data="citationList"}
#pragma comment(lib, "dxgi.lib")        // DXGI接口库:ml-citation{ref="3" data="citationList"}

#endif //PCH_H

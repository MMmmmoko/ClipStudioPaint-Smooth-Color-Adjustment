#include "duilib/RenderSkia/DrawSkiaImage.h"

#include "duilib/Utils/PerformanceUtil.h"

#pragma warning (push)
#pragma warning (disable: 4505)
#define STBIR_DEFAULT_FILTER_UPSAMPLE STBIR_FILTER_TRIANGLE
#include "duilib/third_party/stb_image/stb_image_resize2.h"
#pragma warning (pop)

#include "duilib/RenderSkia/SkiaHeaderBegin.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkData.h"
#include "duilib/RenderSkia/SkiaHeaderEnd.h"

#ifdef DUILIB_HAVE_OPENCV
#pragma warning (push)
#pragma warning (disable: 4127)
#include "opencv2/opencv.hpp"
#pragma warning (pop)
#endif


//此处修改是为了让skia图片渲染使用双线性插值，原作者使用默认参数是临近插值，对图像被缩放的情况会有严重的锯齿
//修改后在头文件"duilib/RenderSkia/DrawSkiaImage.h"中定义下方宏
#ifndef _DrawSkiaImage_DrawImage_patch
static_assert(false,"Need to Patch");
#endif // !_DrawSkiaImage_DrawImage_patch


namespace ui
{


    void DrawSkiaImage::DrawImage(SkCanvas* pSkCanvas,
        const UiRect& rcDest,
        const SkPoint& skPointOrg,
        const sk_sp<SkImage>& skSrcImage,
        const UiRect& rcSrc,
        const SkPaint& skPaint)
    {
        if ((pSkCanvas == nullptr) || (skSrcImage == nullptr)) {
            return;
        }
        SkIRect rcSkDestI = { rcDest.left, rcDest.top, rcDest.right, rcDest.bottom };
        SkRect rcSkDest = SkRect::Make(rcSkDestI);
        rcSkDest.offset(skPointOrg);

        SkIRect rcSkSrcI = { rcSrc.left, rcSrc.top, rcSrc.right, rcSrc.bottom };
        SkRect rcSkSrc = SkRect::Make(rcSkSrcI);

#ifdef DUILIB_HAVE_OPENCV
        sk_sp<SkImage> skNewImage;
        if (ResizeSkiaImageByOpenCV(skSrcImage, rcSrc, rcDest, skNewImage)) {
            PerformanceStat statPerformance(_T("Render_Skia::DrawSkiaImage::DrawImage drawImageRect(OpenCV)"));
            rcSkSrc.fRight = rcSkSrc.fLeft + skNewImage->width();
            rcSkSrc.fBottom = rcSkSrc.fTop + skNewImage->height();
            pSkCanvas->drawImageRect(skNewImage, rcSkSrc, rcSkDest, SkSamplingOptions(), &skPaint, SkCanvas::kStrict_SrcRectConstraint);
        }
#else
        sk_sp<SkImage> skNewImage;

        //这是修改部分
//这是修改部分
//这是修改部分
//这是修改部分
//这是修改部分
//         //源代码这里固定返回false，每次更新的时候还需要检查一下这里
        //if (ResizeSkiaImageByStbImage(skSrcImage, rcSrc, rcDest, skNewImage)) {
        if (false) {
            PerformanceStat statPerformance(_T("Render_Skia::DrawSkiaImage::DrawImage drawImageRect(StbImage)"));
            rcSkSrc.fRight = rcSkSrc.fLeft + skNewImage->width();
            rcSkSrc.fBottom = rcSkSrc.fTop + skNewImage->height();



            pSkCanvas->drawImageRect(skNewImage, rcSkSrc, rcSkDest, SkSamplingOptions(SkFilterMode::kLinear), &skPaint, SkCanvas::kStrict_SrcRectConstraint);

        }
#endif
        else {
            PerformanceStat statPerformance(_T("Render_Skia::DrawSkiaImage::DrawImage drawImageRect(Skia Only)"));



            pSkCanvas->drawImageRect(skSrcImage, rcSkSrc, rcSkDest, SkSamplingOptions(SkFilterMode::kLinear), &skPaint, SkCanvas::kStrict_SrcRectConstraint);
     //这是修改部分
     //这是修改部分
     //这是修改部分
     //这是修改部分
     //这是修改部分
        
        }
    }

} // namespace ui


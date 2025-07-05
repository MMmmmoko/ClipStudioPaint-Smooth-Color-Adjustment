
#include<iostream>
#include"GaussBlur.h"
#include"Graphic/GaussBlurContext.h"
#include"Graphic/D3D11Graphic.h"
#include<chrono>
#include "stb_image_write.h"
#include "TriglavPlugInSDK/TriglavPlugInSDK.h"

GaussBlur* GaussBlur::pthis = NULL;
void GaussBlur::Init(TriglavPlugInServer* _runPluginServer)
{
    if (NULL == _runPluginServer)return;

    runPluginServer = _runPluginServer;

    //创建gpu资源


    //获取源离屏对象
    TriglavPlugInRecordSuite* pRecordSuite = &(*runPluginServer).recordSuite;
    TriglavPlugInOffscreenService* pOffscreenService = (*runPluginServer).serviceSuite.offscreenService;
    //    pWorkingPropertyService    = (*pluginServer).serviceSuite.propertyService;
    TriglavPlugInBitmapService* pBitmapService = (*runPluginServer).serviceSuite.bitmapService;




    TriglavPlugInHostObject hostObject = (*runPluginServer).hostObject;






    if (TriglavPlugInGetFilterRunRecord(pRecordSuite) == NULL || pOffscreenService == NULL /*&& pWorkingPropertyService != NULL */)
        return;


    //获取到源离屏对象
    TriglavPlugInOffscreenObject sourceOffscreen, selectAreaOffscreen;
    TriglavPlugInFilterRunGetSourceOffscreen(pRecordSuite, &sourceOffscreen, hostObject);

    //先确认是否有选区
    TriglavPlugInFilterRunGetSelectAreaOffscreen(pRecordSuite, &selectAreaOffscreen, hostObject);
    TriglavPlugInRect targetRect;
    if (selectAreaOffscreen == NULL)
    {
        pOffscreenService->getRectProc(&targetRect, sourceOffscreen);
    }
    else
    {
        TriglavPlugInFilterRunGetSelectAreaRect(pRecordSuite, &targetRect, hostObject);
    }


    //根据选区创建矩形
    TriglavPlugInRect selectAreaRect;
    TriglavPlugInFilterRunGetSelectAreaRect(pRecordSuite, &selectAreaRect, hostObject);







    //构建图像
    //从原始图像获取位图

    TriglavPlugInBitmapObject sourceBitmap = NULL;
    pBitmapService->createProc(&sourceBitmap, selectAreaRect.right - selectAreaRect.left, selectAreaRect.bottom - selectAreaRect.top, 4, kTriglavPlugInBitmapScanlineHorizontalLeftTop);
    TriglavPlugInPoint pointZero;
    pointZero.x = 0;
    pointZero.y = 0;
    TriglavPlugInPoint bitmapLeftTop;
    bitmapLeftTop.x = selectAreaRect.left;
    bitmapLeftTop.y = selectAreaRect.top;

    pOffscreenService->getBitmapProc(sourceBitmap, &pointZero, sourceOffscreen, &bitmapLeftTop,
        selectAreaRect.right - selectAreaRect.left,
        selectAreaRect.bottom - selectAreaRect.top,
        kTriglavPlugInOffscreenCopyModeNormal
    );

    //创建纹理




    TriglavPlugInPtr bitmapData;
    pBitmapService->getAddressProc(&bitmapData, sourceBitmap, &pointZero);

    TriglavPlugInInt depth = 0;
    pBitmapService->getDepthProc(&depth, sourceBitmap);

    originTexture.Resize(selectAreaRect.right - selectAreaRect.left, selectAreaRect.bottom - selectAreaRect.top, bitmapData);




    //    GaussBlurContext::GetIns().GenerateMipmap(originTexture.GetTex());

    renderTextureStepHor.Resize(originTexture.GetSizeW(), originTexture.GetSizeH());
    renderTextureStepHorVer.Resize(originTexture.GetSizeW(), originTexture.GetSizeH());


    //选区
    if (selectAreaOffscreen != NULL)
    {

        TriglavPlugInBitmapObject selectAreaBitmap = NULL;
        pBitmapService->createProc(&selectAreaBitmap, selectAreaRect.right - selectAreaRect.left, selectAreaRect.bottom - selectAreaRect.top, 4, kTriglavPlugInBitmapScanlineHorizontalLeftTop);


        pOffscreenService->getBitmapProc(selectAreaBitmap, &pointZero, selectAreaOffscreen, &bitmapLeftTop,
            selectAreaRect.right - selectAreaRect.left,
            selectAreaRect.bottom - selectAreaRect.top,
            //                                         kTriglavPlugInOffscreenCopyModeAlpha
            kTriglavPlugInOffscreenCopyModeNormal
        );
        TriglavPlugInPtr selectAreaBitmapData;
        pBitmapService->getAddressProc(&selectAreaBitmapData, selectAreaBitmap, &pointZero);
        TriglavPlugInInt rowBytes;
        pBitmapService->getRowBytesProc(&rowBytes, selectAreaBitmap);
        TriglavPlugInInt depth, colorBytes;
        pBitmapService->getDepthProc(&depth, selectAreaBitmap);
        pBitmapService->getPixelBytesProc(&colorBytes, selectAreaBitmap);

        selectAreaTexture.Resize(originTexture.GetSizeW(), originTexture.GetSizeH(), selectAreaBitmapData, rowBytes);

        pBitmapService->releaseProc(selectAreaBitmap);
    }
    else
    {
        //        selectAreaTexture.Resize(4,1);//纯白
        //        uint32_t white=0xffffffff;
        //        selectAreaTexture.UpdateData(0, 0, 4, 1, &white);
                //位图模式下下方数据是1像素大小而不是4像素大小

        uint32_t white = 0xffffffff;
        selectAreaTexture.Resize(1, 1, &white);//纯白
    }


    //图像已经加载进gpu纹理了，释放原始纹理
    pBitmapService->releaseProc(sourceBitmap);


    //创建渲染目标位图
    runPluginServer->serviceSuite.bitmapService->createProc(&renderBitmap, originTexture.GetSizeW(), originTexture.GetSizeH(), 4, kTriglavPlugInBitmapScanlineHorizontalLeftTop);

    inited = true;
}


void GaussBlur::ShutDown()
{
    inited = false;
    if (runPluginServer && renderBitmap)
    {
        runPluginServer->serviceSuite.bitmapService->releaseProc(renderBitmap);
    }
    renderBitmap = NULL;
    runPluginServer = NULL;


}

void GaussBlur::OnParamChanged(double blurRadius)
{
    //检查预览

    //参数改变时，进行绘制
    if (!renderBitmap)
    {
        if (runPluginServer)
            runPluginServer->serviceSuite.bitmapService->createProc(&renderBitmap, originTexture.GetSizeW(), originTexture.GetSizeH(), 4, kTriglavPlugInBitmapScanlineHorizontalLeftTop);

        if (!renderBitmap)
            return;
    }


    auto start = std::chrono::system_clock::now();
    Render(blurRadius);
    auto end = std::chrono::system_clock::now();
    std::cout<<"cost time:"<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()<<std::endl;
}



void GaussBlur::SetSkip(bool bSkip1Frame)
{
    needSkip = bSkip1Frame;
}

void GaussBlur::PluginModuleCleanUp()
{

       D3D11Graphic::ShutDown();
       GaussBlurContext::ShutDown();

}


void GaussBlur::Render(float blurRadius)
{
    //    blurRadius=200;

    if (needSkip)
    {
        needSkip = false;
        return;
    }

    uint8_t* data = NULL;
    TriglavPlugInPoint zero;
    zero.x = 0;
    zero.y = 0;
    runPluginServer->serviceSuite.bitmapService->getAddressProc((void**)&data, renderBitmap, &zero);
    if (!data)
        return;

    TriglavPlugInInt rowBytesCount;
    runPluginServer->serviceSuite.bitmapService->getRowBytesProc(&rowBytesCount, renderBitmap);


    GaussBlurContext::GetIns().BindAndDraw
    (
        &renderTextureStepHor,
        &renderTextureStepHorVer,
        &originTexture,
        &selectAreaTexture,
        blurRadius
    );


    //通过写文件的形式进行调试
//    {
//        uint8_t* pImage=new uint8_t[originTexture.GetSizeW()*4*originTexture.GetSizeH()];
//        //    memset(pImage, 0xffffffff, blockCount*256*256*4);
//        MTL::Region region=MTL::Region::Make2D(0, 0, originTexture.GetSizeW(),  originTexture.GetSizeH());
//        renderTextureStepHor.GetTex()->getBytes(pImage, originTexture.GetSizeW()*4, region, 0);
//        stbi_write_png("/Users/nijiang/Desktop/testimgHor.png",originTexture.GetSizeW(), originTexture.GetSizeH(), 4, pImage, originTexture.GetSizeW()*4);
//        
//    }
//    {
//        uint8_t* pImage=new uint8_t[originTexture.GetSizeW()*4*originTexture.GetSizeH()];
//        //    memset(pImage, 0xffffffff, blockCount*256*256*4);
//        MTL::Region region=MTL::Region::Make2D(0, 0, originTexture.GetSizeW(),  originTexture.GetSizeH());
//        renderTextureStepHorVer.GetTex()->getBytes(pImage, originTexture.GetSizeW()*4, region, 0);
//        stbi_write_png("/Users/nijiang/Desktop/testimgHorVer.png",originTexture.GetSizeW(), originTexture.GetSizeH(), 4, pImage, originTexture.GetSizeW()*4);
//    }








    //gpu工作完后从renderTextureStepHorVer中获取结果
    //优化选区？

    renderTextureStepHorVer.CopyToMem(data, true, rowBytesCount);

    //应用位图
    //获取目标离屏纹理
    TriglavPlugInRecordSuite* pRecordSuite = &(*runPluginServer).recordSuite;
    TriglavPlugInOffscreenService* pOffscreenService = (*runPluginServer).serviceSuite.offscreenService;
    TriglavPlugInHostObject hostObject = (*runPluginServer).hostObject;
    TriglavPlugInOffscreenObject destOffscreen;
    TriglavPlugInFilterRunGetDestinationOffscreen(pRecordSuite, &destOffscreen, hostObject);
    //    TriglavPlugInOffscreenObject sourceOffscreen;
    //    TriglavPlugInFilterRunGetSourceOffscreen(pRecordSuite,&sourceOffscreen,hostObject);



    TriglavPlugInRect selectAreaRect;
    TriglavPlugInFilterRunGetSelectAreaRect(pRecordSuite, &selectAreaRect, hostObject);

    //这个好像是画布坐标而不是离屏坐标
    TriglavPlugInPoint targetPos;
    //    targetPos.x=-512;
    //    targetPos.y=-512;
    targetPos.x = selectAreaRect.left;
    targetPos.y = selectAreaRect.top;

    runPluginServer->serviceSuite.offscreenService->setBitmapProc
    (destOffscreen, &targetPos, renderBitmap, &zero, originTexture.GetSizeW(), originTexture.GetSizeH(), kTriglavPlugInOffscreenCopyModeNormal);


    //    std::cout<<"destOffscreen:"<<destOffscreen<<",SourceOffscreen:"<<sourceOffscreen<<",Right:"<<selectAreaRect.right<<",Bottom:"<<selectAreaRect.bottom<<std::endl;
    TriglavPlugInFilterRunUpdateDestinationOffscreenRect(pRecordSuite, hostObject, &selectAreaRect);





}


#include "TriglavPlugInSDK/TriglavPlugInSDK.h"
#include<assert.h>
#include <iostream>
#include <vector>
#include<thread>
#include"Graphic/D3D11Graphic.h"
#include"MotionBlur.h"
typedef	unsigned char	BYTE;

static    const int kItemPoint1 = 1;
static    const int kItemPoint2 = 2;
static    const int kItemPreview = 3;
static    const int kItemBlurDirection = 4;
static	const int kItemBlurAlgorithm = 5;

//���ػ��ַ���id
static	const int kStringIDFilterCategoryName = 101;//����ļ�������������ϼ��˵�����
static	const int kStringIDFilterName = 102;//�˾������
static    const int kStringIDPreview = 103;//Ԥ��
static    const int kStringIDBlurDirection = 104;//ģ������
static    const int kStringIDBlurAlgorithm = 105;//ģ���㷨

static    const int kStringIDBlurDirection_ForwardBehind = 106;//ģ������ ǰ��
static    const int kStringIDBlurDirection_Forward = 107;//ģ������ ǰ
static    const int kStringIDBlurDirection_Behind = 108;//ģ������ ��

static    const int kStringIDBlurAlgorithm_Smooth = 109;//ģ���㷨 ����
static	const int kStringIDBlurAlgorithm_Average = 110;//ģ���㷨 ��ֵ

static const char* uuidOfThisPlugin = "4034B383-D830-4D9B-9982-4DB2A006EBC4";




struct	HSVFilterInfo
{
    TriglavPlugInPoint  point1;
    TriglavPlugInPoint	point2;
    TriglavPlugInBool bPreview;
    TriglavPlugInPropertyService* pService1;
    TriglavPlugInPropertyService2* pPropertyService;
};



//	�ץ�ѥƥ����`��Хå�
//���Իص�
static void TRIGLAV_PLUGIN_CALLBACK TriglavPlugInFilterPropertyCallBack(TriglavPlugInInt* result, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInInt notify, TriglavPlugInPtr data)
{



    (*result) = kTriglavPlugInPropertyCallBackResultNoModify;


    HSVFilterInfo* pFilterInfo = static_cast<HSVFilterInfo*>(data);

    if (pFilterInfo != NULL)
    {
        TriglavPlugInPropertyService2* pPropertyService = (*pFilterInfo).pPropertyService;


        if (pPropertyService != NULL)
        {



            //��������޸���
            //����Ǹ�֪������csp�б��޸���
            if (notify == kTriglavPlugInPropertyCallBackNotifyValueChanged)
            {


                (*(*pFilterInfo).pPropertyService).getPointValueProc(&(*pFilterInfo).point1, propertyObject, kItemPoint1);
                (*(*pFilterInfo).pPropertyService).getPointValueProc(&(*pFilterInfo).point2, propertyObject, kItemPoint2);

                TriglavPlugInBool bPreview;
                (*(*pFilterInfo).pService1).getBooleanValueProc(&bPreview, propertyObject, kItemPreview);

                TriglavPlugInInt direction;
                TriglavPlugInInt algorithm;
                (*(*pFilterInfo).pPropertyService).getEnumerationValueProc((TriglavPlugInInt*)&direction, propertyObject, kItemBlurDirection);
                (*(*pFilterInfo).pPropertyService).getEnumerationValueProc((TriglavPlugInInt*)&algorithm, propertyObject, kItemBlurAlgorithm);

                MotionBlur::GetIns().OnParamChanged((*pFilterInfo).point1, (*pFilterInfo).point2, bPreview, (kBlurDirection)direction, (kBlurAlgorithm)algorithm);
                (*result) = kTriglavPlugInPropertyCallBackResultModify;


            }
        }
    }
}



//	�ץ饰����ᥤ��
// plugin main��
void TRIGLAV_PLUGIN_API TriglavPluginCall(TriglavPlugInInt* result, TriglavPlugInPtr* data, TriglavPlugInInt selector, TriglavPlugInServer* pluginServer, TriglavPlugInPtr reserved)
{
    *result = kTriglavPlugInCallResultFailed;
    if (pluginServer != NULL)
    {
        if (selector == kTriglavPlugInSelectorModuleInitialize)
        {



            //ģ���ʼ��
            //	�ץ饰����γ��ڻ�
            TriglavPlugInModuleInitializeRecord* pModuleInitializeRecord = (*pluginServer).recordSuite.moduleInitializeRecord;
            TriglavPlugInStringService* pStringService = (*pluginServer).serviceSuite.stringService;
            if (pModuleInitializeRecord != NULL && pStringService != NULL)
            {

                TriglavPlugInInt	hostVersion;
                (*pModuleInitializeRecord).getHostVersionProc(&hostVersion, (*pluginServer).hostObject);

                if (hostVersion >= kTriglavPlugInNeedHostVersion)
                {
                    //���csp�汾��֧��
                    TriglavPlugInStringObject	moduleID = NULL;
                    const char* moduleIDString = uuidOfThisPlugin;//��ֹ�ظ����ص�uuid

                    //���ú���csp�д���ģ��ʵ��������ģ��id
                    (*pStringService).createWithAsciiStringProc(&moduleID, moduleIDString, static_cast<TriglavPlugInInt>(::strlen(moduleIDString)));

                    (*pModuleInitializeRecord).setModuleIDProc((*pluginServer).hostObject, moduleID);
                    (*pModuleInitializeRecord).setModuleKindProc((*pluginServer).hostObject, kTriglavPlugInModuleSwitchKindFilter);//����ģ�����࣬�ÿ����ĵ�

                    //Ϊʲô������release
                    (*pStringService).releaseProc(moduleID);

                    HSVFilterInfo* pFilterInfo = new HSVFilterInfo;
                    *data = pFilterInfo;
                    *result = kTriglavPlugInCallResultSuccess;
                }
            }
        }
        else if (selector == kTriglavPlugInSelectorModuleTerminate)
        {
            //ģ����ֹʱ����Ϊ
            //	�ץ饰����νK�˄I��
            HSVFilterInfo* pFilterInfo = static_cast<HSVFilterInfo*>(*data);
            delete pFilterInfo;
            *data = NULL;
            *result = kTriglavPlugInCallResultSuccess;

            //�ƺ��ö�̬��ж��ʱ�Զ��ͷ�comָ����������������.
            MotionBlur::PluginModuleCleanUp();


        }
        else if (selector == kTriglavPlugInSelectorFilterInitialize)
        {

            //	�ե��륿�γ��ڻ�
            //�˾���ʼ��

            TriglavPlugInRecordSuite* pRecordSuite = &(*pluginServer).recordSuite;
            TriglavPlugInHostObject					hostObject = (*pluginServer).hostObject;
            TriglavPlugInStringService* pStringService = (*pluginServer).serviceSuite.stringService;
            TriglavPlugInPropertyService* pPropertyService = (*pluginServer).serviceSuite.propertyService;

            //�ӿڲ���Ϊ��
            if (TriglavPlugInGetFilterInitializeRecord(pRecordSuite) != NULL && pStringService != NULL && pPropertyService != NULL)
            {
                //	�ե��륿���ƥ������ȥե��륿�����O��
                //�˾��������˾���
                TriglavPlugInStringObject	filterCategoryName = NULL;
                TriglavPlugInStringObject	filterName = NULL;
                (*pStringService).createWithStringIDProc(&filterCategoryName, kStringIDFilterCategoryName, (*pluginServer).hostObject);
                (*pStringService).createWithStringIDProc(&filterName, kStringIDFilterName, (*pluginServer).hostObject);

                TriglavPlugInFilterInitializeSetFilterCategoryName(pRecordSuite, hostObject, filterCategoryName, 'c');
                TriglavPlugInFilterInitializeSetFilterName(pRecordSuite, hostObject, filterName, 'g');
                (*pStringService).releaseProc(filterCategoryName);
                (*pStringService).releaseProc(filterName);

                //	�ץ�ӥ�`
                //preview�� ɷ��Ƭ����
                //����������Ƿ����Ԥ��
                //����CSPԤ������Ч��������ԣ�̫������
                TriglavPlugInFilterInitializeSetCanPreview(pRecordSuite, hostObject, false);

                //	���`���å� Ŀ��
                //�������������úܶ�Ŀ�꣬����ֻ�����˲�ɫλͼ��rgba;
                //Դ���п��������õ�ֻ��rgba�ˡ��������������ûҶ�ͼ֮�������
                TriglavPlugInInt	target[] = { kTriglavPlugInFilterTargetKindRasterLayerRGBAlpha };
                TriglavPlugInFilterInitializeSetTargetKinds(pRecordSuite, hostObject, target, 1);


                //	�ץ�ѥƥ�������
                //���ɿ��Կ��Ƶ����� Property    ɷ��Ƭ����
                TriglavPlugInPropertyObject	propertyObject;
                (*pPropertyService).createProc(&propertyObject);



                //�˶�ģ����1


                TriglavPlugInPropertyService2* pService2 = (*pluginServer).serviceSuite.propertyService2;
                //    ��������
                TriglavPlugInStringObject    testCaption = NULL;
                (*pStringService).createWithStringIDProc(&testCaption, kStringIDPreview, (*pluginServer).hostObject);
                (*pPropertyService).addItemProc(propertyObject, kItemPoint1, kTriglavPlugInPropertyValueTypePoint, kTriglavPlugInPropertyInputKindCanvas, kTriglavPlugInPropertyInputKindCanvas, testCaption, 'a');
                //Ĭ��������м�
                (*pService2).setPointDefaultValueKindProc(propertyObject, kItemPoint1, kTriglavPlugInPropertyPointDefaultValueKindSelectAreaCenter);

                (*pPropertyService).addItemProc(propertyObject, kItemPoint2, kTriglavPlugInPropertyValueTypePoint, kTriglavPlugInPropertyInputKindCanvas, kTriglavPlugInPropertyInputKindCanvas, testCaption, 'b');
                (*pService2).setPointDefaultValueKindProc(propertyObject, kItemPoint2, kTriglavPlugInPropertyPointDefaultValueKindSelectAreaCenter);





                //enum
                //ģ������
                {
                    TriglavPlugInStringObject strBlurDirection = NULL;
                    TriglavPlugInStringObject strBlurDirectionFB = NULL;
                    TriglavPlugInStringObject strBlurDirectionF = NULL;
                    TriglavPlugInStringObject strBlurDirectionB = NULL;
                    (*pStringService).createWithStringIDProc(&strBlurDirection, kStringIDBlurDirection, (*pluginServer).hostObject);
                    (*pStringService).createWithStringIDProc(&strBlurDirectionFB, kStringIDBlurDirection_ForwardBehind, (*pluginServer).hostObject);
                    (*pStringService).createWithStringIDProc(&strBlurDirectionF, kStringIDBlurDirection_Forward, (*pluginServer).hostObject);
                    (*pStringService).createWithStringIDProc(&strBlurDirectionB, kStringIDBlurDirection_Behind, (*pluginServer).hostObject);

                    (*pPropertyService).addItemProc(propertyObject, kItemBlurDirection, kTriglavPlugInPropertyValueTypeEnumeration, kTriglavPlugInPropertyValueKindDefault, kTriglavPlugInPropertyInputKindDefault, strBlurDirection, 'd');
                    (*pService2).addEnumerationItemProc(propertyObject, kItemBlurDirection, kBlurDirectionFB, strBlurDirectionFB, 'e');
                    (*pService2).addEnumerationItemProc(propertyObject, kItemBlurDirection, kBlurDirectionF, strBlurDirectionF, 'f');
                    (*pService2).addEnumerationItemProc(propertyObject, kItemBlurDirection, kBlurDirectionB, strBlurDirectionB, 'g');
                    pService2->setEnumerationDefaultValueProc(propertyObject, kItemBlurDirection, kBlurDirectionFB);

                    (*pStringService).releaseProc(strBlurDirection);
                    (*pStringService).releaseProc(strBlurDirectionFB);
                    (*pStringService).releaseProc(strBlurDirectionF);
                    (*pStringService).releaseProc(strBlurDirectionB);

                }
                //enum
                //ģ���㷨
                {
                    TriglavPlugInStringObject strBlurAlgorithm = NULL;
                    TriglavPlugInStringObject strBlurAlgorithm_Smooth = NULL;
                    TriglavPlugInStringObject strBlurAlgorithm_Average = NULL;

                    (*pStringService).createWithStringIDProc(&strBlurAlgorithm, kStringIDBlurAlgorithm, (*pluginServer).hostObject);
                    (*pStringService).createWithStringIDProc(&strBlurAlgorithm_Smooth, kStringIDBlurAlgorithm_Smooth, (*pluginServer).hostObject);
                    (*pStringService).createWithStringIDProc(&strBlurAlgorithm_Average, kStringIDBlurAlgorithm_Average, (*pluginServer).hostObject);

                    (*pPropertyService).addItemProc(propertyObject, kItemBlurAlgorithm, kTriglavPlugInPropertyValueTypeEnumeration, kTriglavPlugInPropertyValueKindDefault, kTriglavPlugInPropertyInputKindDefault, strBlurAlgorithm, 's');
                    (*pService2).addEnumerationItemProc(propertyObject, kItemBlurAlgorithm, kBlurAlgorithmSmooth, strBlurAlgorithm_Smooth, 't');
                    (*pService2).addEnumerationItemProc(propertyObject, kItemBlurAlgorithm, kBlurAlgorithmAverage, strBlurAlgorithm_Average, 'u');
                    pService2->setEnumerationDefaultValueProc(propertyObject, kItemBlurAlgorithm, kBlurAlgorithmSmooth);

                    (*pStringService).releaseProc(strBlurAlgorithm);
                    (*pStringService).releaseProc(strBlurAlgorithm_Smooth);
                    (*pStringService).releaseProc(strBlurAlgorithm_Average);
                }






                //Ԥ��
                (*pPropertyService).addItemProc(propertyObject, kItemPreview, kTriglavPlugInPropertyValueTypeBoolean, kTriglavPlugInPropertyValueKindDefault, kTriglavPlugInPropertyInputKindDefault, testCaption, 'p');
                (*pPropertyService).setBooleanValueProc(propertyObject, kItemPreview, true);
                (*pPropertyService).setBooleanDefaultValueProc(propertyObject, kItemPreview, true);

                (*pStringService).releaseProc(testCaption);






                //                //Ԥ��
                //                blurRadiusCaption;
                //                (*pStringService).createWithStringIDProc(&blurRadiusCaption,kStringIDItemBlurRadius,(*pluginServer).hostObject);
                //                (*pPropertyService).addItemProc(propertyObject,kItemBlurRadius,kTriglavPlugInPropertyValueTypeBoolean,kTriglavPlugInPropertyValueKindDefault,kTriglavPlugInPropertyInputKindDefault,blurRadiusCaption,'p');
                //                (*pPropertyService).setb(propertyObject,kItemBlurRadius,0);
                //                (*pPropertyService).setDecimalDefaultValueProc(propertyObject,kItemBlurRadius,6);
                //                (*pPropertyService).setDecimalMinValueProc(propertyObject,kItemBlurRadius,0);
                //                (*pPropertyService).setDecimalMaxValueProc(propertyObject,kItemBlurRadius,256);
                //                (*pStringService).releaseProc(blurRadiusCaption);//�ֶ��ͷ��ַ�������





                                //	�ץ�ѥƥ����O��
                                //property������
                                //���һ����أ�kTriglavPlugInPropertyValueTypeInteger����
                TriglavPlugInFilterInitializeSetProperty(pRecordSuite, hostObject, propertyObject);
                TriglavPlugInFilterInitializeSetPropertyCallBack(pRecordSuite, hostObject, TriglavPlugInFilterPropertyCallBack, *data);

                //	�ץ�ѥƥ����Ɨ�
                //�ͷ�propertyobj Ӧ���Ǽ������ü����ɡ����ⶫ��Ӧ�ò�����ô��ʹ��ڴ����Ƴ�
                (*pPropertyService).releaseProc(propertyObject);

                *result = kTriglavPlugInCallResultSuccess;
            }
        }
        else if (selector == kTriglavPlugInSelectorFilterTerminate)
        {
            //	�ե��륿�νK�˄I��
            //�˾���ֹʱ�Ĵ���
            *result = kTriglavPlugInCallResultSuccess;
        }
        else if (selector == kTriglavPlugInSelectorFilterRun)
        {

            //	�ե��륿�Όg��
            //�˾���ִ��

            auto pWorkingRecordSuite = &(*pluginServer).recordSuite;
            auto pWorkingOffscreenService = (*pluginServer).serviceSuite.offscreenService;
            auto pWorkingPropertyService = (*pluginServer).serviceSuite.propertyService;
            auto workingHostObject = (*pluginServer).hostObject;


            auto& customFilter=MotionBlur::GetIns();

            //RUNRecord���壿
            if (TriglavPlugInGetFilterRunRecord(pWorkingRecordSuite) != NULL && pWorkingOffscreenService != NULL && pWorkingPropertyService != NULL)
            {
                customFilter.Init(pluginServer);




                //���ý���
                //����Ҫ���ý��ȣ�ai��ͼ�ǰ�


                //��ʼ��
                HSVFilterInfo* pFilterInfo = static_cast<HSVFilterInfo*>(*data);
                (*pFilterInfo).pService1 = (*pluginServer).serviceSuite.propertyService;
                (*pFilterInfo).pPropertyService = (*pluginServer).serviceSuite.propertyService2;
                pFilterInfo->bPreview = true;


                pFilterInfo->point1 = { 0,0 };
                pFilterInfo->point2 = { 0,0 };
                //                (*pFilterInfo).blurRadius			= 6;
                //                customFilter.Render(pFilterInfo->blurRadius);
                //                customFilter.Render(200);
                                //����restart���� ��λ��
                bool	restart = true;
                bool firstRun = true;

                std::cout << "loop entrypoint" << std::endl;
                while (true)
                {

                    //����������Ϣ���ɣ���������ڲ����ı�Ļص����Ƚ���������


                    //                    customFilter.SetSkip(false);
                    //                    customFilter.Render(pFilterInfo->point1.x-pFilterInfo->point2.x,pFilterInfo->point1.y-pFilterInfo->point2.y);
                    customFilter.SetSkip(true);
                    //�����״̬
                    TriglavPlugInInt	processResult;
                    TriglavPlugInFilterRunProcess(pWorkingRecordSuite, &processResult, workingHostObject, kTriglavPlugInFilterRunProcessStateEnd);


                    if (processResult == kTriglavPlugInFilterRunProcessResultRestart)
                    {
                        restart = true;
                    }
                    else if (processResult == kTriglavPlugInFilterRunProcessResultExit)
                    {
                        break;
                    }
                }
                *result = kTriglavPlugInCallResultSuccess;


                
            }
            customFilter.ShutDown();
        }
    }
    return;
}


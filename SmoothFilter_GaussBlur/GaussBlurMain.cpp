
#include "TriglavPlugInSDK/TriglavPlugInSDK.h"
#include<assert.h>
#include <iostream>
#include <vector>
#include<thread>
#include"GaussBlur.h"
typedef	unsigned char	BYTE;

static    const int kItemBlurRadius = 1;
static	const int kItemPreview = 2;

//���ػ��ַ���id
static	const int kStringIDFilterCategoryName = 101;//����ļ�������������ϼ��˵�����
static	const int kStringIDFilterName = 102;//�˾������
static    const int kStringIDItemBlurRadius = 103;//ģ���뾶
static	const int kStringIDPreview = 104;//Ԥ��

static const char* uuidOfThisPlugin = "1E8B8396-201B-49D2-9AAC-33E5B76DA8AE";


struct	HSVFilterInfo
{
	double    blurRadius;
	bool	preview;
	TriglavPlugInPropertyService* pPropertyService;
};



//	�ץ�ѥƥ����`��Хå�
//���Իص�
static void TRIGLAV_PLUGIN_CALLBACK TriglavPlugInFilterPropertyCallBack(TriglavPlugInInt* result, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInInt notify, TriglavPlugInPtr data)
{



	(*result) = kTriglavPlugInPropertyCallBackResultNoModify;


	HSVFilterInfo* pFilterInfo = static_cast<HSVFilterInfo*>(data);

	if (pFilterInfo != NULL)
	{
		TriglavPlugInPropertyService* pPropertyService = (*pFilterInfo).pPropertyService;


		if (pPropertyService != NULL)
		{



			//��������޸���
			//����Ǹ�֪������csp�б��޸���
			if (notify == kTriglavPlugInPropertyCallBackNotifyValueChanged)
			{







				TriglavPlugInDouble	value;
				(*(*pFilterInfo).pPropertyService).getDecimalValueProc(&value, propertyObject, kItemBlurRadius);
				TriglavPlugInBool _bpreview;
				(*(*pFilterInfo).pPropertyService).getBooleanValueProc(&_bpreview, propertyObject, kItemPreview);


				if (_bpreview == false)
				{
					if (pFilterInfo->preview)
					{
						GaussBlur::GetIns().OnParamChanged(0);
					}
				}
				else
				{
					GaussBlur::GetIns().OnParamChanged(value);
				}
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
			GaussBlur::PluginModuleCleanUp();
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
//                TriglavPlugInFilterInitializeSetCanPreview(pRecordSuite,hostObject,true);
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

				//��������ɫ�໬��//��������cspԭʼ������ͬ�Ļ���Ӧ�������ֳɵ�����
				//ģ���뾶
				TriglavPlugInStringObject	blurRadiusCaption = NULL;
				(*pStringService).createWithStringIDProc(&blurRadiusCaption, kStringIDItemBlurRadius, (*pluginServer).hostObject);
				(*pPropertyService).addItemProc(propertyObject, kItemBlurRadius, kTriglavPlugInPropertyValueTypeDecimal, kTriglavPlugInPropertyValueKindDefault, kTriglavPlugInPropertyInputKindDefault, blurRadiusCaption, 'b');
				(*pPropertyService).setDecimalValueProc(propertyObject, kItemBlurRadius, 0);
				(*pPropertyService).setDecimalDefaultValueProc(propertyObject, kItemBlurRadius, 6);
				(*pPropertyService).setDecimalMinValueProc(propertyObject, kItemBlurRadius, 0);
				(*pPropertyService).setDecimalMaxValueProc(propertyObject, kItemBlurRadius, 256);
				(*pStringService).releaseProc(blurRadiusCaption);//�ֶ��ͷ��ַ�������


				TriglavPlugInStringObject    blurPreviewCaption = NULL;
				(*pStringService).createWithStringIDProc(&blurPreviewCaption, kStringIDPreview, (*pluginServer).hostObject);
				(*pPropertyService).addItemProc(propertyObject, kItemPreview, kTriglavPlugInPropertyValueTypeBoolean, kTriglavPlugInPropertyValueKindDefault, kTriglavPlugInPropertyInputKindDefault, blurPreviewCaption, 'p');
				(*pPropertyService).setBooleanValueProc(propertyObject, kItemPreview, true);
				(*pPropertyService).setBooleanDefaultValueProc(propertyObject, kItemPreview, true);
				(*pStringService).releaseProc(blurPreviewCaption);



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


			auto& customFilter = GaussBlur::GetIns();

			//RUNRecord���壿
			if (TriglavPlugInGetFilterRunRecord(pWorkingRecordSuite) != NULL && pWorkingOffscreenService != NULL && pWorkingPropertyService != NULL)
			{
				customFilter.Init(pluginServer);




				//���ý���
				//����Ҫ���ý��ȣ�ai��ͼ�ǰ�


				//��ʼ��
				HSVFilterInfo* pFilterInfo = static_cast<HSVFilterInfo*>(*data);
				(*pFilterInfo).pPropertyService = pWorkingPropertyService;
				(*pFilterInfo).blurRadius = 6;
				(*pFilterInfo).preview = true;
				customFilter.Render(pFilterInfo->blurRadius);
				//                customFilter.Render(200);
								//����restart���� ��λ��
				bool	restart = true;


				std::cout << "loop entrypoint" << std::endl;
				while (true)
				{

					//����������Ϣ���ɣ���������ڲ����ı�Ļص����Ƚ���������


					//                    customFilter.SetSkip(false);
					//                    customFilter.Render(pFilterInfo->blurRadius);
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


				customFilter.ShutDown();
			}
		}
	}
	return;
}


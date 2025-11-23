#pragma once
#include<vector>
#include"LayerObject.h"
class ScriptAction_ApplyEffects
{

public:
	bool CheckHasEffects();
	void DoAction();
	void GetProgress(uint32_t* processedLayerCount, uint32_t* totalLayerCount) 
	{ *processedLayerCount= _processedLayerCount; *totalLayerCount= _totalLayerCount;};

	static bool Available() { return LayerObject::ApplyEffectAvailable(); };
	static void Enable();
	static void Disable();
	static bool IsEnabled() { return isEnabled; };

	bool IsEnd() { return  _isEnd; };





	void DoInit();
	bool DoProcessLayer();







private:
	static inline bool isEnabled = false;



	void _ApplyForLayer(const std::vector<int>& layerIndex);



	LayerObject effectsGroup;
	int effectsGroupIndex = -1;
	std::atomic<uint32_t> _processedLayerCount = 0;
	std::atomic<uint32_t> _totalLayerCount = 0;
	std::atomic<bool> _isEnd = false;
	std::vector<std::vector<int>> targetLayers;
	

	


};
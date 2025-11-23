
#include<SDL3/SDL.h>
#include "ScriptAction_ApplyEffects.h"




//打开图层组以及其子图层组//关闭锁定的图层组
void OpenLayerGroup(LayerObject layer, bool bOpen = true)
{
    if (!layer.IsGroup())return;
    if (layer.IsLocked())
    {
        layer.SetGroupOpen(false);
        return;
    }
    layer.SetGroupOpen(bOpen);
    for (int i = 0; i < layer.GetChildLayerCount(); i++)
    {
        //递归
        OpenLayerGroup(layer.GetChildAt(i), bOpen);
    }
}

//根据当前图层组的索引寻找下一个图层位置
std::vector<int> FindNextTarget(const std::vector<int>& curTarget, bool onlyInGroup = false)
{
    //定位到当前图层//虽然是频繁调用的函数，但这里不需要优化vector内存和函数逻辑，
    // 因为时间的绝对大头在图层处理，这里怎么方便怎么写

    //递归的退出条件
    if (curTarget.empty())return std::vector<int>();






    //std::vector<LayerObject> groupList;
    //groupList.push_back(LayerObject::GetCurrentRoot());
    ////auto root=LayerObject::GetCurrentRoot();
    //for (int i = 0; i < curTarget.size() - 1; i++)
    //{
    //    groupList.push_back(groupList.back().GetChildAt_TopToBottom(i));
    //}

    ////在当前图层组里寻找下一个合法图层
    //bool found = false;
    //for (int i = curTarget.back() + 1; i < groupList.back().GetChildLayerCount(); i++)
    //{
    //    auto groupName = groupList.back().GetName();
    //    LayerObject nextLayer = groupList.back().GetChildAt_TopToBottom(i);

    LayerObject curGroup = LayerObject::GetCurrentRoot();
    for (int i = 0; i < curTarget.size() - 1; i++)
    {
        //误写成下面这样的了，找了半天问题..
        //curGroup = curGroup.GetChildAt_TopToBottom(i);
        curGroup = curGroup.GetChildAt_TopToBottom(curTarget[i]);
    }

    //在当前图层组里寻找下一个合法图层
    bool found = false;
    for (int i = curTarget.back() + 1; i < curGroup.GetChildLayerCount(); i++)
    {
        //auto groupName = curGroup.GetName();
        LayerObject nextLayer = curGroup.GetChildAt_TopToBottom(i);








        if (nextLayer.IsLocked())
        {
            if (nextLayer.IsGroup())nextLayer.SetGroupOpen(false);
            continue;
        }
        if (nextLayer.IsAdjustLayer())
        {
            continue;
        }
        if (nextLayer.IsGroup())
        {
            //nextLayer.SetGroupOpen(true);
            std::vector<int> vec = curTarget;
            vec.back() = i;
            vec.push_back(-1);//不包括起始索引，使用负1以使下次搜索从0开始
            vec = FindNextTarget(vec, true);
            if (vec.empty())
                continue;
            return vec;
        }
        else
        {
            //寻找到了未被锁定的非图层组图层
            std::vector<int> vec = curTarget;
            vec.back() = i;
            return vec;
        }
    }
    //如果传入了onlyInGroup则不进行递归
    if (onlyInGroup)return  std::vector<int>();


    //当前图层组未找到，返回上个图层组
    std::vector<int> vec = curTarget;
    vec.pop_back();
    return FindNextTarget(vec);
}




void ScriptAction_ApplyEffects::Enable()
{
    if (!isEnabled && Available())
    {
        isEnabled = true;
    }

}

void ScriptAction_ApplyEffects::Disable()
{
    if (isEnabled && Available())
    {
        isEnabled = false;
    }
}

void ScriptAction_ApplyEffects::DoInit()
{

    LayerObject root = LayerObject::GetCurrentRoot();
    int layerCount = root.GetChildLayerCount();

    //后期层之前的图层组全部关闭，之后的图层组全部打开
    effectsGroup.SetGroupOpen(false);
    //将后期层之前的图层组全部关闭
    for (int i = 0; i < effectsGroupIndex; i++)
    {
        LayerObject curLayer = root.GetChildAt_TopToBottom(i);
        if (curLayer.IsGroup())
            curLayer.SetGroupOpen(false);
    }







    //对需要处理的图层进行统计

    targetLayers.clear();//targetLayers[需要处理的第x个图层][图层所在的位置]
    //如 std::vector<int> 内容为 3 2 4，
    // 表示这个图层在根组的第三号索引位置的图层组中，
    //在这个图层组的第2号索引位置的图层组中的第四号索引
    std::vector<int> vec; vec.push_back(effectsGroupIndex);
    while (true)
    {
        vec = FindNextTarget(vec);
        if (vec.empty())break;
        targetLayers.push_back(vec);

        std::cout << "push vec:";
        for (auto& x : vec)
            std::cout << x << ",";
        std::cout << std::endl;
    }

    _totalLayerCount = static_cast<uint32_t>(targetLayers.size());

    SDL_Log("Layers to Apply Effects Num: %llu", targetLayers.size());



    //(否决)将后期之后的图层全部打开XXXX
//这次实现的机制更改了，应该先将所有图层关闭，以能正确将后期图层组移动到目标图层上
    for (int i = effectsGroupIndex + 1; i < layerCount; i++)
    {
        LayerObject curLayer = root.GetChildAt_TopToBottom(i);
        if (curLayer.IsGroup())
        {
            OpenLayerGroup(curLayer, false);
        }
    }


}

bool ScriptAction_ApplyEffects::DoProcessLayer()
{
    if (targetLayers.empty())return false;

    if (_processedLayerCount == _totalLayerCount)
    {
        //执行完后，将后期图层锁定
        effectsGroup.SetLocked(true);
        effectsGroup.SetVisibility(false);
        effectsGroup.MoveDown();
        effectsGroup.MoveUp();

        _isEnd = true;
        return false;
    }

    //对具体图层进行处理

    auto& x = targetLayers[_processedLayerCount];

    //for (auto& x : targetLayers)
    {
        _ApplyForLayer(x);
        _processedLayerCount++;

        SDL_Log("Layer action: %d/%d", _processedLayerCount.load(), _totalLayerCount.load());
        //if (_processedLayerCount == 1)return;

    }
    return true;
}





bool ScriptAction_ApplyEffects::CheckHasEffects()
{
    if (!isEnabled)return false;


    LayerObject root = LayerObject::GetCurrentRoot();
    if (root.ptr == 0)return false;



    int layerCount = root.GetChildLayerCount();
    if (layerCount <= 1)return false;



    
    //LayerObject effectsGroup;
    bool result = false;

    //-1:effects组不能是最后一个图层
    for (int i = 0; i < layerCount-1; i++)
    {
        LayerObject curLayer = root.GetChildAt_TopToBottom(i);
        if (!curLayer.IsGroup())
        {
            continue;
        }
        if (curLayer.IsLocked())
        {
            continue;
        }
        if (curLayer.GetChildLayerCount()<=0)
        {
            continue;
        }

        std::wstring_view layerNmae=curLayer.GetName();
        if (layerNmae == L"EFFECTS"|| layerNmae==L"Effects"|| layerNmae==L"effects"
            || layerNmae==L"后期")
        {
            effectsGroup = curLayer;
            effectsGroupIndex = i;
            return true;
        }
    }
    return false;
}



















void ScriptAction_ApplyEffects::DoAction()
{
    if (!isEnabled)return;



    LayerObject root = LayerObject::GetCurrentRoot();
    int layerCount = root.GetChildLayerCount();

    //后期层之前的图层组全部关闭，之后的图层组全部打开
    effectsGroup.SetGroupOpen(false);
    //将后期层之前的图层组全部关闭
    for (int i = 0; i < effectsGroupIndex; i++)
    {
        LayerObject curLayer = root.GetChildAt_TopToBottom(i);
        if (curLayer.IsGroup())
            curLayer.SetGroupOpen(false);
    }

    





    //对需要处理的图层进行统计
    std::vector<std::vector<int>> targetLayers;//targetLayers[需要处理的第x个图层][图层所在的位置]
    //如 std::vector<int> 内容为 3 2 4，
    // 表示这个图层在根组的第三号索引位置的图层组中，
    //在这个图层组的第2号索引位置的图层组中的第四号索引
    std::vector<int> vec; vec.push_back(effectsGroupIndex);
    while (true)
    {
        vec=FindNextTarget(vec);
        if (vec.empty())break;
        targetLayers.push_back(vec);
        
        std::cout << "push vec:";
        for (auto& x : vec)
            std::cout << x<<",";
        std::cout << std::endl;
    }

    _totalLayerCount = static_cast<uint32_t>(targetLayers.size());

    SDL_Log("Layers to Apply Effects Num: %llu", targetLayers.size());



    //(否决)将后期之后的图层全部打开XXXX
//这次实现的机制更改了，应该先将所有图层关闭，以能正确将后期图层组移动到目标图层上
    for (int i = effectsGroupIndex + 1; i < layerCount; i++)
    {
        LayerObject curLayer = root.GetChildAt_TopToBottom(i);
        if (curLayer.IsGroup())
        {
            OpenLayerGroup(curLayer, false);
        }
    }


    //对具体图层进行处理
    for (auto& x : targetLayers)
    {
        _ApplyForLayer(x);
        _processedLayerCount++;

        SDL_Log("Layer action: %d/%d", _processedLayerCount.load(), _totalLayerCount.load());
        //if (_processedLayerCount == 1)return;

    }

    //执行完后，将后期图层锁定
    effectsGroup.SetLocked(true);
    effectsGroup.SetVisibility(false);
    effectsGroup.MoveDown();
    effectsGroup.MoveUp();

    _isEnd = true;
}

void ScriptAction_ApplyEffects::_ApplyForLayer(const std::vector<int>& layerIndex)
{

    std::vector<LayerObject> groupList;
    groupList.push_back(LayerObject::GetCurrentRoot());
    for (int i = 0; i < layerIndex.size() - 1; i++)
    {
         groupList.push_back(groupList.back().GetChildAt_TopToBottom(layerIndex[i]));
    }

    //将图层组打开
    for (int i = 1; i < groupList.size(); i++)
    {
        groupList[i].SetGroupOpen(true);
    }
    effectsGroup.SetGroupOpen(false);

    LayerObject targetObj= groupList.back().GetChildAt_TopToBottom(layerIndex.back());
    
    bool hasLayerClippingMask = targetObj.HasLayerClippingMask();
    bool hasLayerMask = targetObj.HasLayerMask();
    bool origVisibility = targetObj.IsVisible();

    targetObj.SetVisibility(true);
    targetObj.SetLayerMaskVisibility(false);
    targetObj.SetLayerClippingMask(false);

    effectsGroup.SetSelect();
    LayerObject::MoveDown();//不要觉得很奇怪，这是为了刷新画面，防止合并、复制的时候CSP计算未设置蒙版等信息时的画面
    LayerObject::MoveUp();
    //if (hasLayerMask)
    {
        targetObj.SetSelect();
        //如果有蒙版则创建新图层
        LayerObject::Duplicate();
        //targetObj.SetOpacity(0);
        //LayerObject::CreateLayer();
        //LayerObject::MoveDown();
        //LayerObject::MergeDown();
        targetObj = groupList.back().GetChildAt_TopToBottom(layerIndex.back());
    }






    effectsGroup.SetSelect();
    LayerObject::Duplicate();

    LayerObject newEffectsGroup = LayerObject::GetCurrentRoot().GetChildAt_TopToBottom(effectsGroupIndex);
    effectsGroup.SetGroupOpen(false);
    newEffectsGroup.SetGroupOpen(false);
    newEffectsGroup.SetSelect();
 //计算需要向下移动的次数
//以{3,4,5}为例子
//图层布局为
//00
//01
//02
//03
//10
//11
//12
//13
//14
//20
//21
//22
//23
//24
//25
//图层总数是4+5+6
//如果在00上方开始移动，则需要移动4+5+6-1次
//effectGroup是关闭状态，假设其index是02
//拷贝后的新effectgroup在其上方，其需要移动的部署可以通过00层上方需要移动的步数做差
//
    int32_t moveTimes = 0;
    for (auto x : layerIndex)
    {
        moveTimes = moveTimes + x + 1;
    }
    moveTimes -= 1;//得到从00层的上方到目标层的移动次数，
    moveTimes -= effectsGroupIndex;

    for(int i=0;i< moveTimes;i++)
        LayerObject::MoveDown();






    //图层组已经移动到了目标层上方进行合并操作
    {
        newEffectsGroup.SetGroupOpen();
        auto childCount=newEffectsGroup.GetChildLayerCount();
        //uint16_t groupOpacity=newEffectsGroup.GetOpacity();
        //newEffectsGroup.SetOpacity(0x100);
        ////将内部所有图层设置为剪辑蒙版,进行透明度转换
        //for (int i = 0; i < childCount; i++)
        //{
        //    auto curLayer = newEffectsGroup.GetChildAt(i);
        //    uint16_t curLayerOpacity=curLayer.GetOpacity();
        //    auto resultOpacity = (groupOpacity==0x100? curLayerOpacity: static_cast<uint16_t>( (curLayerOpacity / 256.f)*(groupOpacity/256.f)*256.f));
        //    resultOpacity = SDL_min(256, resultOpacity);
        //    curLayer.SetOpacity(resultOpacity);
        //    curLayer.SetLayerClippingMask(true);
        //}



        for (int i = 0; i < childCount; i++)
        {
            auto curLayer = newEffectsGroup.GetChildAt_TopToBottom(i);
            curLayer.SetLayerClippingMask(true);
        }

        targetObj.SetSelect();
        LayerObject::MoveUp();//移动到图层组内
        
        for (int i = 0; i < childCount; i++)
        {
            auto curLayer = newEffectsGroup.GetChildAt_TopToBottom(i);
            curLayer.AddSelect();
        }


        newEffectsGroup.AddSelect();
        //合并选择图层
        //合并所选图层会导致图层蒙版消失
        LayerObject::MergeSelect();
        targetObj = groupList.back().GetChildAt_TopToBottom(layerIndex.back());
        targetObj.SetLayerClippingMask(true);
        targetObj.SetSelect();
        LayerObject::MoveUp();//不要觉得很奇怪，这是为了刷新画面，防止合并、复制的时候CSP计算未设置蒙版等信息时的画面
        LayerObject::MoveDown();
        LayerObject::MergeDown();//这里向下合并的是我们创建的有蒙版的空图层
    }




    //恢复状态
    targetObj = groupList.back().GetChildAt_TopToBottom(layerIndex.back());
    targetObj.SetVisibility(origVisibility);
    targetObj.SetLayerMaskVisibility(hasLayerMask);
    targetObj.SetLayerClippingMask(hasLayerClippingMask);//这个会触发刷新，注意顺序以使数据有效

    //将图层组关闭
    for (int i = 1; i < groupList.size(); i++)
    {
        groupList[i].SetGroupOpen(false);
    }
    //恢复图层组
    effectsGroup.SetGroupOpen(false);
    effectsGroup.SetSelect();
}

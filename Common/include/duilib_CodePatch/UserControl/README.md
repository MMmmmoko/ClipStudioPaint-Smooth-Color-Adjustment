这里的控件不是原库的默认控件，不会自动于XML中识别，
需要添加#include"DuiUserControls.h"
然后#include UserControlCreate.inl进ui::GlobalManager::Instance().Startup的创建回调中;
# CLIP STUDIO PAINT 自定义色调调整的可行性

| 产品版本 | 原始可执行文件 | SHA-256 |
| --- | --- | --- |
| 5.0.0 | `CLIPStudioPaint.exe` | `C04A3D8ADD9E0BEEF1F06EDB43D3B3180D2C1E470585B87D5E750F392866B69C` |
| 4.2.0 | `CLIPStudioPaint420.exe` | `D6A2EAC55B22BEFC089A8E7D2BC73B89994B354CECF598AAB80C37BBA6E82E42` |

## 可用的扩展路径

- **直接调整像素**。CELSYS 为 CSP EX 提供滤镜 SDK。仓库里的官方 HSV 示例注册滤镜名称、参数和预览，通过源及目标离屏图像处理选区，并通知宿主刷新。自定义算法可以沿用这套流程，从「滤镜」进入，确认后写入像素。如果入口必须放在「编辑 → 色调调整」，需要另外接入原生命令；SDK 公开了滤镜名称和分类，未发现通用的原生菜单注册接口。[官方 SDK](https://www.clipstudio.net/ja/dl/cspsdk/)、[HSV 示例](../../FilterPlugInCSPSAMPLE/Source/HSV/PIHSVMain.cpp)
- **能表达为内置参数的无损调整**。可以由 CSPMOD 提供自定义控件，将参数转换成原生曲线、色阶等数据，然后创建对应的原生调整图层。例如，可用 RGB 各通道的独立曲线表达的调整可以沿用曲线层，保留原生重算、蒙版和保存机制。双击后会使用宿主原生参数界面，除非再接入 CSPMOD 的编辑入口。
- **独立的无损调整类型**。两个版本都存在集中创建 `PWFilterLayerInfo` 并按内置类型配置回调的路径。色阶、曲线的配置分别绑定渲染准备、离屏区域处理和类型专属的参数读写。CSPMOD 有可调查的接入点。若要让任意算法成为新类型，还需实现新建和再次编辑命令、参数生命周期和保存恢复，并验证复制、撤销和合成。滤镜 SDK 的参数和预览仅用于单次滤镜运行，现有曲线与色阶优化也只改变已存在类型的预览提交。[原生调整图层行为](https://help.clip-studio.com/en-us/manual_en/390_filters/Tonal_Correction.htm)、[色阶实时预览调查](CLIPStudioPaint-Levels-Preview.md)

## 二进制定位复核

| 位置 | 定位及确认条件 | 5.0.0 | 4.2.0 |
| --- | --- | --- | --- |
| 曲线命令标识初始化 | 用 `filtertonecurve` 和 `layeradjustmenttonecurvenew` 两个完整字符串分别查找 RIP 相对引用，核对两个入口跳转至同一标识初始化函数。 | 各 1 | 各 1 |
| 调整图层对象工厂 | 从 `.?AVPWFilterLayerInfo@Planeswalker@@` 的 RTTI 解析虚表，沿引用区分清理与构造。构造路径写入类型、排除零，再用 `类型 − 1 ≤ 11` 的检查进入跳转表。 | 1 | 1 |
| 色阶和曲线各自的类型配置函数 | 由 `SAdjustmentLevels`、`SAdjustmentCurves` 的 `PWImportStream` 与 `PWExportStream` 函数指针类型查找类型擦除的包装代码，再沿只读区回调描述和实际引用找到配置函数。每种类型的两个描述都须落在同一函数，且该函数须由上述工厂的分支调用。 | 各 1 | 各 1 |

从 `PWFilterLayerInfo&` 与 `PWVOffscreenAccessor&`、`SRect const&` 组成的函数签名可找到离屏渲染回调的描述。交叉引用显示两个版本均有 12 个配置函数同时绑定渲染准备和区域渲染回调。这说明可复用宿主的调整图层框架；自定义类型 ID 能否在整个文档读写流程中保留，以及原版 CSP 打开含该类型的文档时的行为，仍需验证。

独立类型的最小验证可使用一个只有一个强度参数、效果明显的颜色变换。从 CSPMOD 菜单创建后，在 5.0.0 验证下层修改会触发重算、调整图层的遮罩可正常限制效果、保存重开能恢复类型和参数，并能再次编辑。两版结论目前均来自静态分析。

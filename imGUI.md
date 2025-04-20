## Text 

| 函数名               | 参数及含义                                                                                                                                           | 用途说明                               |
|----------------------|------------------------------------------------------------------------------------------------------------------------------------------------------|----------------------------------------|
| TextEx() [Internal]  | `text`: 要显示的字符串（不可为 NULL）<br>`text_end`: 字符串结束指针，若为 NULL 则以 '\0' 结尾<br>`flags`: ImGuiTextFlags，用于控制渲染行为        | 底层文本绘制函数，用于显示原始文本     |
| TextUnformatted()    | `text`: 要显示的字符串<br>`text_end`: 可选，字符串结束位置，若为 NULL 则以 '\0' 结尾                                                                 | 显示未格式化的纯文本                    |
| Text()               | `fmt`: 格式字符串<br>`...`: 可变参数，按 printf 规则传入                                                                                             | 显示格式化文本                         |
| TextV()              | `fmt`: 格式字符串<br>`args`: va_list 类型，可变参数                                                           | 与 Text 类似，接受 va_list             |
| TextColored()        | `col`: 颜色（ImVec4）<br>`fmt`: 格式字符串<br>`...`: 可变参数                                                  | 以指定颜色显示格式化文本               |
| TextColoredV()       | `col`: 颜色（ImVec4）<br>`fmt`: 格式字符串<br>`args`: va_list                                                  | 与 TextColored 类似，接受 va_list      |
| TextDisabled()       | `fmt`: 格式字符串<br>`...`: 可变参数                                                                          | 以灰色显示文本，常用于提示或禁用项     |
| TextDisabledV()      | `fmt`: 格式字符串<br>`args`: va_list                                                                          | 与 TextDisabled 类似，接受 va_list     |
| TextWrapped()        | `fmt`: 格式字符串<br>`...`: 可变参数                                                                          | 自动换行显示格式化文本                 |
| TextWrappedV()       | `fmt`: 格式字符串<br>`args`: va_list                                                                          | 与 TextWrapped 类似，接受 va_list      |
| LabelText()          | `label`: 标签文字<br>`fmt`: 格式字符串<br>`...`: 可变参数                                                      | 显示一个标签和其对应内容               |
| LabelTextV()         | `label`: 标签文字<br>`fmt`: 格式字符串<br>`args`: va_list                                                      | 与 LabelText 类似，接受 va_list        |
| BulletText()         | `fmt`: 格式字符串<br>`...`: 可变参数                                                                          | 显示带有项目符号（•）的文本项          |
| BulletTextV()        | `fmt`: 格式字符串<br>`args`: va_list                                                                          | 与 BulletText 类似，接受 va_list       |

## Button

| 函数名                   | 参数及含义                                                                                                                                                                                                                  | 用途说明                                  |
|--------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-------------------------------------------|
| ButtonBehavior() [Internal] | `bb`: 按钮的边界矩形（ImRect）<br>`id`: 唯一控件 ID（ImGuiID）<br>`out_hovered`: 输出参数，是否被鼠标悬停<br>`out_held`: 输出参数，是否被按住<br>`flags`: ImGuiButtonFlags 选项                                   | 内部通用按钮行为处理                      |
| Button()                 | `label`: 显示文本<br>`size`: 按钮尺寸（默认 0,0 表示自动尺寸）                                                                                                                     | 创建一个标准按钮                          |
| SmallButton()            | `label`: 显示文本                                                                                                                                                                   | 创建一个小尺寸按钮                        |
| InvisibleButton()        | `str_id`: 唯一 ID<br>`size`: 按钮尺寸                                                                                                                                                | 创建不可见的按钮区域，响应鼠标操作        |
| ArrowButton()            | `str_id`: 唯一 ID<br>`dir`: 箭头方向（ImGuiDir_Up/Down/Left/Right）                                                                                                                 | 显示一个带箭头的按钮                      |
| CloseButton() [Internal] | `id`: 唯一控件 ID<br>`pos`: 绘制位置<br>`flags`: ImGuiButtonFlags                                                                                                                  | 绘制“×”关闭按钮                           |
| CollapseButton() [Internal] | `id`: 唯一控件 ID<br>`pos`: 绘制位置                                                                                                                                            | 绘制“展开/折叠”按钮                       |

##  Checkbox / RadioButton / ProgressBar / Bullet / Hyperlink

| 函数名                  | 参数及含义                                                                                                                                                      | 用途说明                                 |
|---------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------------------------------|
| Checkbox()                | `label`: 显示文本<br>`v`: bool 指针，表示当前选中状态                                                                                                         | 渲染一个复选框                           |
| CheckboxFlagsT() [Internal] | `label`: 显示文本<br>`flags`: 泛型位掩码指针<br>`flags_value`: 要切换的位值                                                                                 | 内部泛型实现，用于模板化的标志位切换     |
| CheckboxFlags()           | `label`: 显示文本<br>`flags`: int*/uint* 等指针，表示当前标志位<br>`flags_value`: 要切换的特定位                                                             | 渲染一个用于切换标志位的复选框           |
| RadioButton()             | `label`: 显示文本<br>`active`: bool 或 int 当前值                                                                                                              | 渲染一个单选按钮（排他性）               |
| ProgressBar()             | `fraction`: 当前进度 [0.0, 1.0]<br>`size_arg`: 大小（ImVec2）<br>`overlay`: 可选叠加文本                                                                       | 显示一个进度条                           |
| Bullet()                  | 无参数                                                                                                                                                         | 显示一个圆点（用于无序列表）             |
| Hyperlink()               | `label`: 文本<br>`url`: 网址                                                                                                                                   | 显示一个可以点击打开的网址超链接         |

## Spacing / Dummy / NewLine / Separator 


| 函数名                      | 参数及含义                                                                                                                                         | 用途说明                                     |
|-----------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------|----------------------------------------------|
| Spacing()                   | 无参数                                                                                                                                             | 添加一个空行的间距                           |
| Dummy()                     | `size`: 占位尺寸（ImVec2）                                                                                                                         | 添加一个不可见的占位控件                     |
| NewLine()                   | 无参数                                                                                                                                             | 换行（等价于 Dummy(ImVec2(0, text_line_height))) |
| AlignTextToFramePadding()   | 无参数                                                                                                                                             | 将文本垂直对齐到当前控件的 frame padding     |
| SeparatorEx() [Internal]    | `flags`: ImGuiSeparatorFlags                                                                                                                       | 内部分隔线绘制实现                           |
| Separator()                 | 无参数                                                                                                                                             | 添加一个水平分隔线                           |
| SplitterBehavior() [Internal] | `bb`: 拖动区域（ImRect）<br>`id`: 控件 ID<br>`axis`: 分割方向<br>`size1/size2`: 双侧区域尺寸指针<br>`min_size1/2`: 最小值<br>`hovered`: bool* | 内部分隔器行为                               |
| ShrinkWidths() [Internal]   | `items`: 多个区域的宽度数组<br>`count`: 区域数<br>`width_excess`: 要缩减的总宽度                                                                   | 内部用于多列自适应缩放                       |


## Combo

| 函数名                          | 参数及含义                                                                                                                                             | 用途说明                                      |
|-----------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------|
| CalcMaxPopupHeightFromItemCount() [Internal] | `items_count`: 列表项数量                                                                                                                      | 计算弹出列表最大高度（用于下拉菜单）          |
| BeginCombo()                      | `label`: 标签<br>`preview_value`: 当前选项文本<br>`flags`: ImGuiComboFlags（可选）                                                                   | 开始一个下拉列表控件                          |
| BeginComboPopup() [Internal]      | `popup_id`: 唯一ID<br>`bb`: 矩形区域（ImRect）<br>`flags`: 标志                                                                                      | 内部方法，用于下拉菜单的弹出实现              |
| EndCombo()                        | 无参数                                                                                                                                               | 结束 BeginCombo 调用                         |
| BeginComboPreview() [Internal]    | 无参数                                                                                                                                               | 内部方法，用于渲染 Combo 展开前的区域        |
| EndComboPreview() [Internal]      | 无参数                                                                                                                                               | 结束 ComboPreview                             |
| Combo()                           | `label`: 标签<br>`current_item`: 当前选项索引的指针<br>`items[]`: 字符串数组<br>`items_count`: 数量<br>`popup_max_height_in_items`: 限制显示项数  | 渲染一个标准的下拉列表控件                    |

## Drag 

| 函数名                      | 参数及含义                                                                                                                                                          | 用途说明                                  |
|-------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------|
| DragBehaviorT<>() [Internal]  | `v`: 值指针<br>`speed`: 拖动速度<br>`min/max`: 范围限制<br>`format`: 格式化字符串<br>`flags`: 拖动标志                                                         | 模板化内部拖动行为实现                      |
| DragBehavior() [Internal]     | 同上（非模板版本）                                                                                                                                                | 通用内部拖动行为                            |
| DragScalar()                  | `label`: 标签<br>`data_type`: 数据类型<br>`p_data`: 数据指针<br>`v_speed`: 速度<br>`min/max`: 范围<br>`format`: 格式<br>`flags`: 拖动标志                   | 拖动任意基础类型变量                        |
| DragScalarN()                 | `label`: 标签<br>`data_type`: 数据类型<br>`p_data`: 数据数组<br>`components`: 分量数<br>`v_speed`: 速度...（其余同上）                                          | 拖动标量数组                                |
| DragFloat()/Int()/Double()    | `label`: 标签<br>`v`: 数值指针<br>`v_speed`: 拖动速度<br>`v_min/max`: 限制范围<br>`format`: 显示格式<br>`flags`: 拖动标志                                      | 拖动单个 float/int/double 值控件            |
| DragFloat2/3/4()              | 同上，`v`: 指向 2/3/4 个分量的数组                                                                                                                               | 拖动 2/3/4 维向量                           |
| DragFloatRange2()             | `label`: 标签<br>`v_current_min/max`: 最小/最大值指针<br>`v_speed`: 拖动速度<br>`min/max`: 限制范围<br>`format`: 格式<br>`display_format_max`: 格式（最大） | 拖动范围值（两个 float）                    |
| DragInt2/3/4()                | 同 DragFloat2/3/4，只是类型为 int                                                                                                                                  | 拖动多个整数                               |
| DragIntRange2()               | 同 DragFloatRange2 但用于 int 类型                                                                                                                                | 拖动整数范围                                |

## Slider

| 函数名                                | 参数及含义                                                                                                                                                           | 用途说明                                     |
|-----------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------|----------------------------------------------|
| ScaleRatioFromValueT<> [Internal]       | `v`: 当前值<br>`v_min`: 最小值<br>`v_max`: 最大值<br>`is_logarithmic`: 是否对数模式                                                                                  | 将数值映射为比值（内部用于滑块计算）         |
| ScaleValueFromRatioT<> [Internal]       | `t`: 比例<br>`v_min` / `v_max`: 数值范围<br>`is_logarithmic`: 是否对数                                                                                                 | 将比例转换回具体数值（内部滑块使用）         |
| SliderBehaviorT<>() [Internal]          | `bb`: 控件矩形<br>`id`: 控件ID<br>`data_type`: 数据类型<br>`v`: 当前值指针<br>`v_min/max`: 范围<br>`format`: 格式<br>`flags`: 标志                                  | 滑块行为模板实现                             |
| SliderBehavior() [Internal]             | 同上（非模板版本）                                                                                                                                                   | 滑块行为基础实现                             |
| SliderScalar()                          | `label`: 标签<br>`data_type`: 数据类型<br>`p_data`: 数据指针<br>`p_min/max`: 范围<br>`format`: 格式<br>`flags`: 标志                                               | 滑动控制任意类型值                          |
| SliderScalarN()                         | 同上，但带有 `components`: 分量数量                                                                                                                                  | 滑动数组值                                   |
| SliderFloat()/SliderInt()               | `label`: 标签<br>`v`: 值指针<br>`v_min/max`: 限制范围<br>`format`: 显示格式<br>`flags`: 滑块标志                                                                    | 基础的 float / int 滑块控件                 |
| SliderFloat2/3/4()                      | `label`: 标签<br>`v`: 指向 float[2/3/4] 的数组<br>其余同上                                                                                                           | 多维浮点滑块                                 |
| SliderInt2/3/4()                        | 同上，只是类型是 int                                                                                                                                                 | 多维整数滑块                                 |
| SliderAngle()                           | `label`: 标签<br>`v_rad`: 角度值（弧度）指针<br>`v_degrees_min/max`: 最小最大角度（默认 0 到 360）<br>`format`: 格式（默认 "%.0f deg"）                          | 角度滑块控件                                 |
| VSliderScalar()                         | `label`: 标签<br>`size`: 控件大小（ImVec2）<br>`data_type`: 数据类型<br>`p_data`: 数据指针<br>`p_min/max`: 范围<br>`format`: 格式<br>`flags`: 标志               | 垂直滑块（支持任意数据类型）                |
| VSliderFloat()/VSliderInt()             | `label`: 标签<br>`size`: 控件大小<br>`v`: 数据指针<br>`v_min/max`: 范围<br>`format`: 格式<br>`flags`: 标志                                                          | 垂直方向的 float / int 滑块控件             |

## Input

| 函数名                                | 参数及含义                                                                                                                                                      | 用途说明                                        |
|----------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------|-------------------------------------------------|
| TempInputTextScalar() [Internal]       | `label`: 标签<br>`data_type`: 数据类型<br>`p_data`: 值指针<br>`format`: 格式化字符串<br>`flags`: 标志                                                            | 输入临时标量值（内部使用）                     |
| InputScalar()                          | `label`: 标签<br>`data_type`: 数据类型<br>`p_data`: 值指针<br>`step`: 步进值<br>`step_fast`: 快速步进<br>`format`: 显示格式<br>`flags`: 标志                     | 输入任意标量类型数据                            |
| InputScalarN()                         | 同上 + `components`: 输入的组件数量                                                                                                                               | 输入数组形式的标量                              |
| InputFloat()/Int()/Double()            | `label`: 标签<br>`v`: 值指针<br>`step`: 步进<br>`step_fast`: 快速步进<br>`format`: 格式（float 默认"%.3f"，int 默认"%d"）<br>`flags`: 输入标志                   | 输入浮点、整数或双精度值                        |
| InputFloat2/3/4()                      | `label`: 标签<br>`v`: 指向 float[2/3/4] 的数组<br>其余同上                                                                                                        | 输入多个浮点数值                                |
| InputInt2/3/4()                        | `label`: 标签<br>`v`: 指向 int[2/3/4] 的数组                                                                                                                      | 输入多个整数值                                  |
| InputText()                            | `label`: 标签<br>`buf`: 字符数组<br>`buf_size`: 数组大小<br>`flags`: 标志<br>`callback`: 回调函数<br>`user_data`: 用户数据                                      | 单行文本输入框                                  |
| InputTextWithHint()                    | 同上 + `hint`: 占位提示文字                                                                                                                                       | 带有提示的单行输入框                            |
| InputTextMultiline()                   | `label`: 标签<br>`buf`: 缓冲区<br>`buf_size`: 大小<br>`size`: 输入框尺寸<br>`flags`: 标志<br>`callback`: 回调<br>`user_data`: 用户数据                          | 多行文本输入框                                  |
| InputTextGetCharInfo() [Internal]      | `state`: 输入状态<br>`idx`: 字符索引                                                                                                                              | 获取特定字符信息（内部）                        |
| InputTextReindexLines() [Internal]     | `state`: 输入状态                                                                                                                                                | 更新行索引缓存（内部）                          |
| InputTextReindexLinesRange() [Internal]| `state`: 输入状态<br>`begin`, `end`: 行范围                                                                                                                       | 局部重新索引行数据（内部）                      |
| InputTextEx() [Internal]               | `label`: 标签<br>`buf`: 缓冲区<br>`buf_size`: 大小<br>`size_arg`: 控件大小<br>`flags`: 标志<br>`callback`: 回调<br>`user_data`: 用户数据                         | 文本输入主实现函数（内部）                      |
| DebugNodeInputTextState() [Internal]   | `state`: 输入状态                                                                                                                                                | 调试输入框内部状态                              |


## ColorEdit / ColorPicker

| 函数名                               | 参数及含义                                                                                                                                                            | 用途说明                                   |
|--------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------|
| ColorEdit3()                         | `label`: 标签<br>`col`: 颜色值指针（float[3]）<br>`flags`: 标志<br>`display`: 显示模式（默认选择 RGB）                                                                 | 编辑 RGB 颜色                              |
| ColorEdit4()                         | `label`: 标签<br>`col`: 颜色值指针（float[4]）<br>`flags`: 标志<br>`display`: 显示模式（RGBA, HSV等）                                                                 | 编辑 RGBA 颜色                             |
| ColorPicker3()                       | `label`: 标签<br>`col`: 颜色值指针（float[3]）<br>`flags`: 标志<br>`display`: 显示模式（选择 RGB 或 HSV）                                                              | 选择颜色（RGB）                            |
| RenderColorRectWithAlphaCheckerboard() [Internal] | `col`: 颜色值<br>`size`: 大小                                                                                                                                         | 渲染带有 alpha 棋盘图案的颜色矩形（内部）  |
| ColorPicker4()                       | `label`: 标签<br>`col`: 颜色值指针（float[4]）<br>`flags`: 标志<br>`display`: 显示模式（选择 RGB 或 HSV）                                                              | 选择颜色（RGBA）                           |
| ColorButton()                         | `label`: 标签<br>`col`: 颜色值指针<br>`flags`: 标志<br>`size`: 大小                                                                                                                                                        | 颜色选择按钮                              |
| SetColorEditOptions()                | `flags`: 设置颜色编辑的选项（例如是否允许透明度编辑）                                                                                                                | 设置颜色编辑控件的选项                     |
| ColorTooltip() [Internal]            | `col`: 颜色值<br>`size`: 显示尺寸                                                                                                                                      | 显示颜色信息的工具提示（内部）             |
| ColorEditOptionsPopup() [Internal]   | `flags`: 选项标志                                                                                                                                                    | 显示颜色编辑选项弹出框（内部）             |
| ColorPickerOptionsPopup() [Internal] | `flags`: 选项标志                                                                                                                                                    | 显示颜色选择选项弹出框（内部）             |


## TreeNode 

| 函数名                               | 参数及含义                                                                                                                                                            | 用途说明                                   |
|--------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------|
| TreeNode()                           | `label`: 标签<br>`flags`: 标志（例如是否展开）                                                                                                                       | 创建一个树形节点                           |
| TreeNodeV()                          | `label`: 标签<br>`flags`: 标志<br>`user_data`: 用户数据                                                                                                               | 创建一个树形节点，并可附加额外数据         |
| TreeNodeEx()                         | `label`: 标签<br>`flags`: 标志<br>`open`: 初始展开状态                                                                                                                | 扩展的树形节点（自定义更多属性）           |
| TreeNodeExV()                        | `label`: 标签<br>`flags`: 标志<br>`open`: 初始展开状态<br>`user_data`: 用户数据                                                                                      | 扩展的树形节点，并可附加额外数据           |
| TreeNodeBehavior() [Internal]        | `label`: 标签<br>`flags`: 标志<br>`size`: 大小<br>`is_open`: 是否展开                                                                                                  | 内部函数：树节点的行为处理（例如展开状态） |
| TreePush()                           | `str_id`: 树节点的标识符                                                                                                                                             | 推入树形堆栈                               |
| TreePop()                            | 无                                                                                                                                                                    | 弹出树形堆栈                               |
| GetTreeNodeToLabelSpacing()          | 无                                                                                                                                                                    | 获取树形节点标签到节点的水平间距           |
| SetNextItemOpen()                   | `is_open`: 是否展开<br>`cond`: 条件                                                                                                                                  | 设置下一个树节点的展开状态                 |
| CollapsingHeader()                   | `label`: 标签<br>`flags`: 标志<br>`p_open`: 展开状态指针                                                                                                            | 创建可折叠头部                             |

## Selectable 

| 函数名                               | 参数及含义                                                                                                                                                            | 用途说明                                   |
|--------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------|
| Selectable()                         | `label`: 标签<br>`selected`: 是否选中<br>`flags`: 标志<br>`size`: 大小<br>`spacing`: 间距                                                                                                                                 | 创建一个可选择项                           |
| BoxSelectPreStartDrag() [Internal]   | `start`: 起始位置                                                                                                                                                      | 内部函数：开始选择框的拖动操作             |
| BoxSelectActivateDrag() [Internal]   | `drag_id`: 拖动ID                                                                                                                                                      | 内部函数：激活选择框的拖动                 |
| BoxSelectDeactivateDrag() [Internal] | `drag_id`: 拖动ID                                                                                                                                                      | 内部函数：停用选择框的拖动                 |
| BoxSelectScrollWithMouseDrag() [Internal] | `drag_id`: 拖动ID<br>`mouse_pos`: 鼠标位置                                                                                                                              | 内部函数：通过鼠标拖动时，滚动选择框       |
| BeginBoxSelect() [Internal]          | `flags`: 选择框的标志                                                                                                                                                 | 内部函数：开始选择框的创建                 |
| EndBoxSelect() [Internal]            | 无                                                                                                                                                                    | 内部函数：结束选择框的创建                 |
| DebugLogMultiSelectRequests() [Internal] | `selection`: 选择项                                                                                                                                                  | 内部函数：日志记录多选请求                 |
| CalcScopeRect() [Internal]           | 无                                                                                                                                                                    | 内部函数：计算范围矩形                     |
| BeginMultiSelect()                   | 无                                                                                                                                                                    | 开始多选模式                               |
| EndMultiSelect()                     | 无                                                                                                                                                                    | 结束多选模式                               |
| SetNextItemSelectionUserData()       | `user_data`: 用户数据                                                                                                                                                  | 设置下一个项目的选择项数据                 |
| MultiSelectItemHeader() [Internal]   | `item_id`: 项目ID                                                                                                                                                     | 内部函数：多选项的头部                     |
| MultiSelectItemFooter() [Internal]   | `item_id`: 项目ID                                                                                                                                                     | 内部函数：多选项的尾部                     |
| DebugNodeMultiSelectState() [Internal] | `state`: 多选状态                                                                                                                                                     | 内部函数：调试多选状态                     |
| ImGuiSelectionBasicStorage           | 无                                                                                                                                                                    | 基本的选择存储（内部）                     |
| ImGuiSelectionExternalStorage        | `external_storage`: 外部存储                                                                                                                                         | 外部选择存储（内部）                       |


## ListBox 

| 函数名                               | 参数及含义                                                                                                                                                            | 用途说明                                   |
|--------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------|
| BeginListBox()                       | `label`: 标签<br>`size`: 列表框大小<br>`flags`: 标志                                                                                                                                      | 开始一个列表框                             |
| EndListBox()                         | 无                                                                                                                                                                    | 结束列表框                                 |
| ListBox()                            | `label`: 标签<br>`items`: 项目数组<br>`item_count`: 项目数<br>`height_in_items`: 每页显示的项数<br>`item_height`: 单项高度<br>`flags`: 标志                                | 创建一个可滚动的列表框                     |
| PlotEx() [Internal]                  | `label`: 标签<br>`values`: 数据值数组<br>`values_count`: 数据项数<br>`overlay_text`: 显示的文本<br>`scale_min`: 最小刻度<br>`scale_max`: 最大刻度<br>`graph_size`: 图表大小     | 内部函数：绘制图表（例如绘制折线图）        |
| PlotLines()                          | `label`: 标签<br>`values`: 数据值数组<br>`values_count`: 数据项数<br>`scale_min`: 最小刻度<br>`scale_max`: 最大刻度<br>`graph_size`: 图表大小<br>`overlay_text`: 文本显示   | 绘制一条折线图                             |
| PlotHistogram()                      | `label`: 标签<br>`values`: 数据值数组<br>`values_count`: 数据项数<br>`scale_min`: 最小刻度<br>`scale_max`: 最大刻度<br>`graph_size`: 图表大小<br>`overlay_text`: 文本显示   | 绘制直方图                                 |


## Menu

| 函数名                               | 参数及含义                                                                                                                                                            | 用途说明                                   |
|--------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------|
| BeginMenuBar()                       | 无                                                                                                                                                                    | 开始一个菜单栏                             |
| EndMenuBar()                         | 无                                                                                                                                                                    | 结束菜单栏                                 |
| BeginMainMenuBar()                   | 无                                                                                                                                                                    | 开始一个主菜单栏                           |
| EndMainMenuBar()                     | 无                                                                                                                                                                    | 结束主菜单栏                               |
| BeginMenu()                          | `label`: 标签<br>`flags`: 标志                                                                                                                                         | 创建一个菜单项                             |
| EndMenu()                            | 无                                                                                                                                                                    | 结束菜单项                                 |
| MenuItemEx() [Internal]              | `label`: 标签<br>`shortcut`: 快捷键<br>`selected`: 是否选中<br>`enabled`: 是否启用<br>`flags`: 标志                                                                  | 内部函数：菜单项的扩展操作                 |
| MenuItem()                           | `label`: 标签<br>`shortcut`: 快捷键<br>`selected`: 是否选中<br>`enabled`: 是否启用<br>`flags`: 标志                                                                  | 创建一个菜单项                             |


## Tab

| 函数名                               | 参数及含义                                                                                                                                                            | 用途说明                                   |
|--------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------|
| BeginTabBar()                        | `label`: 标签<br>`flags`: 标签栏标志                                                                                                                                     | 开始一个标签栏                             |
| BeginTabBarEx() [Internal]           | `label`: 标签<br>`flags`: 标签栏标志<br>`tab_bar_flags`: 标签栏的额外标志                                                                                             | 内部函数：开始标签栏的扩展操作             |
| EndTabBar()                          | 无                                                                                                                                                                    | 结束标签栏                                 |
| TabBarLayout() [Internal]            | 无                                                                                                                                                                    | 内部函数：标签栏布局                       |
| TabBarCalcTabID() [Internal]         | `tab_name`: 标签名<br>`tab_flags`: 标签标志                                                                                                                                  | 内部函数：计算标签ID                       |
| TabBarCalcMaxTabWidth() [Internal]   | 无                                                                                                                                                                    | 内部函数：计算最大标签宽度                 |
| TabBarFindTabById() [Internal]       | `tab_id`: 标签ID                                                                                                                                                       | 内部函数：根据标签ID查找标签               |
| TabBarFindTabByOrder() [Internal]    | `order`: 标签顺序                                                                                                                                                       | 内部函数：根据顺序查找标签                 |
| TabBarFindMostRecentlySelectedTabForActiveWindow() [Internal] | 无                                                                                                                                                                    | 内部函数：查找最近选中的标签               |
| TabBarGetCurrentTab() [Internal]     | 无                                                                                                                                                                    | 内部函数：获取当前标签                     |
| TabBarGetTabName() [Internal]        | `tab_id`: 标签ID                                                                                                                                                       | 内部函数：获取标签的名称                   |
| TabBarAddTab() [Internal]            | `tab_id`: 标签ID<br>`label`: 标签                                                                                                                                      | 内部函数：添加标签                         |
| TabBarRemoveTab() [Internal]         | `tab_id`: 标签ID                                                                                                                                                       | 内部函数：移除标签                         |
| TabBarCloseTab() [Internal]          | `tab_id`: 标签ID                                                                                                                                                       | 内部函数：关闭标签                         |
| TabBarScrollClamp() [Internal]       | `tab_id`: 标签ID<br>`tab_width`: 标签宽度                                                                                                                                 | 内部函数：标签滚动到最小位置               |
| TabBarScrollToTab() [Internal]       | `tab_id`: 标签ID                                                                                                                                                       | 内部函数：滚动到指定标签位置               |
| TabBarQueueFocus() [Internal]        | `tab_id`: 标签ID                                                                                                                                                       | 内部函数：排队焦点到标签                   |
| TabBarQueueReorder() [Internal]      | `tab_id`: 标签ID<br>`order`: 新顺序                                                                                                                                     | 内部函数：重排标签顺序                     |
| TabBarProcessReorderFromMousePos() [Internal] | `mouse_pos`: 鼠标位置                                                                                                                                                  | 内部函数：处理鼠标位置的标签重新排序       |
| TabBarProcessReorder() [Internal]    | `tab_id`: 标签ID<br>`new_order`: 新顺序                                                                                                                                 | 内部函数：处理标签的重新排序               |
| TabBarScrollingButtons() [Internal]  | 无                                                                                                                                                                    | 内部函数：标签栏的滚动按钮                 |
| TabBarTabListPopupButton() [Internal] | 无                                                                                                                                                                    | 内部函数：标签列表的弹出按钮               |
| BeginTabItem()                       | `label`: 标签<br>`flags`: 标签标志                                                                                                                                         | 开始一个标签项                             |
| EndTabItem()                         | 无                                                                                                                                                                    | 结束标签项                                 |
| TabItemButton()                      | `label`: 标签<br>`flags`: 标签标志                                                                                                                                         | 创建一个标签按钮                           |
| TabItemEx() [Internal]               | `label`: 标签<br>`flags`: 标签标志                                                                                                                                         | 内部函数：扩展标签项                       |
| SetTabItemClosed()                   | `label`: 标签                                                                                                                                                           | 关闭一个标签项                             |
| TabItemCalcSize() [Internal]         | `label`: 标签<br>`flags`: 标签标志                                                                                                                                         | 内部函数：计算标签大小                     |
| TabItemBackground() [Internal]       | 无                                                                                                                                                                    | 内部函数：设置标签项背景                   |
| TabItemLabelAndCloseButton() [Internal] | `label`: 标签<br>`flags`: 标签标志                                                                                                                                         | 内部函数：标签项的标签和关闭按钮           |





# APP UI 第二版本评估与实施文档

版本：V0.5  
日期：2026-05-03  
设计输入：`docs/UI/APP_UI_第二版本.xlsx`

## 1. 结论

第二版 UI 方案整体方向是可取的，建议作为下一版正式交互方案推进。

相比第一版“HOME / ENV / ACT / ALARM / SETTING 多一级页面环切”的结构，第二版把工作态聚合为一个 `HOME` 中心页面，再通过 `灯光 / 热管理 / 水泵 / 氧气泵 / 系统设置 / 采样参数` 六个 tab 进入具体控制。这更符合 128x64 OLED + 5 按键设备的实际使用方式：用户先看到设备功能总览，再进入某个模块做控制，路径更短，认知负担也更低。

目前 7 条实施规则已确认，并已同步修正 Excel：

| 项 | 原问题 | 已确认处理 |
| --- | --- | --- |
| 1 | 6 个 tab 但屏幕一次只能显示 4 个 tab | 引入 `tabIndex + visibleStart`，左右移动时自动滚动窗口 |
| 2 | Excel 中 `Right` 误写为向左选中 tab | 已修正为 `Right 向右选中 tab` |
| 3 | 长按 Enter 退出 tab 与现有系统开关机逻辑冲突 | `Enter Long` 保留给系统开关机，改用 `Left Long` 退出 tab |
| 4 | tab 内 Left/Right 切状态是否立即生效不明确 | 执行器状态即时切换；系统设置只改缓存，保存后写入 |
| 5 | 图标、功率、RGB、热管理、ADC 字段缺少统一取数口径 | 实施前建立统一 UI 数据快照结构 |
| 6 | `P50_ERROR` 只写“按照现有 MAIN1 和 MAIN2” | 推荐采用 `ERROR + CODE + MOD + DESC + ACT` 固定错误页模板 |
| 7 | tab 卡片内文字可能超出 32px 宽度 | 横向内容自动滚动显示；纵向内容通过 `Up/Down` 选择，不做纵向自动滚动 |

综合建议：不要推翻当前 `MD_Display` 已有框架，而是在现有页面上下文、统一绘制入口、脏区刷新和按键分发基础上，把工作态重构为“P20_HOME 容器 + tab 卡片激活模式”。

## 2. 第二版设计摘要

### 2.1 状态页

| 页面 | 状态归属 | 设计意图 | 备注 |
| --- | --- | --- | --- |
| `P00_INIT` | `DS_INIT` | 初始化页，显示项目名、版本、编译时间和初始化动画 | 建议停留 500~1200ms |
| `P10_BOOTING` | `DS_BOOTING` | 启动自检页，显示加载动画 | 可保留三段点动画 |
| `P20_HOME` | `DS_WORK` | 工作态中心页，展示 6 个 tab | 一屏仅显示 4 个 tab |
| `P30_CLOSING` | `DS_CLOSING` | 关机中页，显示关闭动画 | 建议停留 800~1500ms |
| `P40_SLEEP` | `DS_SHUT_DOWN` | 息屏页 | 第一个唤醒键不执行功能 |
| `P50_ERROR` | `DS_ERR` | 系统错误页 | 优先级高于普通工作页 |

### 2.2 工作态 tab

| tab | 内容 | tab 内交互 |
| --- | --- | --- |
| 灯光 | 灯光图标、总功率、白光照明状态、RGB 模式状态 | `Up/Down` 选项目，`Left/Right` 切状态 |
| 热管理 | 热管理图标、总功率、加热模式、散热模式 | `Up/Down` 选项目，`Left/Right` 切状态 |
| 水泵 | 水泵图标、总功率、水泵模式 | `Left/Right` 切状态 |
| 氧气泵 | 氧气泵图标、总功率、氧气泵模式 | `Left/Right` 切状态 |
| 系统设置 | `SLEEP / BUZZER / HIGH BL / LOW BL` | `Up/Down` 选项目，`Left/Right` 改缓存，`Right Long` 保存 |
| 采样参数 | `tAdcSamp` 结构体数据 | 建议分页或滚动显示 |

## 3. 方案优点

1. **用户路径更短**  
   高频控制集中到 `HOME` tab，用户不需要在多个一级页面之间循环找功能。

2. **很适合 128x64 单色屏**  
   一个单元格对应 8x8 像素格，16x8 网格概念清晰，能直接转成 U8g2 坐标。

3. **与硬件模块天然对应**  
   tab 与灯光、热管理、水泵、氧气泵、设置、ADC 采样一一对应，后续测试和联调也更容易按模块拆分。

4. **降低首页信息拥挤度**  
   第一版首页同时展示温度、灯光、泵、告警、倒计时等信息，容易变成数据列表。第二版可以把 `P20_HOME` 作为 6 个 tab 的容器，每个 `P20_HOME_xxx` 工作表定义对应 tab 卡片里的实际显示内容。

5. **更利于后续扩展**  
   以后新增喂食、换水、校准等模块，可以继续作为 tab 扩展，而不是增加新的一级页面。

## 4. 主要问题与优化建议

### 4.1 6 tab 超出一屏

Excel 中说明“一屏只能显示四个 tab”，但实际 tab 有六个。实施时需要明确滚动规则：

| 当前选中 | 可见窗口 | 屏幕显示 |
| --- | --- | --- |
| 0 灯光 | 0~3 | 灯光、热管理、水泵、氧气泵 |
| 1 热管理 | 0~3 | 灯光、热管理、水泵、氧气泵 |
| 2 水泵 | 0~3 | 灯光、热管理、水泵、氧气泵 |
| 3 氧气泵 | 0~3 | 灯光、热管理、水泵、氧气泵 |
| 4 系统设置 | 2~5 | 水泵、氧气泵、系统设置、采样参数 |
| 5 采样参数 | 2~5 | 水泵、氧气泵、系统设置、采样参数 |

建议在顶部标题显示 `HOME 3/6`，并在左右边缘显示很小的 `<` / `>` 滚动提示。

### 4.2 长按 Enter 冲突

现有项目里 `Enter Long` 已用于系统开关机。第二版 Excel 写“长按 Enter 退出当前 tab”，这会带来两个风险：

- 用户想退出 tab，结果触发关机。
- 如果 UI 层抢占 `Enter Long`，底层系统开关机语义会被破坏。

建议最终规则如下：

| 按键 | HOME 容器/选中态 | tab 激活态 |
| --- | --- | --- |
| `Enter Short` | 进入当前 tab | 确认/执行当前项 |
| `Enter Long` | 系统开关机 | 系统开关机 |
| `Left Long` | 回到第一个 tab 或保持 HOME | 退出 tab，返回 HOME 总览 |
| `Left/Right Short` | 选择 tab | 调整当前项 |
| `Up/Down Short` | 无动作 | 选择 tab 内项目 |

本方案已按此规则修正 Excel：`Enter Long` 始终保留系统开关机，tab 激活态统一使用 `Left Long` 退出。

### 4.3 HOME tab 卡片内容已承担状态摘要

这里需要修正前一版文档的说法：你的设计里 `P20_HOME` 本身是 6 个 tab 的容器布局，`P20_HOME_灯光 / 热管理 / 水泵 / 氧气泵 / 系统设置 / 采样参数` 是每个 tab 卡片中的实际内容模板。因此不需要再额外增加一层“HOME 状态摘要页”。

更准确的实现理解是：

- `P20_HOME` 负责顶栏、4 个可见 tab 区域、`tabIndex + visibleStart` 滚动窗口和当前选中 tab。
- `P20_HOME_xxx` 负责该 tab 在 32x56 卡片里的显示内容。
- `Enter Short` 不是跳到独立全屏详情页，而是让当前 tab 从“选中态”进入“激活态”。
- 激活态下，`Up/Down` 在当前 tab 卡片内部选择控制项，`Left/Right` 调整当前项，`Left Long` 退出激活态回到 HOME 选中态。

所以后续优化重点不是“再加摘要”，而是把每个 `P20_HOME_xxx` 卡片里的状态字段定义得更短、更稳定、更能塞进 32x56。
新增滚动原则：

- 横向溢出：任一 tab 卡片内的标签、状态、数值超出当前字段宽度时，自动水平滚动显示。
- 纵向溢出：不做自动纵向滚动，统一通过 `Up/Down` 切换当前控制项、字段或设置项。
- 只有当前选中字段或激活 tab 内的焦点字段允许横向滚动；非焦点字段建议截断或显示短缩写，避免整屏同时动。
- 水平滚动建议延迟 600~800ms 后启动，步进 1px 或 1 字符，末尾停留 600~800ms 后循环。

### 4.4 tab 卡片模板需要统一

建议每个 tab 卡片按 32x56 的相对坐标统一分区：

| 卡片区域 | 相对像素 | 内容 |
| --- | --- | --- |
| 头部 | `0~15` | 图标或模块短名 + 总功率/关键值 |
| 项目 1 | `16~31` | 第一项标签 + 当前状态 |
| 项目 2 | `32~47` | 第二项标签 + 当前状态，单项 tab 可留作大状态 |
| 焦点/提示 | `48~55` | 激活态焦点、横向滚动标记或短提示 |

这样 `P20_HOME_灯光 / 热管理 / 水泵 / 氧气泵 / 采样参数` 都比较容易落地。`P20_HOME_系统设置` 当前有 `SLEEP / BUZZER / HIGH BL / LOW BL` 四项，32x56 卡片内会偏挤，建议垂直方向通过 `Up/Down` 选择当前项；横向状态值若超出字段宽度，再对焦点字段做水平滚动显示，避免 4 项全塞导致文字重叠。

### 4.5 系统设置必须使用编辑缓存

系统设置 tab 内的 `SLEEP / BUZZER / HIGH BL / LOW BL` 不建议 `Left/Right` 一改就写 Flash。建议沿用当前第一版已有的设置缓存逻辑：

- 进入设置 tab 时加载当前参数到缓存。
- `Left/Right` 只改缓存。
- `Enter Short` 确认当前项或切换编辑态。
- `Right Long` 或专用保存动作保存全部缓存。
- 退出未保存时保留提示 `CACHE` 或自动丢弃缓存。

### 4.6 采样参数需要分组

`P20_HOME_采样参数` 只写“显示 tAdcSamp 结构体里面的数据”，实现粒度不够。`tAdcSamp` 字段较多，一屏无法完整展示。

建议分成 3 组：

| 分组 | 字段示例 | 显示策略 |
| --- | --- | --- |
| 温度 | 水温 1、水温 2、5V 板温、12V 板温 | 默认第一页 |
| 电源 | 12V 电压、灯光电流、关键 ADC 原值 | 第二页 |
| 光照 | 光敏 ADC、换算值、采样状态 | 第三页 |

`Up/Down` 在组内移动，`Left/Right` 切分组。若该页主要给研发使用，可以在 HOME 默认隐藏，或放到最后一个 tab。

### 4.7 错误页推荐方案

`P50_ERROR` 不建议继续使用“按照现有 MAIN1 和 MAIN2”这种内部描述。推荐固定为用户可理解、工程也容易映射的 4 行模板：

```text
ERROR
CODE  003 HIGH
MOD   SYSTEM
DESC  INPUT UNDER
ACT   CHECK VIN
```

推荐规则：

- `CODE` 显示错误码和等级，如 `003 HIGH`、`012 WARN`。
- `MOD` 显示故障模块，如 `SYSTEM / LIGHT / HEAT / WPUMP / O2PUMP / ADC`。
- `DESC` 显示短错误描述，限制在 12~16 个 ASCII 字符内。
- `ACT` 显示建议动作，如 `CHECK VIN`、`CHECK NTC`、`REDUCE LOAD`。
- 严重错误覆盖普通 HOME/tab；错误解除后返回来源 tab，来源无效则回 HOME 总览。
- `Enter Short` 用于确认/消音，`Enter Long` 仍保留系统开关机。

这样比 MAIN1/MAIN2 更适合实机：用户能直接知道哪里错、严重程度和下一步动作；代码侧则可以继续复用现有 `usDisp_ErrCodeDisplay()` 和错误文本映射表。

## 5. 推荐页面模型

### 5.1 页面枚举

保留现有状态页枚举：

```c
typedef enum
{
    DPI_NONE = 0,
    DPI_INIT,
    DPI_BOOTING,
    DPI_HOME,
    DPI_UPGRADE,
    DPI_CLOSING,
    DPI_SLEEP,
    DPI_ERROR,
} DispPageId_E;
```

第一版里的 `DPI_ENV / DPI_ACT / DPI_ALARM / DPI_SETTING / DPI_QUICK` 可以先保留，避免一次性大改；第二版工作态默认只进入 `DPI_HOME`，并通过 HOME 上下文区分“选中态”和“tab 激活态”。

### 5.2 HOME 内部模型

建议新增：

```c
typedef enum
{
    DHT_LIGHT = 0,
    DHT_HEAT,
    DHT_WPUMP,
    DHT_O2PUMP,
    DHT_SETTING,
    DHT_ADC,
    DHT_COUNT,
} DispHomeTabId_E;

typedef enum
{
    DHM_OVERVIEW = 0,
    DHM_TAB_ACTIVE,
} DispHomeMode_E;
```

建议扩展 `DispPageCtx_T`：

```c
typedef struct
{
    DispPageId_E ePageId;
    DispPageId_E ePrevPageId;
    DispHomeMode_E eHomeMode;
    DispHomeTabId_E eHomeTab;
    u8 ucTabVisibleStart;
    u8 ucTabItemIndex;
    u8 ucAdcGroupIndex;
    bool bEditing;
    vu16 usDirtyMask;
} DispPageCtx_T;
```

这样可以保持状态机仍然是 `DS_WORK -> DPI_HOME`，但 UI 交互已经变成第二版 tab 模式。

## 6. 推荐代码落点

| 文件 | 当前职责 | 第二版建议动作 |
| --- | --- | --- |
| `Hardware/MD_Display/md_display_task.h` | 页面、焦点、上下文枚举 | 新增 HOME tab 枚举、HOME 模式、tab 窗口字段 |
| `Hardware/MD_Display/md_display_task.c` | 页面切换、按键分发、设置缓存 | 将工作态按键从一级页面环切改为 HOME tab 选择与 tab 激活态控制 |
| `Hardware/MD_Display/md_display_api.c` | U8g2 绘制、数据快照 | 新增 P20_HOME 容器绘制和 6 个 tab 卡片绘制函数 |
| `md_display_queue_task_work.c` | 工作态刷新入口 | 继续调用统一渲染入口，不需要大改 |
| `md_display_queue_task_init.c` | 初始化显示 | 按第二版补项目名、版本、编译时间 |
| `md_display_queue_task_booting.c` | 启动显示 | 简化为 BOOTING 动画，保留进度数据 |
| `md_display_queue_task_err.c` | 错误显示 | 使用固定错误页模板 |
| `md_display_queue_task_closing.c` | 关机显示 | 使用 CLOSING 动画模板 |

### 6.1 图标资源说明

第二版 UI 的 6 个 tab 已有对应图标资源，建议作为正式实施输入：

| tab | PNG 源文件 | C 数组名 |
| --- | --- | --- |
| 灯光 | `docs/UI/Icon/灯光.png` | `icon_light_16x16` |
| 热管理 | `docs/UI/Icon/热管理.png` | `icon_heat_16x16` |
| 水泵 | `docs/UI/Icon/水泵.png` | `icon_wpump_16x16` |
| 氧气泵 | `docs/UI/Icon/氧气泵.png` | `icon_o2pump_16x16` |
| 系统设置 | `docs/UI/Icon/系统设置.png` | `icon_setting_16x16` |
| 采样参数 | `docs/UI/Icon/采样参数.png` | `icon_adc_16x16` |

生成文件：`docs/UI/Icon/icon_bitmaps.h`。  
生成脚本：`docs/UI/Icon/png_to_u8g2.py`。

使用规则：

```c
u8g2_DrawXBMP(&u8g2, x, y, 16, 16, icon_light_16x16);
```

注意事项：

- 图标尺寸统一为 `16x16`，每个图标 32 字节。
- `icon_bitmaps.h` 已按当前 U8g2 的 `u8g2_DrawXBMP()` 要求生成，位序为 `LSB first`，即 bit0 对应最左侧像素。
- 正式编码时建议只在一个显示绘制源文件中包含图标数组，避免多个 C 文件重复生成静态副本。
- 若 Keil/EIDE 未将 `docs/UI/Icon` 加入 include path，建议实施时把图标头文件迁移或复制到 `Hardware/MD_Display` 下，例如 `md_display_icon.h`。

## 7. 数据源映射建议

| UI 字段 | 推荐数据源 | 缺口 |
| --- | --- | --- |
| 软件版本号 | `boardSOFTWARE_VERSION` | 已有 |
| 硬件版本号 | `boardHARDWARE_VERSION` | 确认是否已定义 |
| 编译时间 | `tAppMemParam.tVerInfo.saBuildDate / saBuildTime` | 已有 |
| 白光状态 | `tLight.eWordMode` | 已有 |
| 白光功率/电流 | `tAdcSamp.fLightCurr` 或灯光模块功率估算 | 需要确认单位 |
| RGB 模式 | RGB 控制状态 | 当前代码未看到明确数据源，需要补接口 |
| 加热模式 | `bHeat_IsUiForceOn()`、温度策略 | 已有基础 |
| 散热模式 | `eFan_GetWorkMode()` | 已有基础 |
| 水泵模式 | `tPump.eMode / tPump.usSpeed` | 已有 |
| 氧气泵模式 | `tO2Pump.eMode / tO2Pump.usSpeed` | 已有 |
| 息屏时间 | `tAppMemParam.tDISP.usAutoOffTime` | 已有 |
| 蜂鸣器 | `tAppMemParam.tSYS.bBuzSwitchOff` | 已有 |
| 高低亮度 | `tAppMemParam.tDISP.ucHighLightValue / ucLowLightValue` | 已有 |
| 采样参数 | `tAdcSamp` | 需要字段分组和显示顺序 |
| 错误码 | `usDisp_ErrCodeDisplay()` | 已有 |


## 8. 滚动与焦点规则

| 类型 | 触发条件 | 推荐行为 |
| --- | --- | --- |
| tab 窗口滚动 | `tabIndex` 移到当前 4-tab 可见窗口外 | 更新 `visibleStart`，保证当前 tab 可见 |
| 横向文本滚动 | 当前焦点字段文本宽度超过字段宽度 | 延迟启动水平滚动，只滚动当前焦点字段 |
| 横向非焦点文本 | 非焦点字段文本宽度超过字段宽度 | 优先短缩写，其次截断，避免多处同时滚动 |
| 纵向内容超出 | 当前 tab 项目数超过可见行数 | 使用 `Up/Down` 移动焦点，必要时移动可见项目窗口 |
| 系统设置超出 | `SLEEP / BUZZER / HIGH BL / LOW BL` 无法全部稳定显示 | 显示当前 2 项或当前项摘要，`Up/Down` 选择其余项 |
| 采样字段超出 | `tAdcSamp` 字段过多 | 先按温度/电源/光照分组，组内用 `Up/Down` 选择字段 |

实现建议：新增一个轻量文本滚动状态，例如 `DispTextScroll_T`，至少包含 `focusId / offset / holdCnt / textHash`。当焦点变化或文本变化时重置滚动；当文本宽度小于字段宽度时 offset 固定为 0。

## 9. 分阶段实施计划

建议按 4 个阶段推进。因为第一版 UI 框架已经较完整，第二版主要是工作态交互重构，预估总工期为 4~6 人天。

### 阶段 A：设计定稿与数据模型

工期：0.5~1 人天

任务：

- Excel 中 HOME 的 `Right` 描述已修正为向右选中 tab。
- `Enter Long` 已确认保留系统开关机，tab 退出使用 `Left Long`。
- 确认 6 个 tab 的顺序、短名、图标和核心状态字段。
- 确认各模式枚举：白光、RGB、加热、散热、水泵、氧气泵。
- 确认采样参数字段分组。

完成标准：

- 可以生成 `DispHomeTabId_E` 和每个 tab 的字段表。
- 不再存在“某个状态从哪里来”的不确定项。

### 阶段 B：P20_HOME 容器与 tab 卡片

工期：1~1.5 人天

任务：

- 扩展 `DispPageCtx_T`，增加 `eHomeMode / eHomeTab / ucTabVisibleStart / ucTabItemIndex`。
- 实现 `v_disp_draw_home_container()` 和 6 个 `v_disp_draw_home_tab_xxx()` 卡片绘制函数。
- 实现 4 可见 tab + 6 tab 滚动窗口。
- 实现 HOME 总览按键：
  - `Left/Right Short` 选择 tab。
  - `Enter Short` 进入当前 tab 激活态。
  - `Up/Down Short` 无动作。
  - 息屏下首个短按只亮屏。

完成标准：

- 六个 tab 都能选中。
- 第 5、6 个 tab 可通过滚动窗口显示。
- HOME 上能看到每个 tab 的模块名和核心状态。

### 阶段 C：tab 激活态交互

工期：1.5~2 人天

任务：

- 复用 6 个 tab 卡片绘制函数，并补齐激活态焦点/编辑绘制：
  - `v_disp_draw_tab_light()`
  - `v_disp_draw_tab_heat()`
  - `v_disp_draw_tab_wpump()`
  - `v_disp_draw_tab_o2pump()`
  - `v_disp_draw_tab_setting()`
  - `v_disp_draw_tab_adc()`
- 实现 tab 内焦点项。
- 实现 tab 内按键：
  - `Up/Down Short` 选择项目。
  - `Left/Right Short` 调整状态或切换分组。
  - `Enter Short` 确认/执行。
  - `Left Long` 返回 HOME 总览。
- 设置 tab 接入缓存编辑与保存策略。
- 采样 tab 实现分组分页。

完成标准：

- 每个 tab 都能进入、操作、退出。
- 设置项不会因左右调整而频繁写 Flash。
- 文本不越界、不重叠。

### 阶段 D：状态页与异常闭环

工期：1~1.5 人天

任务：

- 按第二版优化 `INIT / BOOTING / CLOSING` 动画和文案。
- 按 `ERROR + CODE + MOD + DESC + ACT` 模板补齐 `P50_ERROR`。
- 确认 `DS_ERR` 覆盖优先级高于 HOME/tab。
- 错误解除后返回来源 tab；来源无效则回 HOME 总览。
- 调整刷新脏区，减少 tab 状态变化时闪烁。

完成标准：

- 状态流 `INIT -> BOOTING -> HOME -> CLOSING -> SLEEP` 正常。
- 错误覆盖和恢复路径可验证。
- 工作态数据刷新稳定，tab 切换无明显闪烁。

## 10. 验收清单

### 9.1 页面流

| 编号 | 测试项 | 期望结果 |
| --- | --- | --- |
| UI2-P01 | 上电进入 INIT | 显示项目名、版本、编译时间、初始化动画 |
| UI2-P02 | INIT 后进入 BOOTING | 显示 BOOTING 动画 |
| UI2-P03 | BOOTING 后进入 HOME | 显示 4 个可见 tab |
| UI2-P04 | HOME 右移到第 5、6 个 tab | 可见窗口自动滚动 |
| UI2-P05 | Enter 进入 tab | 当前 tab 进入激活态，可在卡片内选项和调整 |
| UI2-P06 | Left Long 退出 tab | 退出激活态，返回 HOME 容器选中态 |
| UI2-P07 | 长按关机 | 进入 CLOSING，再进入 SLEEP |

### 9.2 交互流

| 编号 | 测试项 | 期望结果 |
| --- | --- | --- |
| UI2-K01 | HOME 下 Up/Down | 无误触发 |
| UI2-K02 | 灯光 tab 调整白光状态 | 状态按枚举循环，UI 立即反馈 |
| UI2-K03 | 热管理 tab 调整加热/散热 | 状态按枚举循环，UI 立即反馈 |
| UI2-K04 | 水泵 tab 调整模式 | 模式循环，速度/状态刷新 |
| UI2-K05 | 氧气泵 tab 调整模式 | 模式循环，速度/状态刷新 |
| UI2-K06 | 设置 tab 修改亮度 | 只改缓存，保存后才持久化 |
| UI2-K07 | 采样 tab 切组 | 温度/电源/光照分组可切换 |

### 9.3 异常流

| 编号 | 测试项 | 期望结果 |
| --- | --- | --- |
| UI2-E01 | HOME 触发严重错误 | 立即覆盖到 ERROR |
| UI2-E02 | tab 内触发严重错误 | 立即覆盖到 ERROR，并记录来源 tab |
| UI2-E03 | 错误解除 | 返回来源 tab 或 HOME |
| UI2-E04 | 息屏后任意短按 | 只亮屏，不执行原按键功能 |
| UI2-E05 | 升级/关机状态下按键 | 普通 UI 操作不生效 |

## 11. 风险与应对

| 风险 | 影响 | 应对 |
| --- | --- | --- |
| 长按 Enter 语义冲突 | 可能误关机或破坏系统开关机 | 默认保留系统开关机，使用 Left Long 退出 tab |
| 六 tab 滚动未定义 | 第 5、6 个 tab 不可达或焦点混乱 | 实现 `ucTabVisibleStart` 并写验收用例 |
| tab 内模式枚举不完整 | 控制行为无法落地 | 实施前补全模式枚举与数据源 |
| 采样参数过多 | 屏幕信息拥挤 | 分组分页，不一屏塞满 |
| 设置项频繁写 Flash | Flash 寿命和交互卡顿 | 使用编辑缓存，确认后保存 |
| 图标资源接入路径不明确 | 编译找不到头文件或重复生成静态副本 | 使用 `docs/UI/Icon/icon_bitmaps.h` 作为设计输入，实施时迁移到 `Hardware/MD_Display` 或加入 include path，并只在一个绘制源文件中包含 |
| 当前第一版代码已成型 | 大改容易引入回归 | 保留现有状态页和刷新框架，只重构 `DPI_HOME` 工作态 |

## 12. 建议最终交互规则

| 场景 | Enter Short | Enter Long | Left Short | Right Short | Up Short | Down Short | Left Long |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 息屏 | 亮屏 | 系统开关机 | 亮屏 | 亮屏 | 亮屏 | 亮屏 | 亮屏 |
| HOME 总览 | 进入 tab | 系统开关机 | 上一个 tab | 下一个 tab | 无 | 无 | 回第 1 个 tab |
| 灯光 tab | 执行当前项 | 系统开关机 | 上一个状态 | 下一个状态 | 上一项 | 下一项 | 返回 HOME |
| 热管理 tab | 执行当前项 | 系统开关机 | 上一个状态 | 下一个状态 | 上一项 | 下一项 | 返回 HOME |
| 水泵 tab | 执行/切换 | 系统开关机 | 上一个模式 | 下一个模式 | 无 | 无 | 返回 HOME |
| 氧气泵 tab | 执行/切换 | 系统开关机 | 上一个模式 | 下一个模式 | 无 | 无 | 返回 HOME |
| 系统设置 tab | 进入/确认编辑 | 系统开关机 | 减小/上一选项 | 增大/下一选项 | 上一项 | 下一项 | 返回 HOME |
| 采样参数 tab | 切换显示细节 | 系统开关机 | 上一组 | 下一组 | 上一字段 | 下一字段 | 返回 HOME |
| ERROR | 确认/消音 | 系统开关机 | 无 | 无 | 无 | 无 | 无 |

## 13. 结论建议

第二版 UI 建议推进，但应按“设计修正 -> P20_HOME 容器与 tab 卡片 -> 横向滚动/纵向焦点规则 -> tab 激活态交互 -> 状态页和异常闭环”的顺序落地。

最重要的工程策略是：**保留现有 `MD_Display` 状态页、统一渲染和刷新框架，只把 `DS_WORK` 下的工作态页面重构为第二版 tab 模式**。这样既能获得第二版更清晰的用户体验，也能把改动范围控制在可联调、可回退的范围内。

## 14. 当前实现进度记录（2026-05-03）

本节用于记录截至 2026-05-03 晚间的实际落地状态，避免后续继续实施时遗忘当前完成度和代码落点。

### 14.1 已完成

#### A. HOME 容器与内部上下文

- 已在 `Hardware/MD_Display/md_display_task.h` 中新增 `DispHomeTabId_E`、`DispHomeMode_E`。
- 已在 `DispPageCtx_T` 中新增：
   - `eHomeMode`
   - `eHomeTab`
   - `ucTabVisibleStart`
   - `ucTabItemIndex`
   - `ucAdcGroupIndex`
   - `DispTextScroll_T tTextScroll`
   - `DispHomeRestore_T tHomeRestore`
- 当前工作态仍保持 `DS_WORK -> DPI_HOME`，但交互模型已切换为第二版 HOME/tab 模式。

#### B. HOME 总览与 tab 激活态按键规则

- 已在 `Hardware/MD_Display/md_display_task.c` 实现：
   - `Left/Right Short`：HOME 总览下切换 tab
   - `Enter Short`：进入当前 tab 激活态
   - `Up/Down Short`：tab 激活态下切换项目
   - `Left Long`：退出 tab 激活态或回第一个 tab
   - `Right Long`：在设置 tab 中保存缓存
- `Enter Long` 仍保留给系统开关机，没有被 UI 抢占。

#### C. HOME 六个 tab 卡片绘制

- 已在 `Hardware/MD_Display/md_display_api.c` 完成 `P20_HOME` 容器绘制。
- 已实现 4 个可见卡片窗口和 6 个 tab 滚动显示。
- 已接入 6 个 tab 图标资源：
   - `LIGHT`
   - `HEAT`
   - `WPUMP`
   - `O2PUMP`
   - `SETTING`
   - `ADC`
- 已实现当前选中卡片与激活态卡片的不同边框/反显效果。

#### D. tab 内部基础交互

- `LIGHT`：已接入白光模式切换，卡片显示功率/模式摘要。
- `HEAT`：
   - 第一项已接入加热 UI 强制开关。
   - 第二项已接入风扇档位人工切换，不再只是提示语。
- `WPUMP`：已接入模式循环切换。
- `O2PUMP`：已接入模式循环切换。
- `SETTING`：
   - 已接入 `SLEEP / BUZZER / HIGH BL / LOW BL` 四项缓存编辑。
   - 已实现 2 项可见窗口 + `Up/Down` 滚动选择。
   - 已实现 `Right Long` 保存。
- `ADC`：
   - 已按温度 / 电源 / 光照三组分页。
   - 已实现 `Left/Right` 切组、`Up/Down` 切字段。

#### E. 横向滚动 / 截断策略

- 已在 `Hardware/MD_Display/md_display_task.h` 和 `Hardware/MD_Display/md_display_queue_task_work.c` 中实现轻量滚动状态。
- 当前行为如下：
   - 焦点字段超宽时，延时后自动横向滚动。
   - 非焦点字段超宽时，不滚动，直接截断显示。
   - 切 tab、切焦点、切页面时滚动状态会自动复位。

#### F. 状态页模板

- 已在 `Hardware/MD_Display/md_display_api.c` 中完成以下状态页模板收口：
   - `P00_INIT`
   - `P10_BOOTING`
   - `P30_CLOSING`
   - `P40_SLEEP`
   - `P50_ERROR`
- `P50_ERROR` 已采用固定模板：
   - `ERROR`
   - `CODE`
   - `MOD`
   - `DESC`
   - `ACT`

#### G. 错误覆盖与恢复

- `DS_ERR` 已覆盖普通 HOME/tab。
- 错误页 `Enter Short` 已接入确认/消音处理。
- 已记录 HOME 来源上下文，错误解除后可恢复到来源 HOME tab / 模式 / 焦点，而不是仅恢复到 `DPI_HOME` 页面。

#### H. 编译状态

- 当前工程已可稳定编译通过。
- 参考构建结果：
   - `build/APP/uv4_build.log`
   - EIDE builder 输出
- 当前最新构建结果为：`0 Error(s), 0 Warning(s)`。

### 14.2 已额外修复的工程问题

- 修复了 `Hardware/MD_HeatManage/md_hm_task.c` 中原有的语法残缺和重复代码问题。
- 补齐了热管理相关 UI 接口：
   - `bHeat_SetUiForce()`
   - `bHeat_ToggleUiForce()`
   - `bHeat_IsUiForceOn()`
   - `bFan_CycleUiMode()`
   - `bFan_IsUiOverride()`

### 14.3 尚未完全完成

以下内容仍建议继续完善，但不影响当前第二版 UI 主流程联调：

1. **状态页动画细化**
    - `INIT / BOOTING / CLOSING` 目前已是第二版模板，但动画仍偏简化。

2. **统一 UI 数据快照进一步抽象**
    - 当前 `md_display_api.c` 已集中采集数据，但还没有完全抽成一套独立、可维护的正式 UI snapshot 层。

3. **灯光 tab 的 RGB 数据源**
    - 当前卡片中 RGB 相关状态仍偏摘要展示，后续若有明确 RGB 控制状态源，建议继续接入。

4. **异常流实机验证**
    - 虽然代码已补到错误覆盖/恢复，但还没有完成逐项板级验收。

5. **文档第 10 节验收清单尚未逐条打勾**
    - 当前属于“代码完成度较高”，但不等于“实机验收全部完成”。

### 14.4 建议后续继续顺序

建议后续继续按以下顺序推进：

1. 烧录实机验证 HOME/tab 基本交互。
2. 验证错误覆盖与恢复路径。
3. 细化 `INIT / BOOTING / CLOSING` 动画。
4. 如有需要，再补灯光 RGB 的正式数据源与显示细节。

### 14.5 当前结论

截至 2026-05-03，**第二版 UI 的核心工作态框架、HOME tab 交互、设置/采样交互、横向滚动策略、错误页模板和错误恢复主路径已经完成，且工程已通过编译**。

因此，当前状态可以定义为：

- **代码层：第二版 UI 主流程已基本落地**
- **联调层：可进入实机验证阶段**
- **验收层：尚未完成全部板级验证与动画打磨**

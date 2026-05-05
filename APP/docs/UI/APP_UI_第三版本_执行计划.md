# APP UI 第三版本执行计划

文档状态: 待评审  
设计输入: `docs/UI/APP_UI_第三版本_修正版.xlsx`  
适用模块: `Hardware/MD_Display`  
目标屏幕: `128x64` OLED, 顶部 `128x8` 菜单栏, 内容区 `128x56`

## 1. 背景

第二版本采用 `HOME + 6 个功能 tab` 的结构, 每个功能 tab 在 `32x56` 小区域内显示摘要和控制项。实机屏幕只有 `128x64`, 单个模块可用区域过小, 容易出现字段显示不完整、状态文字滚动频繁、焦点不直观等问题。

第三版本改为“主页面 + 模块详情页”的结构:

- `P20_HOME`: 工作总览页, 用完整 `128x56` 内容区展示四个核心模块摘要。
- `P25_SETTING`: 系统设置主页面, 用完整 `128x56` 内容区编辑设置缓存。
- `P26_ADC`: 采样参数主页面, 按温度组、电源组、光照组分页显示。
- `P21_LIGHT / P22_HEAT / P23_WPUMP / P24_O2PUMP`: 从 HOME 进入的四个模块详情页, 每个详情页独占完整内容区。

本次执行计划的核心不是新增一套显示框架, 而是在保留现有 `MD_Display` 任务、U8g2 绘制入口、按键分发和脏区刷新机制的基础上, 将工作态 UI 从第二版小 tab 模式重构为第三版大页面模式。

## 2. 总体目标

1. 工作态默认进入 `P20_HOME` 总览页。
2. `Left/Right Short` 在 `HOME / SET / ADC` 三个主页面之间循环切换。
3. `P20_HOME` 内通过 `Up/Down Short` 选择 `LIGHT / HEAT / WPUMP / O2PUMP`, `Enter Short` 进入对应详情页。
4. 模块详情页内使用完整 `128x56` 内容区显示和控制模块状态。
5. `Enter Long` 始终保留系统开关机语义。
6. 模块详情页中 `Left Long` 返回 `P20_HOME`。
7. 系统设置页只修改缓存, `Right Long` 保存缓存并写入持久化参数。
8. 严重错误页覆盖普通工作页, 错误解除后恢复来源页面或返回 `P20_HOME`。

## 3. 页面模型

| 页面ID | 状态归属 | 页面名称 | 页面类型 | 说明 |
|---|---|---|---|---|
| `P00_INIT` | `DS_INIT` | 初始化页 | 生命周期页 | 上电后短暂停留 |
| `P10_BOOTING` | `DS_BOOTING` | 启动自检页 | 生命周期页 | 显示启动动画/自检状态 |
| `P20_HOME` | `DS_WORK` | 工作总览页 | 主页面 | 四模块摘要 + 模块入口 |
| `P21_LIGHT` | `DS_WORK` | 灯光详情页 | 模块详情页 | 白光/RGB/亮度/电流 |
| `P22_HEAT` | `DS_WORK` | 热管理详情页 | 模块详情页 | 水温/加热/散热/目标温度 |
| `P23_WPUMP` | `DS_WORK` | 水泵详情页 | 模块详情页 | 模式/转速/状态 |
| `P24_O2PUMP` | `DS_WORK` | 氧气泵详情页 | 模块详情页 | 模式/转速/状态 |
| `P25_SETTING` | `DS_WORK` | 系统设置页 | 主页面 | SLEEP/BUZZER/HIGH BL/LOW BL/RESET |
| `P26_ADC` | `DS_WORK` | 采样参数页 | 主页面 | 温度组/电源组/光照组 |
| `P30_CLOSING` | `DS_CLOSING` | 关机中页 | 生命周期页 | 关机动画 |
| `P40_SLEEP` | `DS_SHUT_DOWN` | 息屏页 | 生命周期页 | 默认关闭 OLED |
| `P50_ERROR` | `DS_ERR` | 系统错误页 | 覆盖页 | 错误码/模块/描述/动作 |

## 4. 按键规则

| 场景 | Enter Short | Enter Long | Left Short | Right Short | Up Short | Down Short | Left Long | Right Long |
|---|---|---|---|---|---|---|---|---|
| `P20_HOME` | 进入选中模块详情页 | 系统开关机 | 切到上一主页面 | 切到下一主页面 | 选择上一模块 | 选择下一模块 | 保持/回到 HOME 默认焦点 | 无 |
| `P21_LIGHT` | 执行/确认当前字段 | 系统开关机 | 上一状态/减小 | 下一状态/增大 | 选择上一字段 | 选择下一字段 | 返回 HOME | 无 |
| `P22_HEAT` | 执行/确认当前字段 | 系统开关机 | 上一状态/减小 | 下一状态/增大 | 选择上一字段 | 选择下一字段 | 返回 HOME | 无 |
| `P23_WPUMP` | 执行/确认当前模式 | 系统开关机 | 上一模式 | 下一模式 | 保留 | 保留 | 返回 HOME | 无 |
| `P24_O2PUMP` | 执行/确认当前模式 | 系统开关机 | 上一模式 | 下一模式 | 保留 | 保留 | 返回 HOME | 无 |
| `P25_SETTING` | 进入编辑/确认字段 | 系统开关机 | 上一页面或编辑态减小 | 下一页面或编辑态增大 | 选择上一字段 | 选择下一字段 | 返回 HOME | 保存设置缓存 |
| `P26_ADC` | 切换字段详情 | 系统开关机 | 上一采样组 | 下一采样组 | 选择上一字段 | 选择下一字段 | 返回 HOME | 无 |
| `P50_ERROR` | 确认/消音 | 系统开关机 | 按错误策略处理 | 按错误策略处理 | 无 | 无 | 无 | 无 |

## 5. 数据结构调整

### 5.1 页面枚举

在 `Hardware/MD_Display/md_display_task.h` 中调整工作态页面枚举。建议保留现有生命周期页面, 新增第三版工作页:

- `DPI_HOME`
- `DPI_LIGHT`
- `DPI_HEAT`
- `DPI_WPUMP`
- `DPI_O2PUMP`
- `DPI_SETTING`
- `DPI_ADC`

旧的 `DPI_ENV / DPI_ACT / DPI_ALARM / DPI_QUICK` 可先保留但不作为第三版工作态默认入口, 避免一次性删除带来回退困难。

### 5.2 工作态上下文

新增或改造工作态上下文字段:

| 字段 | 用途 |
|---|---|
| `eMainPage` | 当前主页面: HOME / SET / ADC |
| `eHomeModule` | HOME 当前选中模块: LIGHT / HEAT / WPUMP / O2PUMP |
| `ucFieldIndex` | 当前页面内字段焦点 |
| `ucAdcGroupIndex` | 采样组: TEMP / POWER / LIGHT |
| `bEditing` | 设置页是否处于编辑态 |
| `tSettingCache` | 设置缓存, 保存前不写 Flash |
| `tWorkRestore` | 错误覆盖前的来源页面、模块、字段和编辑态 |

第二版中的 `eHomeTab / ucTabVisibleStart / ucTabItemIndex` 不再作为工作态主导航核心字段。实现时可先兼容保留, 但第三版绘制和按键逻辑不再依赖它们。

### 5.3 字段数据源映射表

第三版页面建议继续使用 `md_display_api.c` 中的 `DispUiSnapshot_T` 作为显示快照, 绘制函数只读取快照和页面上下文, 不在绘制过程中散落读取业务模块全局变量。

| 页面 | 字段 | 当前建议数据源 | 单位/格式 | 备注 |
|---|---|---|---|---|
| `P20_HOME` | `VIN` | `tAdcSamp.usVinVolt` | `0.1V`, 显示 `--.-V` | 需要扩展到 `DispUiSnapshot_T` |
| `P20_HOME` | `IIN` | `tAdcSamp.fVinCurr` | `A`, 显示 `--.-A` | 需要扩展到 `DispUiSnapshot_T` |
| `P20_HOME` | `PIN` | `tAdcSamp.usVinVolt / 10.0f * tAdcSamp.fVinCurr` | `W` | 建议在快照里预计算 |
| `P20_HOME` | `LIGHT PWR` | `tAdcSamp.us12VVolt / 10.0f * tAdcSamp.fLightCurr` | `W` | 可先显示 `LCURR` 替代 |
| `P20_HOME` | `HEAT PWR` | `tAdcSamp.us12VVolt / 10.0f * tAdcSamp.fHeatCurr` | `W` | 可先显示 `HCURR` 替代 |
| `P20_HOME` | `WP PWR` | `tAdcSamp.us12VVolt / 10.0f * tAdcSamp.fPumpCurr` | `W` | 可先显示 `PCURR` 替代 |
| `P20_HOME` | `O2 PWR` | `tAdcSamp.us12VVolt / 10.0f * tAdcSamp.fO2Curr` | `W` | 可先显示 `OCURR` 替代 |
| `P20_HOME` | `WHITE` | `tLight.eWordMode` -> `pc_disp_light_white_state()` | `OFF/DIM/FUL/SOS/TWK` | 已有辅助函数 |
| `P20_HOME` | `RGB` | `tLight.usRed/usGreen/usBlue` 和 `tLight.eWordMode` -> `pc_disp_light_rgb_state()` | `OFF/ON/SOS/TWK` | 已有辅助函数 |
| `P20_HOME` | `HT` | `bHeat_IsUiForceOn()` + 水温阈值 | `OFF/KEEP/HIGH/MAN` | 当前阈值在 `md_display_api.c` 中为 `25C/22C` |
| `P20_HOME` | `FAN` | `eFan_GetWorkMode()` | `OFF/LOW/MID/HIGH/FULL` | 已有 `pc_disp_fan_mode()` |
| `P20_HOME` | `WP MODE/SPD` | `tPump.eMode`, `tPump.usSpeed` | `OFF/LOW/MID/HIGH/MAX`, PWM | 已有 `pc_disp_pump_mode()` |
| `P20_HOME` | `O2 MODE/SPD` | `tO2Pump.eMode`, `tO2Pump.usSpeed` | `OFF/LOW/MID/HIGH/MAX`, PWM | 已有 `pc_disp_o2pump_mode()` |
| `P21_LIGHT` | `WHITE` | `tLight.eWordMode` | `OFF/DIM/FULL/SOS/TWK` | 设计完整枚举已存在, 当前循环函数只覆盖部分模式 |
| `P21_LIGHT` | `RGB` | `tLight.usRed/usGreen/usBlue` | `OFF/ON/SOS/TWK` | 当前缺少独立 RGB UI 控制接口 |
| `P21_LIGHT` | `WARM` | `tLight.usWarm` | `0~100%` 或 `0~1000 PWM` | 需要确认显示百分比还是 PWM 原值 |
| `P21_LIGHT` | `CURR` | `tAdcSamp.fLightCurr` | `mA` | 快照中已有 `usLightCurrMa` |
| `P22_HEAT` | `WATER` | `tAdcSamp.sWaterTemp1/2` 平均值 | `C` | 现有 `s_disp_get_water_temp()` 可复用 |
| `P22_HEAT` | `HEAT` | `bHeat_IsUiForceOn()` + 温度阈值 | `OFF/KEEP/HIGH/MAN` | 手动强制开显示 `MAN` |
| `P22_HEAT` | `FAN` | `eFan_GetWorkMode()` 或 `tHM.eWordMode` | `OFF/LOW/MID/HIGH/FULL` | 控制接口已存在 |
| `P22_HEAT` | `TARGET` | `DISP_HEAT_TARGET_TEMP_C` | `C` | 当前是显示常量, 暂无设置接口 |
| `P23_WPUMP` | `MODE` | `tPump.eMode` | `OFF/LOW/MID/HIGH/MAX` | 控制接口已存在 |
| `P23_WPUMP` | `SPEED` | `tPump.usSpeed` | PWM 或 `%` | 百分比 = `usSpeed * 100 / pumpPWM_MAX_VALUE` |
| `P23_WPUMP` | `STATE` | `tPump.eDevState` 或 `tPump.usSpeed > 0` | `RUN/STOP/FAULT` | FAULT 需后续接错误码 |
| `P24_O2PUMP` | `MODE` | `tO2Pump.eMode` | `OFF/LOW/MID/HIGH/MAX` | 控制接口已存在 |
| `P24_O2PUMP` | `SPEED` | `tO2Pump.usSpeed` | PWM 或 `%` | 百分比 = `usSpeed * 100 / o2PWM_MAX_VALUE` |
| `P24_O2PUMP` | `STATE` | `tO2Pump.eDevState` 或 `tO2Pump.usSpeed > 0` | `RUN/STOP/FAULT` | FAULT 需后续接错误码 |
| `P25_SETTING` | `SLEEP` | `tDispPageCtx.tSettingCache.usSleepTime` | `s` | 持久化源为 `tAppMemParam.tDISP.usAutoOffTime` |
| `P25_SETTING` | `BUZZER` | `tDispPageCtx.tSettingCache.bBuzOff` | `ON/OFF` | 持久化源为 `tAppMemParam.tSYS.bBuzSwitchOff` |
| `P25_SETTING` | `HIGH BL` | `tDispPageCtx.tSettingCache.ucHighLightValue` | `0~255` | 当前调整步进为 `5` |
| `P25_SETTING` | `LOW BL` | `tDispPageCtx.tSettingCache.ucLowLightValue` | `0~255` | 当前调整步进为 `5` |
| `P25_SETTING` | `RESET` | `tDispPageCtx.tSettingCache.bRestoreDefault` | `NO/YES` | 保存时装载默认参数 |
| `P26_ADC` | `WT1/WT2` | `tAdcSamp.sWaterTemp1/2` | `C` | 已有 ADC 字段 |
| `P26_ADC` | `BOARD5/BOARD12` | `tAdcSamp.s5VTemp/s12VTemp` | `C` | 已有 ADC 字段 |
| `P26_ADC` | `12V` | `tAdcSamp.us12VVolt` | `0.1V` | 已有 ADC 字段 |
| `P26_ADC` | `VIN/IIN/PIN` | `tAdcSamp.usVinVolt`, `tAdcSamp.fVinCurr`, 计算功率 | `V/A/W` | 需要扩展快照 |
| `P26_ADC` | `LIGHT/LCURR` | `tAdcSamp.usLightRes`, `tAdcSamp.fLightCurr` | raw/mA | 已有 ADC 字段 |
| `P50_ERROR` | `CODE/MOD/DESC/ACT` | `usDisp_ErrCodeDisplay()`, `pc_disp_err_*()` | 短文本 | 现有错误页辅助函数可复用 |

### 5.4 控制动作接口表

| 页面 | 字段 | Left/Right 动作 | 当前可用接口 | 需要补齐 |
|---|---|---|---|---|
| `P21_LIGHT` | `WHITE` | 按枚举前后切换 | `vLight_CircSelectMode()` | 当前只循环 `OFF/HALF/FULL`, 需补齐 `SOS/TWINKLE` 或明确隐藏 |
| `P21_LIGHT` | `RGB` | 切换 RGB 模式 | 暂无独立公开接口 | 建议新增 `bLight_SetRgbMode()` 或统一到灯光模式枚举 |
| `P21_LIGHT` | `WARM` | 增减亮度 | 暂无独立公开接口, 底层 `v_light_set_rgbw()` 为 static | 如需可调, 新增公开亮度设置接口 |
| `P22_HEAT` | `HEAT` | 自动/手动强制切换 | `bHeat_ToggleUiForce()` | 可用 |
| `P22_HEAT` | `FAN` | 风扇档位前后切换 | `bFan_CycleUiMode(bool add)` | 可用 |
| `P22_HEAT` | `TARGET` | 增减目标温度 | 暂无 | 当前先只显示 `DISP_HEAT_TARGET_TEMP_C` |
| `P23_WPUMP` | `MODE` | `OFF <-> LOW <-> MID <-> HIGH <-> MAX` | `bPump_SetMode(PumpWorkMode_E mode)` | 可用 |
| `P24_O2PUMP` | `MODE` | `OFF <-> LOW <-> MID <-> HIGH <-> MAX` | `bO2Pump_SetMode(O2PumpWorkMode_E mode)` | 可用 |
| `P25_SETTING` | `SLEEP` | `-30s/+30s`, 范围 `0~3600s` | `b_disp_adjust_setting_item(DSI_SLEEP, add)` | 可复用或改名为第三版通用函数 |
| `P25_SETTING` | `BUZZER` | `ON/OFF` 切换 | `b_disp_adjust_setting_item(DSI_BUZZER, add)` | 可用 |
| `P25_SETTING` | `HIGH BL` | `-5/+5`, 当前范围约 `5~250` | `b_disp_adjust_setting_item(DSI_HIGH_LIGHT, add)` | 可用 |
| `P25_SETTING` | `LOW BL` | `-5/+5`, 当前范围约 `5~250` | `b_disp_adjust_setting_item(DSI_LOW_LIGHT, add)` | 可用 |
| `P25_SETTING` | `RESET` | `NO/YES` 切换 | `b_disp_adjust_setting_item(DSI_RESET, add)` | 可用 |
| `P25_SETTING` | 保存 | 写入显示/系统记忆参数 | `b_disp_save_setting_cache()` -> `cApp_UpdataMemParam()` | 可用 |
| `P26_ADC` | 采样组 | `TEMP <-> POWER <-> LIGHT` | 当前 `ucAdcGroupIndex` 逻辑可复用 | 建议改名为第三版 ADC 组字段 |

### 5.5 图标映射表

现有图标文件为 `Hardware/MD_Display/icon_bitmaps.h`, 每个图标 `16x16`, 使用 `u8g2_DrawXBMP()` 绘制。

| 页面/模块 | 图标数组名 | 备注 |
|---|---|---|
| `LIGHT` | `icon_light_16x16` | 已存在 |
| `HEAT` | `icon_heat_16x16` | 已存在 |
| `WPUMP` | `icon_wpump_16x16` | 已存在 |
| `O2PUMP` | `icon_o2pump_16x16` | 已存在 |
| `SET` | `icon_setting_16x16` | 已存在 |
| `ADC` | `icon_adc_16x16` | 已存在 |

### 5.6 U8g2 坐标落地建议

第三版 Excel 使用 `8x8` 像素格表达布局。代码实现时建议统一用下面的坐标约定, 保证每页绘制函数风格一致。

| 区域 | x | y | w | h | 字体/绘制 | 说明 |
|---|---:|---:|---:|---:|---|---|
| 顶部栏 | 0 | 0 | 128 | 8 | `u8g2_font_5x8_tr` | 白底黑字, 标题基线建议 `y=7` |
| 内容区 | 0 | 8 | 128 | 56 | 页面自定 | 所有第三版主页面和详情页共用 |
| 底部提示行 | 0 | 56 | 128 | 8 | `u8g2_font_5x8_tr` | 可显示焦点/保存/返回提示 |
| HOME 电源摘要 | 0 | 8 | 128 | 8 | `u8g2_font_5x8_tr` | `VIN/IIN/PIN` |
| HOME 左上模块 | 0 | 16 | 64 | 20 | icon + 5x8 文本 | `LIGHT` |
| HOME 右上模块 | 64 | 16 | 64 | 20 | icon + 5x8 文本 | `HEAT` |
| HOME 左下模块 | 0 | 36 | 64 | 20 | icon + 5x8 文本 | `WPUMP` |
| HOME 右下模块 | 64 | 36 | 64 | 20 | icon + 5x8 文本 | `O2PUMP` |
| 详情页图标 | 0 | 8 | 16 | 16 | XBMP | 当前模块图标 |
| 详情页摘要 | 20 | 8 | 108 | 16 | `u8g2_font_5x8_tr` | 电压/电流/功率 |
| 详情页字段 1 | 0 | 24 | 128 | 8 | `u8g2_font_5x8_tr` | 焦点可反显 |
| 详情页字段 2 | 0 | 32 | 128 | 8 | `u8g2_font_5x8_tr` | 焦点可反显 |
| 详情页字段 3 | 0 | 40 | 128 | 8 | `u8g2_font_5x8_tr` | 焦点可反显 |
| 详情页字段 4 | 0 | 48 | 128 | 8 | `u8g2_font_5x8_tr` | 焦点可反显 |

焦点绘制建议:

- HOME 模块焦点: 当前模块块区域外框或标题反显。
- 详情页字段焦点: 当前字段整行反显, 或字段名反显。
- 设置页编辑态: 字段值反显; 查看态只反显字段名。
- 横向滚动: 只对当前焦点字段启用, 其他字段截断显示。

### 5.7 设置页保存和退出策略

为避免小屏幕上出现复杂确认弹窗, 第三版设置页采用简单规则:

- 进入 `P25_SETTING` 时加载当前参数到 `tSettingCache`。
- `Left/Right Short` 在编辑态只修改 `tSettingCache`, 不写 Flash。
- `Right Long` 调用保存函数, 成功后清除 `bDirty`, 并提示 `SAVE OK`。
- `Left Long` 返回 `P20_HOME`。如果 `bDirty == true`, 直接丢弃未保存修改, 重新加载当前持久化参数, 并短暂提示 `DISCARD`。
- 页面切到 `HOME/ADC` 时也按未保存丢弃处理, 避免缓存跨页面悬挂。
- `RESET=YES` 只在 `Right Long` 保存时生效; 未保存退出不恢复默认参数。

### 5.8 旧页面处理策略

第三版实施期间不建议立即删除旧页面, 以降低回退风险。

| 旧页面/字段 | 处理策略 |
|---|---|
| `DPI_ENV` | 保留绘制函数和枚举, 普通工作态导航隐藏 |
| `DPI_ACT` | 保留绘制函数和枚举, 普通工作态导航隐藏 |
| `DPI_ALARM` | 若错误页已覆盖主要告警, 普通工作态导航隐藏; 后续可作为调试页 |
| `DPI_QUICK` | 保留为临时调试入口或后续删除 |
| `DHT_* / DHM_TAB_ACTIVE` | 第三版代码不再依赖; 可先兼容保留, 完成验收后清理 |
| `ucTabVisibleStart` | 第三版不使用; 编译稳定后删除或迁移为主页面索引字段 |

## 6. 实施阶段

### 阶段 A: 设计冻结与代码基线

目标: 确认第三版 Excel 已作为唯一设计输入, 并拿到当前工程可编译基线。

任务:

- 确认 `APP_UI_第三版本_修正版.xlsx` 页面结构无歧义。
- 确认图标资源 `Hardware/MD_Display/icon_bitmaps.h` 与 `LIGHT / HEAT / WPUMP / O2PUMP / SET / ADC` 对应。
- 记录当前 `MD_Display` 工作态入口、页面枚举、按键分发和绘制函数。
- 执行一次工程编译, 确认改造前基线可用。

验收:

- 第三版页面 ID 和按键规则冻结。
- 基线编译结果记录完成。
- 明确哪些第二版字段保留兼容, 哪些字段不再参与第三版逻辑。

### 阶段 B: 页面状态机重构

目标: 将 `DS_WORK` 下的默认工作态从第二版小 tab 模式改为第三版主页面/详情页模式。

任务:

- 更新 `DispPageId_E` 或新增工作态视图枚举。
- 实现 `P20_HOME -> P21/P22/P23/P24` 的详情页进入逻辑。
- 实现 `P20_HOME / P25_SETTING / P26_ADC` 三个主页面之间的 `Left/Right Short` 循环切换。
- 实现详情页 `Left Long` 返回 `P20_HOME`。
- 保持 `Enter Long` 继续走系统开关机流程。
- 处理从错误页恢复到来源工作页的上下文保存与恢复。

验收:

- 工作态默认进入 `P20_HOME`。
- 三个主页面可循环切换。
- 四个详情页可从 HOME 进入并返回。
- `Enter Long` 在所有页面都不会被第三版逻辑截获为其他行为。

### 阶段 C: 页面绘制实现

目标: 按第三版 Excel 完成所有工作态页面绘制。

任务:

- 统一顶部 `128x8` 白底黑字标题栏绘制。
- 实现 `P20_HOME` 总览页:
  - 顶部显示 `HOME 1/3`。
  - 显示 `VIN / IIN / PIN`。
  - 两行两列显示 `LIGHT / HEAT / WPUMP / O2PUMP` 摘要。
  - 当前模块焦点有明确反显或边框提示。
- 实现 `P21_LIGHT`:
  - `WHITE / RGB / WARM / CURR` 字段。
  - 支持当前字段焦点。
- 实现 `P22_HEAT`:
  - `WATER / HEAT / FAN / TARGET` 字段。
  - 支持当前字段焦点。
- 实现 `P23_WPUMP`:
  - `MODE / SPEED / STATE / NOTE` 字段。
- 实现 `P24_O2PUMP`:
  - `MODE / SPEED / STATE / NOTE` 字段。
- 实现 `P25_SETTING`:
  - `SLEEP / BUZZER / HIGH BL / LOW BL / RESET` 字段。
  - 区分查看态和编辑态。
- 实现 `P26_ADC`:
  - 温度组: `WT1 / WT2 / BOARD5 / BOARD12 / MAXT`。
  - 电源组: `12V / VIN / IIN / PIN / ADC RAW`。
  - 光照组: `LIGHT / LCURR / WARM / RGB / BOARD`。
- 补齐 `P40_SLEEP` 默认息屏策略和 `P50_ERROR` 错误模板。

验收:

- 所有第三版页面都能独立绘制。
- 页面不再使用第二版 `4 个 32px 小卡片` 布局。
- 单页内容不出现明显横向挤压。
- 当前焦点在每个可操作页面都可识别。

### 阶段 D: 数据绑定与控制动作

目标: 将页面字段绑定到现有任务数据和控制接口。

任务:

- 扩展 `DispUiSnapshot_T` 或等价快照结构, 补齐第三版页面需要的数据。
- 绑定灯光:
  - 白光状态: `OFF / DIM / FULL / SOS / TWK`
  - RGB 状态: `OFF / ON / SOS / TWK`
  - 亮度或暖光百分比
  - 灯光电流
- 绑定热管理:
  - 水温
  - 加热模式
  - 风扇模式
  - 目标温度
- 绑定水泵和氧气泵:
  - 模式
  - 速度/占空比
  - 运行状态
- 绑定系统设置:
  - 息屏时间
  - 蜂鸣器开关
  - 高亮亮度
  - 低亮亮度
  - 恢复默认设置确认项
- 绑定采样参数:
  - `tAdcSamp` 温度、电源、光照相关字段。
- 对暂时没有数据源的字段使用明确占位值, 并在代码注释或任务清单中标注待接入项。

验收:

- 页面显示值来自统一快照, 不在绘制函数中直接散落读取业务全局变量。
- 控制动作调用现有模块接口, 不绕过任务状态机。
- 设置页修改缓存时不写 Flash, `Right Long` 后才持久化。

### 阶段 E: 焦点、滚动与刷新优化

目标: 确保低分辨率 OLED 上显示稳定, 操作反馈清晰。

任务:

- 为每个页面定义字段数量和焦点移动规则。
- 横向文本超出时只允许当前焦点字段滚动。
- 切页面、切焦点、退出详情页时重置文本滚动状态。
- 页面切换、字段变更、数据刷新时正确设置 dirty mask。
- 避免全屏无意义刷新造成闪烁。

验收:

- 长文本不会遮挡其他字段。
- 非焦点字段保持静态截断或短标签显示。
- 快速按键时焦点和页面不会错乱。
- 数据周期刷新时无明显闪烁。

### 阶段 F: 异常页与低功耗闭环

目标: 保证第三版工作页与错误页、息屏页、关机页配合稳定。

任务:

- 严重错误触发时覆盖当前页面。
- 错误解除后恢复来源页面、模块和字段焦点。
- 来源无效时返回 `P20_HOME`。
- 息屏后第一个唤醒键只唤醒屏幕, 不执行页面功能。
- 关机中页不响应普通短按业务动作。

验收:

- 任意工作页触发严重错误都能进入 `P50_ERROR`。
- 错误恢复后页面上下文符合预期。
- 息屏唤醒不会误触发开关、模式切换或保存动作。

### 阶段 G: 联调与验收

目标: 在编译、仿真/实机和人工操作路径上确认第三版可交付。

任务:

- 工程编译通过。
- 逐页检查显示内容。
- 逐键检查短按和长按语义。
- 检查设置缓存保存和掉电恢复。
- 检查错误覆盖与恢复。
- 检查息屏唤醒。
- 记录剩余数据源缺口和实机观察问题。

验收:

- 第三版全部页面可达。
- 页面显示与 Excel 规格一致。
- 所有按键路径符合第 4 节规则。
- 无编译错误。
- 无明显屏幕闪烁、错位、文字覆盖。

## 7. 建议工期

| 阶段 | 内容 | 预计时间 |
|---|---|---|
| A | 设计冻结与代码基线 | 0.5 天 |
| B | 页面状态机重构 | 1 天 |
| C | 页面绘制实现 | 1.5 天 |
| D | 数据绑定与控制动作 | 1 天 |
| E | 焦点、滚动与刷新优化 | 0.5 天 |
| F | 异常页与低功耗闭环 | 0.5 天 |
| G | 联调与验收 | 1 天 |

总计建议预留 `5~6` 个工作日。若控制接口和 ADC 数据源已有稳定封装, 可压缩到 `4` 个工作日左右。

## 8. 风险与对策

| 风险 | 影响 | 对策 |
|---|---|---|
| 第二版 tab 上下文残留 | 页面切换和焦点状态混乱 | 第三版逻辑不再依赖 `ucTabVisibleStart`; 保留字段只做兼容 |
| 页面 ID 重构影响旧页面 | 可能破坏现有 ENV/ACT/ALARM 路径 | 旧页面先保留, 第三版工作态不默认进入 |
| 中文或长文本显示溢出 | OLED 上显示不完整 | 实际屏幕优先用短英文标签, 中文只放文档说明 |
| 设置项误写 Flash | Flash 写入次数增加或配置异常 | 设置页使用缓存, `Right Long` 才保存 |
| 长按语义冲突 | 用户误关机或无法退出详情页 | `Enter Long` 固定系统开关机; 详情页退出固定 `Left Long` |
| 错误覆盖后无法恢复来源 | 用户回到错误页面前的位置失败 | 进入错误页前保存来源页面和焦点上下文 |

## 9. 验收用例清单

| 用例ID | 操作 | 预期结果 |
|---|---|---|
| UI3-P01 | 上电进入工作态 | 默认显示 `P20_HOME` |
| UI3-P02 | HOME 中 `Left/Right Short` | 在 `HOME / SET / ADC` 三个主页面循环切换 |
| UI3-P03 | HOME 中 `Up/Down Short` | 在 `LIGHT / HEAT / WPUMP / O2PUMP` 四个模块间移动焦点 |
| UI3-P04 | HOME 选中 LIGHT 后 `Enter Short` | 进入 `P21_LIGHT` |
| UI3-P05 | HOME 选中 HEAT 后 `Enter Short` | 进入 `P22_HEAT` |
| UI3-P06 | HOME 选中 WPUMP 后 `Enter Short` | 进入 `P23_WPUMP` |
| UI3-P07 | HOME 选中 O2PUMP 后 `Enter Short` | 进入 `P24_O2PUMP` |
| UI3-P08 | 任意详情页 `Left Long` | 返回 `P20_HOME` |
| UI3-K01 | 任意页面 `Enter Long` | 进入系统开关机流程 |
| UI3-K02 | LIGHT 详情页调整 WHITE/RGB | 状态切换并立即反馈 |
| UI3-K03 | HEAT 详情页调整 HEAT/FAN | 状态切换并立即反馈 |
| UI3-K04 | WPUMP 详情页 `Left/Right` | 水泵模式循环切换 |
| UI3-K05 | O2PUMP 详情页 `Left/Right` | 氧气泵模式循环切换 |
| UI3-S01 | SET 页面修改设置但不保存 | 只改变缓存, 不立即写 Flash |
| UI3-S02 | SET 页面 `Right Long` | 保存设置缓存 |
| UI3-A01 | ADC 页面 `Left/Right` | 在温度组、电源组、光照组之间切换 |
| UI3-A02 | ADC 页面 `Up/Down` | 在当前组字段间移动焦点 |
| UI3-E01 | 工作页触发严重错误 | 覆盖显示 `P50_ERROR` |
| UI3-E02 | 错误解除 | 返回来源页面; 来源无效则返回 HOME |
| UI3-L01 | 息屏后短按任意键 | 只唤醒屏幕, 不执行业务动作 |

## 10. 交付物

- 第三版 UI 规格: `docs/UI/APP_UI_第三版本_修正版.xlsx`
- 第三版执行计划: `docs/UI/APP_UI_第三版本_执行计划.md`
- 代码改造范围:
  - `Hardware/MD_Display/md_display_task.h`
  - `Hardware/MD_Display/md_display_task.c`
  - `Hardware/MD_Display/md_display_api.c`
  - `Hardware/MD_Display/md_display_queue_task_work.c`
  - 必要时补充 `Hardware/MD_Display/icon_bitmaps.h`

## 11. 推荐推进顺序

1. 先完成页面状态机, 让所有页面可进入、可返回。
2. 再完成静态绘制, 确保布局和焦点符合第三版 Excel。
3. 然后绑定真实数据和控制动作。
4. 最后处理横向滚动、脏区刷新、错误恢复和息屏唤醒。

这样推进可以让每一阶段都有可见结果, 也便于在实机上快速发现布局和按键问题。

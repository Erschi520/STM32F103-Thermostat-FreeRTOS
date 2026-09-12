# STM32F103-Thermostat-FreeRTOS
基于 STM32F103C8T6 + FreeRTOS 恒温控制系统，DS18B20 采集、PID‑PWM 控制加热片风扇，OLED 显示，课程设计项目。
# STM32F103‑Thermostat‑FreeRTOS
> 课程设计项目：基于STM32F103C8T6与FreeRTOS的闭环恒温控制系统

## 项目概述
本项目采用STM32F103C8T6为主控芯片，基于FreeRTOS(CMSIS‑RTOS v2)实现多任务框架。DS18B20单总线传感器采集温度，位置式PID算法运算输出PWM，分别驱动加热片升温、风扇散热；0.96寸I2C‑OLED显示设定温度与实时温度，按键可以修改目标恒温值，实现室内闭环恒温控制。

> 本仓库**仅维护FreeRTOS实时操作系统版本**，无裸机工程。

### 硬件组成
- MCU：STM32F103C8T6
- 温度传感器：DS18B20（单总线）
- 显示：0.96‑inch I2C OLED
- 执行机构：加热片、散热风扇（三极管PWM驱动）
- 输入：独立按键，用于设置目标温度

### 性能指标
- 可控温度范围：0℃ ~ 60℃
- 稳态控制精度：±0.5℃
- 最大温度超调：≤1℃
- PWM输出频率：1kHz
- OLED刷新频率：10Hz

## 开发环境
- MCU配置工具：STM32CubeMX
- IDE：Keil‑MDK5
- 固件库：STM32 HAL Library
- RTOS：FreeRTOS (CMSIS‑RTOS V2)
- PCB绘制：Altium Designer

## 软件框架说明
`main()`完成HAL初始化、系统时钟、GPIO/I2C/TIM外设初始化，启动PWM通道；初始化RTOS内核后，`MX_FREERTOS_Init()`中创建用户任务`TempDisplayTask`，负责温度采集、滑动滤波、PID计算、PWM更新、OLED屏幕刷新；设置合理任务栈与优先级，调用`osKernelStart()`开启调度器，操作系统接管全部任务。

任务列表：
1. **TempDisplayTask**：温度采集、PID闭环运算、OLED显示更新，优先级高于空闲任务，栈大小512 words
2. StartDefaultTask：CubeMX自动生成的预留默认任务

## 使用方法
1. 使用STM32CubeMX打开`.ioc`工程，或者直接在Keil MDK5打开工程文件
2. 编译，通过SWD下载程序至STM32F103C8T6
3. 上电，OLED显示当前温度与设定温度
4. 按键调整目标温度；系统自动PID调节：低温开启加热片，高温启动风扇散热

## 文件说明
├── Hardware/         原理图、PCB、
├── Software/FreeRTOS‑Keil    CubeMX + Keil 完整工程
├── Documents/        设计文档、测试数据
└── Images/           实物、框图、调试截图

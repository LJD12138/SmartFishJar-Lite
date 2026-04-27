
#include "Usb/usb_prot_frame.h"

#if(boardUSB_EN)
#include "Usb/usb_queue_task.h"
#include "Usb/usb_task.h"
#include "Usb/usb_iface.h"
#if(boardPRINT_IFACE)
#include "Print/print_task.h"
#endif  //boardPRINT_IFACE

// #include "check.h"


// #define       	usbDEV_ADRR                          	0x01
// #define  		usbWAIT_NOTIFY_OUTTIME              	1000     //任务通知超时时间 MS
// #define       	usbTX_PROTO_BUFF_LEN                   	128
// #define       	usbRX_PROTO_BUFF_LEN                   	256

//*********************************寄存器地址********************************
#define     	SW3516_REG1_ADDR              			0x01//VOUT
#define     	SW3516_REG2_ADDR              			0x02//VOUT
#define     	SW3516_PD1_ADDR              			0x31//VOUT
#define     	SW3516_ADC_EN_ADDR              		0x78//VOUT
#define     	SW3516_VOUT1_ADDR              			0x92//VOUT
#define     	SW3516_VOUT2_ADDR              			0x93//VOUT
#define     	SW3516_PD_IOUT1_ADDR             		0x94//IOUT1
#define     	SW3516_PD_IOUT2_ADDR             		0x95//IOUT2
#define     	SW3516_QC_IOUT1_ADDR             		0x96//IOUT1
#define     	SW3516_QC_IOUT2_ADDR             		0x97//IOUT2


//****************************************************参数初始化**************************************************//
// __ALIGNED(4) 	ModbusProtoTx_t *tpUsbProtoTx = NULL;	//发送协议
// __ALIGNED(4) 	ModbusProtoRx_t *tpUsbProtoRx = NULL;	//接受协议
#pragma pack(1)
typedef struct
{
	vs8            		cTemp;              //温度
    vs16           		sPower;             //1W 总功率 
    vu16           		usVolt;             //mV
    vu16           		usPdCurr;       	//mA
	vu16           		usQcCurr;       	//mA
	vu16           		usPdPwr;        	//1W
    vu16           		usQcPwr;        	//1W
}USB_IC_T; 
#pragma pack()

//****************************************************函数声明****************************************************//
// static s8 c_usb_data_trans(u8 cmd, u16 reg_addr, u8* data, u8 len);



/***********************************************************************************************************************
-----函数功能    通讯协议初始化
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
bool bUsb_SendProtInit(void)
{
	// s8 c_result = 1;

	// c_result = cModbus_TransProtoInit(&tpUsbProtoTx, usbTX_PROTO_BUFF_LEN, usbDEV_ADRR);
	// if(c_result <= 0)
	// {
	// 	if(uPrint.tFlag.bUsbTask || uPrint.tFlag.bImportant)
	// 		log_e("bUsbTask:tpUsbProtoTx协议对象初始化失败,代码%d",c_result);
		
	// 	return false;
	// }
	
	return true;
}

bool bUsb_RecProtInit(void)
{
	// s8 c_result = cModbus_RecProtoInit(&tpUsbProtoRx, 	//协议指针
	// 							usbRX_PROTO_BUFF_LEN,	//协议缓存器大小
	// 							usbDEV_ADRR,			//协议设备ID
	// 							boardREPET_TIMER_CYCLE_TMIE);			//计数器采样时间
	// if(c_result <= 0)
	// {
	// 	if(uPrint.tFlag.bUsbRecTask || uPrint.tFlag.bImportant)
	// 		log_e("bUsbRecTask:tpUsbProtoRx协议对象初始化失败,代码%d",c_result);
	// 	return false;
	// }
	
	return true;
}

/*****************************************************************************************************************
-----函数功能    指令:初始化IC
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
s8 c_usb_cs_ic_init(const I2cObj_T *p_i2c_obj)
{
	u8 data[1] = {0};
	u8 read_data[3] = {0};
	static u8 uc_index[2] = {0};
	static u8 uc_lost_cnt[2] = {0};
	static u8 uc_err_reg[2][3] = {0}; //保存0x89-0x8B异常状态寄存器值
	u8 ch = (p_i2c_obj == &tUSB_IC1_I2C) ? 0 : 1;
	
	switch(uc_index[ch])
	{
		case 0:
		{
			data[0] = 0x7F;
			if(cI2C_WriteBytes(p_i2c_obj, SW3516_REG1_ADDR, data, sizeof(data)) <= 0)
			{
				if(uc_lost_cnt[ch] < 0xff) 
					uc_lost_cnt[ch]++;
				break;
			}
			else
			{
				uc_lost_cnt[ch] = 0;
				uc_index[ch]++;
			}
		}

		case 1:
		{
			data[0] = 0x06;
			if(cI2C_WriteBytes(p_i2c_obj, 0x03, data, sizeof(data)) <= 0)
			{
				if(uc_lost_cnt[ch] < 0xff) 
					uc_lost_cnt[ch]++;
				break;
			}
			else
			{
				uc_lost_cnt[ch] = 0;
				uc_index[ch]++;
			}
		}
		
		case 2:
		{
			data[0] = 0xA0;
			if(cI2C_WriteBytes(p_i2c_obj, SW3516_ADC_EN_ADDR, data, sizeof(data)) <= 0)
			{
				if(uc_lost_cnt[ch] < 0xff) 
					uc_lost_cnt[ch]++;
				break;
			}
			else
			{
				uc_lost_cnt[ch] = 0;
				uc_index[ch]++;
			}
		}

		case 3:
		{
			data[0] = 0x48;
			if(cI2C_WriteBytes(p_i2c_obj, 0x30, data, sizeof(data)) <= 0)
			{
				if(uc_lost_cnt[ch] < 0xff) 
					uc_lost_cnt[ch]++;
				break;
			}
			else
			{
				uc_lost_cnt[ch] = 0;
				uc_index[ch]++;
			}
		}

		case 4:
		{
			data[0] = 0xFF;
			if(cI2C_WriteBytes(p_i2c_obj, 0x31, data, sizeof(data)) <= 0)
			{
				if(uc_lost_cnt[ch] < 0xff) 
					uc_lost_cnt[ch]++;
				break;
			}
			else
			{
				uc_lost_cnt[ch] = 0;
				uc_index[ch]++;
			}
		}

		case 5:
		{
			data[0] = 0x80;
			if(cI2C_WriteBytes(p_i2c_obj, 0x4A, data, sizeof(data)) <= 0)
			{
				if(uc_lost_cnt[ch] < 0xff) 
					uc_lost_cnt[ch]++;
				break;
			}
			else
			{
				uc_lost_cnt[ch] = 0;
				uc_index[ch]++;
			}
		}

		case 6:
		{
			data[0] = 0x40;
			if(cI2C_WriteBytes(p_i2c_obj, 0x70, data, sizeof(data)) <= 0)
			{
				if(uc_lost_cnt[ch] < 0xff) 
					uc_lost_cnt[ch]++;
				break;
			}
			else
			{
				uc_lost_cnt[ch] = 0;
				uc_index[ch]++;
			}
		}

		case 7:
		{
			data[0] = 0x8C;
			if(cI2C_WriteBytes(p_i2c_obj, 0x04, data, sizeof(data)) <= 0)
			{
				if(uc_lost_cnt[ch] < 0xff) 
					uc_lost_cnt[ch]++;
				break;
			}
			else
			{
				uc_lost_cnt[ch] = 0;
				uc_index[ch]++;
			}
		}

		case 8:
		{
			memset(read_data, 0, sizeof(read_data));
//			if(cI2C_ReadBytes(p_i2c_obj, 0x89, read_data, sizeof(read_data)) <= 0)
//			{
//				if(uc_lost_cnt[ch] < 0xff) 
//					uc_lost_cnt[ch]++;
//				break;
//			}
//			else
			{
				uc_err_reg[ch][0] = read_data[1];
				uc_lost_cnt[ch] = 0;
				uc_index[ch]++;
			}
		}

		case 9:
		{
			memset(read_data, 0, sizeof(read_data));
			if(cI2C_ReadBytes(p_i2c_obj, 0x8A, read_data, sizeof(read_data)) <= 0)
			{
				if(uc_lost_cnt[ch] < 0xff) 
					uc_lost_cnt[ch]++;
				break;
			}
			else
			{
				uc_err_reg[ch][1] = read_data[1];
				uc_lost_cnt[ch] = 0;
				uc_index[ch]++;
			}
		}

		case 10:
		{
			memset(read_data, 0, sizeof(read_data));
//			if(cI2C_ReadBytes(p_i2c_obj, 0x8B, read_data, sizeof(read_data)) <= 0)
//			{
//				if(uc_lost_cnt[ch] < 0xff) 
//					uc_lost_cnt[ch]++;
//				break;
//			}
//			else
			{
				uc_err_reg[ch][2] = read_data[1];
				uc_lost_cnt[ch] = 0;
				uc_index[ch]++;
			}
		}

		case 11:
		{
			// 检查0x89-0x8B是否存在异常(任何非零位表示异常保护)
			if(uc_err_reg[ch][0] != 0 || uc_err_reg[ch][1] != 0 || uc_err_reg[ch][2] != 0)
			{
				// 清空异常状态寄存器
				data[0] = 0x00;
				cI2C_WriteBytes(p_i2c_obj, 0x89, data, sizeof(data));
				cI2C_WriteBytes(p_i2c_obj, 0x8A, data, sizeof(data));
				cI2C_WriteBytes(p_i2c_obj, 0x8B, data, sizeof(data));
				
				if(uPrint.tFlag.bUsbTask || uPrint.tFlag.bImportant)
					log_w("bUsbTask:SW3518[%d]异常状态[0x%02X,0x%02X,0x%02X],已清除并重新初始化", 
							ch, uc_err_reg[ch][0], uc_err_reg[ch][1], uc_err_reg[ch][2]);
				
				// 重新初始化
				uc_index[ch] = 0;
				return 0;
			}
			
			uc_index[ch] = 0;
			return 1;
		}
		
		default:
			uc_index[ch] = 0;
		break;
	}

	if(uc_lost_cnt[ch] >= 10)
	{
		if(ch == 0)
		{
			if(tUsb.uErrCode.tCode.bIc1Lost == 0)
			{
				bUsb_SetErrCode(UEC_IC1_LOST,true);
			
				if(uPrint.tFlag.bUsbTask || uPrint.tFlag.bImportant)
					log_e("bUsbTask:IC1丢失");
			}
		}
		else
		{
			if(tUsb.uErrCode.tCode.bIc2Lost == 0)
			{
				bUsb_SetErrCode(UEC_IC2_LOST,true);
			
				if(uPrint.tFlag.bUsbTask || uPrint.tFlag.bImportant)
					log_e("bUsbTask:IC2丢失");
			}
		}
		
		return -1;
	}
	else 
	{
		if(ch == 0)
		{
			if(tUsb.uErrCode.tCode.bIc1Lost == 1)
				bUsb_SetErrCode(UEC_IC1_LOST,false);
		}
		else
		{
			if(tUsb.uErrCode.tCode.bIc2Lost == 1)
				bUsb_SetErrCode(UEC_IC2_LOST,false);
		}
		
		return 0;
	}
}


USB_IC_T tUsbIc[2] = {0}; //IC参数: [0]PD100W芯片, [1]无线充芯片
/*****************************************************************************************************************
-----函数功能    指令:获取参数
-----说明(备注)  p_i2c_obj: 传入&tUSB_IC1_I2C或&tUSB_IC2_I2C
-----传入参数    p_i2c_obj: I2C对象指针
-----输出参数    none
-----返回值      -1:IC丢失  0:进行中  1:成功
******************************************************************************************************************/
s8 c_usb_cs_get_ic_param(const I2cObj_T *p_i2c_obj)
{
	static u8 uc_index[2] = {0};
	static u8 uc_lost_cnt[2] = {0};
	u8 ch = (p_i2c_obj == &tUSB_IC1_I2C) ? 0 : 1;
	u8 buff[6] = {0};
	u8 data[3] = {0};
	USB_IC_T *p_ic = &tUsbIc[ch];

	switch (uc_index[ch])
	{
		case 0:
		{
			data[0] = 0xA0;
			if(cI2C_WriteBytes(p_i2c_obj, SW3516_ADC_EN_ADDR, data, sizeof(data)) <= 0)
			{
				if(uc_lost_cnt[ch] < 0xff) 
					uc_lost_cnt[ch]++;
				break;
			}
			else
			{
				uc_lost_cnt[ch] = 0;
				uc_index[ch]++;
			}
		}
		
		//获取功率
		case 1:
		{
			//******************************获取参数*********************************
			memset(&data, 0, sizeof(data));
			if(cI2C_ReadBytes(p_i2c_obj, SW3516_VOUT1_ADDR, data, sizeof(data)) <= 0)
			{
				if(uc_lost_cnt[ch] < 0xff) uc_lost_cnt[ch]++;
				break;
			}
			else 
				uc_lost_cnt[ch] = 0;
			buff[0] = data[1];
			
			memset(&data, 0, sizeof(data));
			if(cI2C_ReadBytes(p_i2c_obj, SW3516_VOUT2_ADDR, data, sizeof(data)) <= 0)
			{
				if(uc_lost_cnt[ch] < 0xff) uc_lost_cnt[ch]++;
				break;
			}
			else 
				uc_lost_cnt[ch] = 0;
			buff[1] = data[1];

			memset(&data, 0, sizeof(data));
			if(cI2C_ReadBytes(p_i2c_obj, SW3516_PD_IOUT1_ADDR, data, sizeof(data)) <= 0)
			{
				if(uc_lost_cnt[ch] < 0xff) uc_lost_cnt[ch]++;
				break;
			}
			else 
				uc_lost_cnt[ch] = 0;
			buff[2] = data[1];
			
			memset(&data, 0, sizeof(data));
			if(cI2C_ReadBytes(p_i2c_obj, SW3516_PD_IOUT2_ADDR, data, sizeof(data)) <= 0)
			{
				if(uc_lost_cnt[ch] < 0xff) uc_lost_cnt[ch]++;
				break;
			}
			else 
				uc_lost_cnt[ch] = 0;
			buff[3] = data[1];
			
			memset(&data, 0, sizeof(data));
			if(cI2C_ReadBytes(p_i2c_obj, SW3516_QC_IOUT1_ADDR, data, sizeof(data)) <= 0)
			{
				if(uc_lost_cnt[ch] < 0xff) uc_lost_cnt[ch]++;
				break;
			}
			else 
				uc_lost_cnt[ch] = 0;
			buff[4] = data[1];
			
			memset(&data, 0, sizeof(data));
			if(cI2C_ReadBytes(p_i2c_obj, SW3516_QC_IOUT2_ADDR, data, sizeof(data)) <= 0)
			{
				if(uc_lost_cnt[ch] < 0xff) uc_lost_cnt[ch]++;
				break;
			}
			else 
				uc_lost_cnt[ch] = 0;
			buff[5] = data[1];
			
			p_ic->usVolt = (buff[0] << 8) | buff[1];    //mV
			if(p_ic->usVolt >= 100)
				p_ic->usVolt -= 100;
			
			p_ic->usPdCurr = (buff[2] << 8) | buff[3];    //mA
			if(p_ic->usPdCurr >= 255)
				p_ic->usPdCurr -= 255;
			
			p_ic->usQcCurr = (buff[4] << 8) | buff[5];    //mA
			if(p_ic->usQcCurr >= 255)
				p_ic->usQcCurr -= 255;
			
			//***************************************处理数据******************************************
			p_ic->usPdPwr = (p_ic->usPdCurr / 1000.0f) * (p_ic->usVolt / 1000.0f);
			p_ic->usQcPwr = (p_ic->usQcCurr / 1000.0f) * (p_ic->usVolt / 1000.0f);
			p_ic->sPower = p_ic->usPdPwr + p_ic->usQcPwr;
			us_usb_total_out_pwr += p_ic->sPower;
			
			uc_index[ch] = 0;
			
			if(uPrint.tFlag.bUsbTask)
				sMyPrint("bUsbTask:SW3518[%d]电压 = %dmV, 功率 = %dW \r\n", ch, p_ic->usVolt, p_ic->sPower);
		}
		break;

		default:
			uc_index[ch] = 0;
			break ;
	}

	//处理状态
	if(uc_lost_cnt[ch] >= 16)  //标志
	{
		if(ch == 0)
		{
			if(tUsb.uErrCode.tCode.bIc1Lost == 0)
			{
				bUsb_SetErrCode(UEC_IC1_LOST,true);
			
				if(uPrint.tFlag.bUsbTask || uPrint.tFlag.bImportant)
					log_e("bUsbTask:IC1丢失");
			}
		}
		else
		{
			if(tUsb.uErrCode.tCode.bIc2Lost == 0)
			{
				bUsb_SetErrCode(UEC_IC2_LOST,true);
			
				if(uPrint.tFlag.bUsbTask || uPrint.tFlag.bImportant)
					log_e("bUsbTask:IC2丢失");
			}
		}
		return -1;
	}
	else if(!uc_lost_cnt[ch])  //清除
	{
		if(ch == 0)
		{
			if(tUsb.uErrCode.tCode.bIc1Lost == 1)
				bUsb_SetErrCode(UEC_IC1_LOST,false);
		}
		else
		{
			if(tUsb.uErrCode.tCode.bIc2Lost == 1)
				bUsb_SetErrCode(UEC_IC2_LOST,false);
		}
		return 1;
	}
	else
		return 0;
}

/*****************************************************************************************************************
-----函数功能    指令:开关MPPT
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
s8 c_usb_set_pwr_cs(u16 pwr)
{
	// if(c_usb_data_trans(modbusWRITE_SINGLE_REG, 
	// 					usbREG_ADDR_SET_CHG_PWR, 
	// 					(u8*)&pwr, 
	// 					1) <= 0)
	// 	return false;
	
	return 1;
}

/***********************************************************************************************************************
-----函数功能	数据传输
-----说明(备注) 
-----传入参数	cmd:指令
				data:指向数据指针
				len:数据的长度
-----输出参数	none
-----返回值		-1:写入的Len超出最大长度
				-2:等会回复超时
				-3:数据发送错误
				0:无操作
				1:操作成功
************************************************************************************************************************/
// static s8 c_usb_data_trans(u8 cmd, u16 reg_addr, u8* data, u8 len)
// {
// 	s8 result = 0;
	
// 	if(dcacSemaphoreMutex == NULL)
// 		return 0;
	
// 	if(tpProtoTx == NULL)
// 		return 0;
	
// 	//开始互斥
// 	#if(boardUSE_OS)
// 	if(xSemaphoreTake(dcacSemaphoreMutex, pdMS_TO_TICKS(1000)) == pdFAIL)
// 		return -99;
// 	#endif  //boardUSE_OS
	
// 	//开始发送
// 	#if(boardDCAC_EN)
// 	result = cModbus_ProtoCreate(tpProtoTx, cmd, reg_addr, data, len);
// 	if(result > 0)
// 	{
// 		if(bDcac_DataSendStart(tpProtoTx->ucaFrameData, tpProtoTx->ucFrameLen) == true)
// 		{
// 			//等待任务通知,等待时间为1S
// 			#if(boardUSE_OS)
// 			if(ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(usbWAIT_NOTIFY_OUTTIME)) <= 0) 
// 			{
// 				if(uPrint.tFlag.bUsbTask)
// 					log_w("bUsbTask:等待指令0x%x,地址0x%x回复超时", cmd, reg_addr);
				
// 				result = -2;
// 			}
// 			#endif  //boardUSE_OS
// 		}
// 		else 
// 			result = -3;
// 	}
// 	#endif
	
// 	cModbus_ResetTx(tpUsbProtoTx, usbTX_PROTO_BUFF_LEN);
	
// 	//释放互斥量
// 	#if(boardUSE_OS)
// 	xSemaphoreGive(dcacSemaphoreMutex);
// 	#endif  //boardUSE_OS
	
// 	return result;
// }

#endif  //boardUSB_EN

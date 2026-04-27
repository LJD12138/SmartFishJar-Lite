#ifndef UPDATA_H_
#define UPDATA_H_

#include "board_config.h"

#if(boardUPDATA)
#include "Updata/xmodem_proto.h"
#include "Updata/baiku_proto.h"

extern Xmodem_T tXmodem;
extern BaiKuProto_T tBaiKuProto;

#endif //#if(boardUPDATA)

#endif //UPDATA_H_



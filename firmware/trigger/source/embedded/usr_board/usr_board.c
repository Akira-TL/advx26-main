#include "tal_api.h"

#include "pn532_i2c.h"
#include "usr_board.h"

OPERATE_RET usr_register_hardware(void)
{
    return pn532_i2c_init();
}

#ifndef MODBUS_H
#define MODBUS_H

#define MAX_MSG_LENGTH 260

///Function Code
enum{
    READ_COILS       = 0x01,
    READ_INPUT_BITS  = 0x02,
    READ_REGS        = 0x03,
    READ_INPUT_REGS  = 0x04,
    WRITE_COIL       = 0x05,
    WRITE_REG        = 0x06,
    WRITE_COILS      = 0x0F,
    WRITE_REGS       = 0x10,
};

///Exception Codes
enum {
    EX_ILLEGAL_FUNCTION = 0x01, // Function Code not Supported
    EX_ILLEGAL_ADDRESS  = 0x02, // Output Address not exists
    EX_ILLEGAL_VALUE    = 0x03, // Output Value not in Range
    EX_SERVER_FAILURE   = 0x04, // Slave Deive Fails to process request
    EX_ACKNOWLEDGE      = 0x05, // Service Need Long Time to Execute
    EX_SERVER_BUSY      = 0x06, // Server Was Unable to Accept MB Request PDU
    EX_GATEWAY_PROBLEMP = 0x0A, // Gateway Path not Available
    EX_GATEWYA_PROBLEMF = 0x0B, // Target Device Failed to Response
};
/*
void modbus_read(int address, int amount, int func){
    uint8_t to_send[12];
    modbus_build_request(to_send, address, func);
    to_send[5] = 6;
    to_send[10] = (uint8_t) (amount >> 8);
    to_send[11] = (uint8_t) (amount & 0x00FF);
    modbus_send(to_send, 12);
}

void modbus_build_request(uint8_t *to_send, int address, int func) {
    to_send[0] = (uint8_t) _msg_id >> 8;
    to_send[1] = (uint8_t) (_msg_id & 0x00FF);
    to_send[2] = 0;
    to_send[3] = 0;
    to_send[4] = 0;
    to_send[6] = (uint8_t) _slaveid;
    to_send[7] = (uint8_t) func;
    to_send[8] = (uint8_t) (address >> 8);
    to_send[9] = (uint8_t) (address & 0x00FF);
}

ssize_t modbus_send(uint8_t *to_send, int length) {
    _msg_id++;
    return send(_socket, to_send, (size_t)length, 0);
}
*/

#endif // MODBUS_H

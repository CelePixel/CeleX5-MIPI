#ifndef __CYPRESS_H__
#define __CYPRESS_H__

#include "USBIR.h"

typedef enum
{
    CTRL_I2C_SET_REG  = 0x100,
    CTRL_I2C_GET_REG  = 0x200,
    CTRL_MIPI_SET_REG = 0x300,
    CTRL_MIPI_GET_REG = 0x400,
}CONTROL_COMMAND;

class Cypress : public USBIR
{
    public:
        Cypress();
        virtual ~Cypress();

		bool StreamOn(void);
		bool open_usb(void); //added by xiaoqin @2018.11.02
		bool open_stream(void); //added by xiaoqin @2018.11.02

        void StreamOff(void);
		void close_usb(void); //added by xiaoqin @2018.11.07
		void close_stream(void); //added by xiaoqin @2018.11.07

        bool USB_Set(uint16_t wId, uint16_t Reg, uint16_t Value);
        bool USB_Get(uint16_t wId, uint16_t Reg, uint16_t &Value);

    protected:

    private:
};

#endif // __CYPRESS_H__

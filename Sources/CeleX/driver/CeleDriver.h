#ifndef CELEPIXEL_H
#define CELEPIXEL_H

#include <stdint.h>
#include <vector>

#ifdef __linux__
using namespace std;

class CeleDriver
#else
#ifdef FRONTPANEL_EXPORTS
#define CELEPIXEL_DLL_API __declspec(dllexport)
#else
#define CELEPIXEL_DLL_API __declspec(dllimport)
#endif

using namespace std;

class CELEPIXEL_DLL_API CeleDriver
#endif
{
public:
	CeleDriver(void);
    ~CeleDriver(void);

public:
    bool Open(void);
	bool openUSB(); //added by xiaoqin @2018.11.02
	bool openStream(); //added by xiaoqin @2018.11.02

    void Close(void);
	void closeUSB(); //added by xiaoqin @2018.11.07
	void closeStream(); //added by xiaoqin @2018.11.07

    bool getimage(vector<uint8_t> &image);
	bool getPackage(unsigned char* buffer, uint32_t* length);
	void clearData();

public:
    bool i2c_set(uint16_t reg,uint16_t value);
    bool i2c_get(uint16_t reg,uint16_t &value);
    bool mipi_set(uint16_t reg,uint16_t value);
    bool mipi_get(uint16_t reg,uint16_t &value);
};

#endif // CELEPIXEL_H

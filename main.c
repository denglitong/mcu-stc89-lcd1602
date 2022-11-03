#include <reg52.h>

// data bus 数据总线
#define LCD1602_DB P0

// Enable
sbit LCD1602_E = P1 ^ 5;   
// Read/Write
sbit LCD1602_RW = P1 ^ 1;
// Record/inStruction
sbit LCD1602_RS = P1 ^ 0;

void InitLcd1602();
void LcdSetCursor(unsigned char x, unsigned char y);
void LcdShowStr(unsigned char x, unsigned char y, unsigned char *str);

void ConfigUART(unsigned int baud);
unsigned char DATA_CNT = 0;

void main() {
	unsigned char str[] = "ABCDEFGHIJKLMNOPQRST";

	InitLcd1602();

	EA = 1; // enable global interrupt
	ConfigUART(9600);

	// 因为教学班子上使用串口来烧录程序+液晶显示屏读写指令/数据
	// 所以第一次点亮的时候液晶显示屏的显示不是清空的，
	// 再重启一次板子就可以清空了
	LcdSetCursor(0, 0);

	// LcdShowStr(0, 0, str);
	// LcdShowStr(0, 1, "`!@#$%^&*()_+");

	while (1) {
		if (DATA_CNT == 16) {
			LcdSetCursor(0, 1);
		}
		if (DATA_CNT >= 32) {
			DATA_CNT = 0;
			// 数据指针清零，所有显示清零
			LcdWriteCmd(0x01);
			// LcdSetCursor(0, 0);
		}
		
	}
}

void LcdWaitReady() {
	unsigned char sta = 0xFF;

	// 1. read instruction
	LCD1602_RW = 1;
	LCD1602_RS = 0;

	// disable default, in case the P0 output affter other circle
	LCD1602_DB = 0xFF;
	LCD1602_E = 0;

	// wait until STA7 is 0 (not busy, then we can send instruction/data)
	do {
		LCD1602_E = 1;
		sta = LCD1602_DB;
		// after we have read the data, we should turn off P0 immediately
		// in case the P0 output affect other circle
		LCD1602_E = 0;
	} while (sta & 0x80);
}

void LcdWriteCmd(unsigned char cmd) {
	LcdWaitReady();
	LCD1602_RW = 0;
	LCD1602_RS = 0;
	LCD1602_DB = cmd;
	LCD1602_E = 1;
	LCD1602_E = 0;
}

void InitLcd1602() {
	// 设置 16x2 显示，5x7点阵，8位数据接口
	LcdWriteCmd(0x38);
	// 设置显示开，不显示光标
	LcdWriteCmd(0x0D);
	// 当读写一个字符后地址指针加一，且光标加一
	LcdWriteCmd(0x06);
	// 数据指针清零，所有显示清零
	LcdWriteCmd(0x01);
}

// 00 01 02 03 ... 0E 0F
// 40 41 42 43 ... 4E 4F
void LcdSetCursor(unsigned char x, unsigned char y) {
	unsigned char addr;
	if (y == 0) {
		addr = 0x00 + x;
	} else {
		addr = 0x40 + x;
	}
	// 设置数据地址指针
	LcdWriteCmd(0x80 | addr);
}

void LcdWriteData(unsigned char dat) {
	LcdWaitReady();
	LCD1602_RW = 0;
	LCD1602_RS = 1;
	LCD1602_DB = dat;
	LCD1602_E = 1;
	LCD1602_E = 0;
}

void LcdShowStr(unsigned char x, unsigned char y, unsigned char *str) {
	LcdSetCursor(x, y);
	while (*str != '\0') {
		LcdWriteData(*str);
		str++;
	}
}

void ConfigUART(unsigned int baud) {
	// SCON 0101 0000
	// SM1=1, REN=1，串行通信模式1
	SCON = 0x50;
	TMOD &= 0x0F; // Timer1
	TMOD |= 0x20; // Reload mode
	TH1 = 256 - (11059200/12/32)/baud;
	TL1 = TH1;
	ET1 = 0; // 使能T1作为波特率发生器，而禁止T1用作定时器
	ES = 1; // 使能串口中断
	TR1 = 1; // start T1
}

void InterruptUART() interrupt 4 {
	// 如果是接收中断标志位
	if (RI) {
		RI = 0;	
		SBUF = SBUF;
		// LcdSetCursor(DATA_CNT%16, DATA_CNT/16);
		LcdWriteData(SBUF);
		
		DATA_CNT++;
	}
	// 如果是发送中断标志位
	if (TI) {
		TI = 0;
	}
}
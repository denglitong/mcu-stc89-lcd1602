#include <reg52.h>

// data bus ��������
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

	// ��Ϊ��ѧ������ʹ�ô�������¼����+Һ����ʾ����дָ��/����
	// ���Ե�һ�ε�����ʱ��Һ����ʾ������ʾ������յģ�
	// ������һ�ΰ��ӾͿ��������
	LcdSetCursor(0, 0);

	// LcdShowStr(0, 0, str);
	// LcdShowStr(0, 1, "`!@#$%^&*()_+");

	while (1) {
		if (DATA_CNT == 16) {
			LcdSetCursor(0, 1);
		}
		if (DATA_CNT >= 32) {
			DATA_CNT = 0;
			// ����ָ�����㣬������ʾ����
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
	// ���� 16x2 ��ʾ��5x7����8λ���ݽӿ�
	LcdWriteCmd(0x38);
	// ������ʾ��������ʾ���
	LcdWriteCmd(0x0D);
	// ����дһ���ַ����ַָ���һ���ҹ���һ
	LcdWriteCmd(0x06);
	// ����ָ�����㣬������ʾ����
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
	// �������ݵ�ַָ��
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
	// SM1=1, REN=1������ͨ��ģʽ1
	SCON = 0x50;
	TMOD &= 0x0F; // Timer1
	TMOD |= 0x20; // Reload mode
	TH1 = 256 - (11059200/12/32)/baud;
	TL1 = TH1;
	ET1 = 0; // ʹ��T1��Ϊ�����ʷ�����������ֹT1������ʱ��
	ES = 1; // ʹ�ܴ����ж�
	TR1 = 1; // start T1
}

void InterruptUART() interrupt 4 {
	// ����ǽ����жϱ�־λ
	if (RI) {
		RI = 0;	
		SBUF = SBUF;
		// LcdSetCursor(DATA_CNT%16, DATA_CNT/16);
		LcdWriteData(SBUF);
		
		DATA_CNT++;
	}
	// ����Ƿ����жϱ�־λ
	if (TI) {
		TI = 0;
	}
}
#include <fxcg/display.h>
#include <fxcg/keyboard.h>
#include <fxcg/rtc.h>
#include <fxcg/system.h>
#include <stdlib.h>

unsigned short opcode; //Current opcode
unsigned char memory[4096]; //Memory (4096 bytes)
unsigned char V[16]; //Registers and carry flag
unsigned short I; //Index register
unsigned short pc; //Program Counter
unsigned char gfx[64 * 32];
unsigned char delay_timer;
unsigned char sound_timer;
unsigned short stack[16]; //Stack
unsigned short sp; //StackPointer
unsigned char key[16];
bool drawFlag;

unsigned char chip8_fontset[80] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
unsigned char pong[] = {
	0x22, 0xfc, 0x6b, 0x0c, 0x6c, 0x3f, 0x6d, 0x0c, 0xa2, 0xea, 0xda, 0xb6,
	0xdc, 0xd6, 0x6e, 0x00, 0x22, 0xd4, 0x66, 0x03, 0x68, 0x02, 0x60, 0x60,
	0xf0, 0x15, 0xf0, 0x07, 0x30, 0x00, 0x12, 0x1a, 0xc7, 0x17, 0x77, 0x08,
	0x69, 0xff, 0xa2, 0xf0, 0xd6, 0x71, 0xa2, 0xea, 0xda, 0xb6, 0xdc, 0xd6,
	0x60, 0x01, 0xe0, 0xa1, 0x7b, 0xfe, 0x60, 0x04, 0xe0, 0xa1, 0x7b, 0x02,
	0x60, 0x1f, 0x8b, 0x02, 0xda, 0xb6, 0x60, 0x0c, 0xe0, 0xa1, 0x7d, 0xfe,
	0x60, 0x0d, 0xe0, 0xa1, 0x7d, 0x02, 0x60, 0x1f, 0x8d, 0x02, 0xdc, 0xd6,
	0xa2, 0xf0, 0xd6, 0x71, 0x86, 0x84, 0x87, 0x94, 0x60, 0x3f, 0x86, 0x02,
	0x61, 0x1f, 0x87, 0x12, 0x46, 0x00, 0x12, 0x78, 0x46, 0x3f, 0x12, 0x82,
	0x47, 0x1f, 0x69, 0xff, 0x47, 0x00, 0x69, 0x01, 0xd6, 0x71, 0x12, 0x2a,
	0x68, 0x02, 0x63, 0x01, 0x80, 0x70, 0x80, 0xb5, 0x12, 0x8a, 0x68, 0xfe,
	0x63, 0x0a, 0x80, 0x70, 0x80, 0xd5, 0x3f, 0x01, 0x12, 0xa2, 0x61, 0x02,
	0x80, 0x15, 0x3f, 0x01, 0x12, 0xba, 0x80, 0x15, 0x3f, 0x01, 0x12, 0xc8,
	0x80, 0x15, 0x3f, 0x01, 0x12, 0xc2, 0x60, 0x20, 0xf0, 0x18, 0x22, 0xd4,
	0x8e, 0x34, 0x22, 0xd4, 0x66, 0x3e, 0x33, 0x01, 0x66, 0x03, 0x68, 0xfe,
	0x33, 0x01, 0x68, 0x02, 0x12, 0x16, 0x79, 0xff, 0x49, 0xfe, 0x69, 0xff,
	0x12, 0xc8, 0x79, 0x01, 0x49, 0x02, 0x69, 0x01, 0x60, 0x04, 0xf0, 0x18,
	0x76, 0x01, 0x46, 0x40, 0x76, 0xfe, 0x12, 0x6c, 0xa2, 0xf2, 0xfe, 0x33,
	0xf2, 0x65, 0xf1, 0x29, 0x64, 0x14, 0x65, 0x02, 0xd4, 0x55, 0x74, 0x15,
	0xf2, 0x29, 0xd4, 0x55, 0x00, 0xee, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xc0, 0xc0, 0x00, 0xff, 0x00,
	0x6b, 0x20, 0x6c, 0x00, 0xa2, 0xf6, 0xdb, 0xc4, 0x7c, 0x04, 0x3c, 0x20,
	0x13, 0x02, 0x6a, 0x00, 0x6b, 0x00, 0x6c, 0x1f, 0xa2, 0xfa, 0xda, 0xb1,
	0xda, 0xc1, 0x7a, 0x08, 0x3a, 0x40, 0x13, 0x12, 0xa2, 0xf6, 0x6a, 0x00,
	0x6b, 0x20, 0xdb, 0xa1, 0x00, 0xee
};

void initialize();
void emulateCycle();
int randInt(int* rnd_seed);
void plot(int x0, int y0, int color);
int keydown(int basic_keycode);
void drawGraphics();
void setKeys();

int kektus;

int main()
{
	Bdisp_EnableColor(1);
	initialize();
	while (1)
	{
		emulateCycle();
		if (drawFlag)
			drawGraphics();

		setKeys();
	}
}

void initialize()
{
	pc = 0x200;
	opcode = 0;
	I = 0;
	sp = 0;
	// Clear display	
	for (int i = 0; i < 64 * 32; i++)
	{
		gfx[i] = 0;
	}
	// Clear stack
	for (int i = 0; i < 16; i++)
	{
		stack[i] = 0;
	}
	// Clear registers V0-VF
	for (int i = 0; i < 16; i++)
	{
		V[i] = 0;
	}
	// Clear memory
	for (int i = 0; i < 4096; i++)
	{
		memory[i] = 0;
	}
	// Load fontset
	for (int i = 0; i < 80; ++i)
		memory[i] = chip8_fontset[i];

	for (int i = 0; i < sizeof(pong); i++)
		memory[i + 512] = pong[i];

	drawFlag = false;
}

void emulateCycle()
{
	// Fetch Opcode
	opcode = memory[pc] << 8 | memory[pc + 1];
	//Execute Opcode
	switch (opcode & 0xF000)
	{
	case 0x0000:
		switch (opcode & 0x000F)
		{
		case 0x0000:
			//memset(gfx, 0, 2048);
			memsetZero(gfx, 2048);
			drawFlag = true;
			pc += 2;
			break;
		case 0x000E:
			sp--;
			pc = stack[sp];
			pc += 2;
			break;
		}
		break;
	case 0x1000:
		pc = opcode & 0x0FFF;
		break;
	case 0x2000:
		stack[sp] = pc;
		sp++;
		pc = opcode & 0x0FFF;
		break;
	case 0x3000:
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
		{
			pc += 4;
		}
		else
		{
			pc += 2;
		}
		break;
	case 0x4000:
		if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
		{
			pc += 4;
		}
		else
		{
			pc += 2;
		}
		break;
	case 0x5000:
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00F0) >> 4)
		{
			pc += 4;
		}
		else
		{
			pc += 2;
		}
		break;
	case 0x6000:
		V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
		pc += 2;
		break;
	case 0x7000:
		V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
		pc += 2;
		break;
	case 0x8000:
		switch (opcode & 0x000F)
		{
		case 0x0000:
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		case 0x0001:
			V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		case 0x0002:
			V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		case 0x0003:
			V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		case 0x0004:
			if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
			{
				V[0xF] = 1; //carry
			}
			else
			{
				V[0xF] = 0;
			}
			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		case 0x0005:
			if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
			{
				V[0xF] = 0;
			}
			else
			{
				V[0xF] = 1;
			}
			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		case 0x0006:
			V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
			V[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
			break;
		case 0x0007:
			if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
			{
				V[0xF] = 1;
			}
			else
			{
				V[0xF] = 0;
			}
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;
		case 0x000E:
			V[0xF] = V[(opcode & 0x0F00)] >> 7;
			V[(opcode & 0x0F00) >> 8] <<= 1;
			break;
		}
		break;
	case 0x9000:
		if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
		{
			pc += 4;
		}
		else
		{
			pc += 2;
		}
		break;
	case 0xA000:
		I = opcode & 0x0FFF;
		pc += 2;
		break;
	case 0xB000:
		pc = (opcode & 0x0FFF) + V[0];
		break;
	case 0xC000:
	{
		int seed = RTC_GetTicks() + (GetMainBatteryVoltage(1) % 100);
		V[(opcode & 0x0F00) >> 8] = (randInt(&seed) % 0xFF) & (opcode & 0x00FF);
		pc += 2;
		break;
	}
	case 0xD000:
	{
		unsigned short x = V[(opcode & 0x0F00) >> 8];
		unsigned short y = V[(opcode & 0x00F0) >> 4];
		unsigned short height = opcode & 0x000F;
		unsigned short pixel;

		V[0xF] = 0;
		for (int yline = 0; yline < height; yline++)
		{
			pixel = memory[I + yline];
			for (int xline = 0; xline < 8; xline++)
			{
				if ((pixel & (0x80 >> xline)) != 0)
				{
					if (gfx[(x + xline + ((y + yline) * 64))] == 1)
					{
						V[0xF] = 1;
					}
					gfx[x + xline + ((y + yline) * 64)] ^= 1;
				}
			}
		}

		drawFlag = true;
		pc += 2;
	}
	break;
	case 0xE000:
		switch (opcode & 0x00FF)
		{
		case 0x009E:
			if (key[V[(opcode & 0x0F00) >> 8]] != 0)
			{
				pc += 4;
			}
			else
			{
				pc += 2;
			}
			break;
		case 0x00A1:
			if (key[V[(opcode & 0x0F00) >> 8]] == 0)
			{
				pc += 4;
			}
			else
			{
				pc += 2;
			}
			break;
		}
		break;
	case 0xF000:
		switch (opcode & 0x00FF)
		{
		case 0x0007:
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;
		case 0x000A:
		{
			bool keyPress = false;

			for (int i = 0; i < 16; ++i)
			{
				if (key[i] != 0)
				{
					V[(opcode & 0x0F00) >> 8] = i;
					keyPress = true;
				}
			}

			// If we didn't received a keypress, skip this cycle and try again.
			if (!keyPress)
				return;

			pc += 2;
			break;
		}
		case 0x0015:
			delay_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;
		case 0x0018:
			sound_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;
		case 0x001E:
			if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
			{
				V[0xF] = 1;
			}
			else
			{
				V[0xF] = 0;
			}

			I += V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;
		case 0x0029:
			I = V[(opcode & 0x0F00) >> 8] * 0x5;
			pc += 2;
			break;
		case 0x0033:
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
			pc += 2;
			break;
		case 0x0055:
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				memory[I + i] = V[i];

			// On the original interpreter, when the operation is done, I = I + X + 1.
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;
		case 0x0065:
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				V[i] = memory[I + i];

			// On the original interpreter, when the operation is done, I = I + X + 1.
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;
		}
		break;
	}

	//Update timers
	if (delay_timer > 0)
		--delay_timer;
	if (sound_timer > 0)
	{
		//if(sound_timer == 1)
			//Beep
		--sound_timer;
	}
}

int randInt(int* rnd_seed)
{
	int k1;
	int ix = *rnd_seed;

	k1 = ix / 127773;
	ix = 16807 * (ix - k1 * 127773) - k1 * 2836;
	if (ix < 0)
		ix += 2147483647;
	*rnd_seed = ix;
	return *rnd_seed;
}
void plot(int x0, int y0, int color) {
	char* VRAM = (char*)0xA8000000;
	VRAM += 2 * (y0*LCD_WIDTH_PX + x0);
	*(VRAM++) = (color & 0x0000FF00) >> 8;
	*(VRAM++) = (color & 0x000000FF);
	return;
}
void setKeys()
{

	/* Original Keypad
	1	2	3	C
	4	5	6	D
	7	8	9	E
	A	0	B	F
	*/
	/* Prizm Keypad
	7   8   9     DEL
	4   5   6     X
	1   2   3     +
	0   .   EXP   (-)
	*/
	key[0x1] = (keydown(KEY_PRGM_7)) ? true : false;
	key[0x2] = (keydown(KEY_PRGM_8)) ? true : false;
	key[0x3] = (keydown(KEY_PRGM_9)) ? true : false;
	key[0x4] = (keydown(KEY_PRGM_4)) ? true : false;
	key[0x5] = (keydown(KEY_PRGM_5)) ? true : false;
	key[0x6] = (keydown(KEY_PRGM_6)) ? true : false;
	key[0x7] = (keydown(KEY_PRGM_1)) ? true : false;
	key[0x8] = (keydown(KEY_PRGM_2)) ? true : false;
	key[0x9] = (keydown(KEY_PRGM_3)) ? true : false;
	key[0xA] = (keydown(KEY_PRGM_0)) ? true : false;
	key[0] = (keydown(KEY_CHAR_COMMA)) ? true : false;
	key[0xB] = (keydown(KEY_CHAR_EXP)) ? true : false;
	key[0xC] = (keydown(KEY_CTRL_DEL)) ? true : false;
	key[0xD] = (keydown(KEY_CHAR_MULT)) ? true : false;
	key[0xE] = (keydown(KEY_CHAR_PLUS)) ? true : false;
	key[0xF] = (keydown(KEY_CHAR_MINUS)) ? true : false;
	if (keydown(KEY_PRGM_MENU))
		GetKey(&kektus);
}
void drawGraphics()
{
	for (int y = 0; y < 32; ++y)
	{
		for (int x = 0; x < 64; ++x)
		{
			if (gfx[(y * 64) + x] == 0) 
			{
				plot(x + 50, y + 50, COLOR_BLACK);
// 				plot(x + 50 + 1, y + 50, COLOR_BLACK);
// 				plot(x + 50, y + 50 + 1, COLOR_BLACK);
// 				plot(x + 50 + 1, y + 50 + 1, COLOR_BLACK);
			}
			else
			{
				plot(x + 50, y + 50, COLOR_WHITE);
// 				plot(x + 50 + 1, y + 50, COLOR_WHITE);
// 				plot(x + 50, y + 50 + 1, COLOR_WHITE);
// 				plot(x + 50 + 1, y + 50 + 1, COLOR_WHITE);
			}
		}
	}
	Bdisp_PutDisp_DD();
	drawFlag = false;
}

int keydown(int basic_keycode)
{
	const unsigned short* keyboard_register = (unsigned short*)0xA44B0000;
	int row, col, word, bit;
	row = basic_keycode % 10;
	col = basic_keycode / 10 - 1;
	word = row >> 1;
	bit = col + 8 * (row & 1);
	return (0 != (keyboard_register[word] & 1 << bit));
}
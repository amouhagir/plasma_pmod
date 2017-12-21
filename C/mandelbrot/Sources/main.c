#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "../../shared/plasmaCoprocessors.h"
#include "../../shared/plasmaIsaCustom.h"
#include "../../shared/plasmaMisc.h"
#include "../../shared/plasmaSoPCDesign.h"
#include "../../shared/plasmaMyPrint.h"
#include "../../shared/plasma.h"

//#define COLOR

#define Nf 18
#define Nr 12

#define RIGHT_BUTTON  0x00000010
#define LEFT_BUTTON   0x00000008
#define DOWN_BUTTON   0x00000004
#define UP_BUTTON     0x00000002
#define CENTER_BUTTON 0x00000001

int Convergence(int x_C, int y_C, int Imax) {
    long long int x = 0;
    long long int y = 0;
    int x2 = (x * x) >> Nf;
    int y2 = (y * y) >> Nf;
    int iter = 0;
    int x_new;
    int quatre = 4 << Nf;
    int mod2 = x2 + y2;

    /*while( (mod2 <= quatre) && ( iter < Imax) )
    {

      x_new = isa_custom_2(x, y) + x_C;

      y = isa_custom_1(x,y) + y_C;

      x = x_new;

      mod2 = isa_custom_3(x,y);
      iter++;
    }*/

    while ((mod2 <= quatre) && (iter < Imax)) {
        x_new = x2 - y2 + x_C;
        y = 2 * ((x * y) >> Nf) + y_C;
        x = x_new;
        x2 = (x * x) >> Nf;
        y2 = (y * y) >> Nf;
        mod2 = x2 + y2;
        iter++;
    }


    return iter;
}

void generate_mandelbrot(int H, int W, int Imax, int FS, int x_A, int y_A, int x_B, int y_B)
{

	int x_C;
	int y_C;

	int dx = (x_B - x_A); // A(Ni+1,Nf)
  	int dy = (y_B - y_A); // U(Ni+1,Nf)
  	long int dx64 = dx;
  	long int dy64 = dy;

	dx64 = ((dx64<<Nr) / W) >> Nr; // A(Ni+1,Nf+12)
  	dy64 = ((dy64<<Nr) / H) >> Nr; // A(Ni+1,Nf+12)
  	dx = dx64;
  	dy = dy64;

	int buff, pixel;
/*
	MemoryWrite(OLED_RST, 1); // reset the oled_rgb
	for (int py = 0; py < H; py++)
	{
		for (int px = 0; px < W; px++)
		{
			buff = px;
			buff = (buff << 6) | py;
			buff = (buff << 16) | 31 ; // blue screen
			MemoryWrite(OLED_RW, buff);
		}
	}
*/
	//MemoryWrite(OLED_RST, 1); // reset the oled_rgb
	for(int py = 0; py < H; py++)
	{
		y_C = y_A + dy*py;

		for(int px = 0; px < W; px ++){

			x_C = x_A + dx*px;

			int i = Convergence(x_C, y_C, Imax);
			//coproc_write(COPROC_1_RW, x_C);
			//coproc_write(COPROC_1_RW, y_C);

			i = (0x0000FFFF) & i;

			//pixel = ((i >> 3) << 11 | (i >> 2) << 6 | (i >> 3));
			pixel = i;
			//if(i == Imax) pixel = 0xFFFF;
			//else pixel = 0;

			buff = px;
			buff = (buff << 6) | py;
			buff = (buff << 16) | pixel;
			MemoryWrite(OLED_RW, buff);
		}
	}
}

int main(int argc, char **argv) {

	int x_C, y_C, Imax, iter;

	x_C = 1548;
	y_C = 3141;
	Imax = 255;

	my_printf("x_C=", x_C);
	my_printf("y_C=", y_C);
	my_printf("I_max=", Imax);

	iter = Convergence(x_C, y_C, Imax);

	my_printf("iter = ", iter);


/*	int H = 64; // H of the image in pixel
	int W = 96; // W of the image in pixel

	int Imax = 255;
	int FS = 65535; // RGB full scale

	int buff;

	int x_A = ~(3 << Nf - 2) + 1; // -1.75 au format A(31-Nf,Nf)
	int y_A = ~(3 << Nf - 2) + 1; // -1.5 au format A(31-Nf,Nf)
	int x_B = (3 << Nf - 2); // 0.75 au format A(31-Nf,Nf)
	int y_B = (3 << Nf - 2); // 1.5 au format A(31-Nf,Nf)

	MemoryWrite(OLED_RST, 1); // reset the oled_rgb

	coproc_reset(COPROC_1_RST);

	generate_mandelbrot(H, W, 255, 255, x_A, y_A, x_B, y_B);
	
	while(1)
	{
		//puts("\nPlease press center button when ready\n");

		//while(MemoryRead(BUTTONS_CHANGE) == 0){} // loop while no event on the buttons 
		//while(MemoryRead(BUTTONS_VALUES) != CENTER_BUTTON){} // after an event if center button is pushed
		//while(MemoryRead(BUTTONS_CHANGE) == 0){} // wait for button release

		int sw;
		int DX, DY;

		puts("\nReady, push a button to move/zoom\n");
		while(MemoryRead(BUTTONS_CHANGE) == 0){}
		int buttons = MemoryRead(BUTTONS_VALUES);
		while(MemoryRead(BUTTONS_CHANGE) == 0){}
	
		sw = MemoryRead(CTRL_SL_READ);

		switch(buttons)
		{
			case(CENTER_BUTTON):
				if((sw&1) == 1)
				{
					printf("zoom in\n");
					DX = x_B - x_A;
					DY = y_B - y_A;
					x_A = x_A + (DX >> 2);
					y_A = y_A + (DY >> 2);
					x_B = x_B - (DX >> 2);
					y_B = y_B - (DY >> 2);
					//printf("x_A,=%d, y_A=%d, x_B=%f, y_B=%f\n",x_A,y_A,x_B,y_B);
					generate_mandelbrot(H, W, Imax, 255, x_A, y_A, x_B, y_B);
				}
				else if((sw&1) == 0)
				{
					printf("zoom out\n");
					DX = x_B - x_A;
					DY = y_B - y_A;
					x_A = x_A - (DX >> 1);
					y_A = y_A - (DY >> 1);
					x_B = x_B + (DX >> 1);
					y_B = y_B + (DY >> 1);
					//printf("x_A,=%f, y_A=%f, x_B=%f, y_B=%f",x_A,y_A,x_B,y_B);
					generate_mandelbrot(H, W, Imax, 255, x_A, y_A, x_B, y_B);
				}
				else
				{
					break;
				}
				break;
			case(DOWN_BUTTON):
				printf("move down\n");
				DY = y_B - y_A;
				y_A = y_A + (DY >> 3);
				y_B = y_B + (DY >> 3);
				//printf("x_A,=%f, y_A=%f, x_B=%f, y_B=%f",x_A,y_A,x_B,y_B);
				generate_mandelbrot(H, W, Imax, 255, x_A, y_A, x_B, y_B);
			break;
			case(UP_BUTTON):
				printf("move up\n");
				DY = y_B - y_A;
				y_A = y_A - (DY >> 3);
				y_B = y_B - (DY >> 3);
				//printf("x_A,=%f, y_A=%f, x_B=%f, y_B=%f",x_A,y_A,x_B,y_B);
				generate_mandelbrot(H, W, Imax, 255, x_A, y_A, x_B, y_B);
			break;
			case(LEFT_BUTTON):
				printf("move left\n");
				DX = x_B - x_A;
				x_A = x_A - (DX >> 3);
				x_B = x_B - (DX >> 3);
				//printf("x_A,=%f, y_A=%f, x_B=%f, y_B=%f",x_A,y_A,x_B,y_B);
				generate_mandelbrot(H, W, Imax, 255, x_A, y_A, x_B, y_B);
			break;
			case(RIGHT_BUTTON):
				printf("move right\n");
				DX = x_B - x_A;
				x_A = x_A + (DX >> 3);
				x_B = x_B + (DX >> 3);
				//printf("x_A,=%f, y_A=%f, x_B=%f, y_B=%f",x_A,y_A,x_B,y_B);
				generate_mandelbrot(H, W, Imax, 255, x_A, y_A, x_B, y_B);
			break;
		}

	}
*/
}



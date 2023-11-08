#include <8051.h>
 
void delay(void);
 
void main( void )
{
    unsigned short int i ;
	
    P2 = 0x00;
 
    while (1)
    {
        for( i = 0; i < 4; i++)
        {
            P2 = (1 << i) | (1 << (7-i));
            delay();
        }
    }
}

void delay(void)
{
    int i,j;
	
    for(i=0;i<0xff;i++)
         for(j=0;j<0x3f;j++)
		;
}
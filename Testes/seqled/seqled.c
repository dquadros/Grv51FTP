#include <8051.h>
 
void delay(void);
 
void main( void )
{
    unsigned short int i ;
	
    P2 = 0x00;
 
    while (1)
    {
        for( i = 0; i < 8; i++)
        {
            P2 = 1 << i;
            delay();
        }
        for( i = 6; i > 0; i--)
        {
            P2 = 1 << i;
            delay();
        }
    }
}

void delay(void)
{
    int i,j;
	
    for(i=0;i<0xff;i++)
         for(j=0;j<0xff;j++)
		;
}
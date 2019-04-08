//Guess for the bluetooth module is 9600 baud

//Flow controll disabeled
//8+1
//PLLFOUT is 144MHz to get 9600baud we need a divider of:
//(144 000 000)/(9 600)=(15 000)
//we chose Mode 1 over Mode 0 because (15000%16!=0)
//DIVIDER_X =14, BRD=998
//DIVIDER_X=0xE
//BRD=0x3E6
//-> Baudrate (144 000 000)/((DIVIDER_X+1)*(BRD+2))=9600

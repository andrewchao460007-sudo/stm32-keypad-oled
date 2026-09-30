#include "keypad.h"

void Keypad_Pin_Init(){
    // Enable GPIO clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;	
	
	// Setting PC 0, 1, 2, 3 as GPIO output
	// GPIO Mode: Input(00), Output(01), AlterFunc(10), Analog(11, reset)
	// Use the values " GPIO_MODER_MODE0  , GPIO_MODER_MODE1 , GPIO_MODER_MODE2 , GPIO_MODER_MODE3"  to set the GPIO pins 0,1,2,3 as output
	GPIOC -> MODER &=~ (GPIO_MODER_MODE0  | GPIO_MODER_MODE1 | GPIO_MODER_MODE2 | GPIO_MODER_MODE3); //clear the GPIOC MODER register here
  GPIOC->MODER |= (GPIO_MODER_MODE0_0  | GPIO_MODER_MODE1_0 | GPIO_MODER_MODE2_0 | GPIO_MODER_MODE3_0);// Set the GPIOC MODER to output here 
	
	// Setting PC 4, 10, 11, 12 as GPIO input
	// GPIO Mode: Input(00), Output(01), AlterFunc(10), Analog(11, reset)
	// Use the values " GPIO_MODER_MODE4  , GPIO_MODER_MODE10 , GPIO_MODER_MODE11 , GPIO_MODER_MODE12"  to set the GPIO pins 4,10,11, 12 as input
	GPIOC -> MODER &=~ (GPIO_MODER_MODE4  | GPIO_MODER_MODE10 | GPIO_MODER_MODE11 | GPIO_MODER_MODE12);
	//Set the GPIOC MODER to Input here

	// GPIO Output Type: Output push-pull (0, reset), Output open drain (1) 
	//Use the values "GPIO_IDR_ID0, GPIO_IDR_ID1, GPIO_IDR_ID2 , GPIO_IDR_ID3 to set to OPEN DRAIN "
	GPIOC->OTYPER |= (GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1 | GPIO_OTYPER_OT2| GPIO_OTYPER_OT3); // Set the GPIOC OTYPER to Open-drain here
	
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD4 | GPIO_PUPDR_PUPD10 | GPIO_PUPDR_PUPD11 | GPIO_PUPDR_PUPD12);
  GPIOC->PUPDR |= (GPIO_PUPDR_PUPD4_0 | GPIO_PUPDR_PUPD10_0 | GPIO_PUPDR_PUPD11_0 | GPIO_PUPDR_PUPD12_0);

}

unsigned char keypad_scan(){
    unsigned char row, col;
	unsigned char key;
	
	unsigned char keymap[4][4] = {
		{'1','2','3','A'},
		{'4','5','6','B'},
		{'7','8','9','C'},
		{'*','0','#','D'}
		
		//fill out other rows for the keypad keymap here
	};
	
	uint32_t inputMask  = GPIO_IDR_ID4 | GPIO_IDR_ID10 | GPIO_IDR_ID11 | GPIO_IDR_ID12;
  uint32_t outputMask = GPIO_ODR_OD0 | GPIO_ODR_OD1  | GPIO_ODR_OD2  | GPIO_ODR_OD3;
	
  uint32_t outputs[4] = {GPIO_ODR_OD0, GPIO_ODR_OD1,  GPIO_ODR_OD2,  GPIO_ODR_OD3};
	uint32_t inputs[4]  = {GPIO_IDR_ID4, GPIO_IDR_ID10, GPIO_IDR_ID11, GPIO_IDR_ID12};

	//Write 0b0000 to the rows
	GPIOC->ODR &= ~outputMask;
	
	waitms(3); // short delay 

	while( (GPIOC->IDR & inputMask) == inputMask){;}  // Wait until key pressed
	int i,j;
	for(row = 0; row<4 ; row++){ // Row scan , scan each row by row
		GPIOC->ODR |= outputMask;
		GPIOC->ODR &= ~outputs[row];
		
		waitms(3); // short delay 
		for(col = 0 ; col<4; col++){// Column scan , scan each column			
			if((GPIOC->IDR & inputs[col]) == 0){ // check if the specific column went low when a key was pressed
				key = keymap [row][col]; //get the pressed key from the keymap using the row and column you scanned
				while( (GPIOC->IDR & inputMask) != inputMask){;} // Wait until key released
				return key;
			}
		}
	}
	return 0xFF; //return a value when no key is pressed, check manual for waht to return.
}

void waitms(unsigned int ms){
	int i, j;
	for(i = 0; i < ms; i++){
		for(j=0; j < 4000; j++);
	}	
}

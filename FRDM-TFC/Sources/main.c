#include "derivative.h" /* include peripheral declarations */
#include "TFC\TFC.h"

#define FTM1_CLOCK                                                                  (CORE_CLOCK)
#define FTM1_CLK_PRESCALE                                  						     6// Prescale Selector value - see comments in Status Control (SC) section for more details
#define FTM1_OVERFLOW_FREQUENCY 50  // Desired Frequency of PWM Signal - Here 50Hz => 20ms period
// use these to dial in servo steering to your particular servo
#define SERVO_MIN_DUTY_CYCLE                                          (float)(.0010*FTM1_OVERFLOW_FREQUENCY)  // The number here should be be *pulse width* in seconds to move servo to its left limit
#define SERVO_MAX_DUTY_CYCLE                                         (float)(.0020*FTM1_OVERFLOW_FREQUENCY)  // The number here should be be *pulse width* in seconds to move servo to its Right limit

uint16_t * centralDifference(uint16_t * input);

int main(void) {
	uint32_t t, i = 0;

	TFC_Init();

	for (;;) {
		//TFC_Task must be called in your main loop.  This keeps certain processing happy (I.E. Serial port queue check)
		TFC_Task();

		//This Demo program will look at the middle 2 switch to select one of 4 demo modes.
		//Let's look at the middle 2 switches
		switch ((TFC_GetDIP_Switch() >> 1) & 0x03) {
		default:
			TFC_BAT_LED0_ON;
			break;
		case 0:
			//Demo mode 0 just tests the switches and LED's
			if (TFC_PUSH_BUTTON_0_PRESSED)
				TFC_BAT_LED0_ON;
			else
				TFC_BAT_LED0_OFF;

			if (TFC_PUSH_BUTTON_1_PRESSED)
				TFC_BAT_LED3_ON;
			else
				TFC_BAT_LED3_OFF;

			if (TFC_GetDIP_Switch() & 0x01)
				TFC_BAT_LED1_ON;
			else
				TFC_BAT_LED1_OFF;

			if (TFC_GetDIP_Switch() & 0x08)
				TFC_BAT_LED2_ON;
			else
				TFC_BAT_LED2_OFF;

			break;

		case 1:

			//Demo mode 1 will just move the servos with the on-board potentiometers
			if (TFC_Ticker[0] >= 20) {
				TFC_Ticker[0] = 0; //reset the Ticker
				//Every 20 mSeconds, update the Servos
				TFC_SetServo(0, TFC_ReadPot(0));
				//TFC_SetServo(1, TFC_ReadPot(1));
			}
			//Let's put a pattern on the LEDs
			if (TFC_Ticker[1] >= 125) {
				TFC_Ticker[1] = 0;
				t++;
				if (t > 4) {
					t = 0;
				}
				TFC_SetBatteryLED_Level(t);
			}

			TFC_SetMotorPWM(0, 0); //Make sure motors are off
			TFC_HBRIDGE_DISABLE;

			break;

		case 2:

			//Demo Mode 2 will use the Pots to make the motors move
			TFC_HBRIDGE_ENABLE;
			TFC_SetMotorPWM(TFC_ReadPot(0), TFC_ReadPot(1));

			//Let's put a pattern on the LEDs
			if (TFC_Ticker[1] >= 125) {
				TFC_Ticker[1] = 0;
				t++;
				if (t > 4) {
					t = 0;
				}
				TFC_SetBatteryLED_Level(t);
			}
			break;

		case 3:

			//Demo Mode 3 will be in Freescale Garage Mode.  It will beam data from the Camera to the 
			//Labview Application

			if (TFC_Ticker[0] > 100 && LineScanImageReady == 1) {
				TFC_Ticker[0] = 0;
				LineScanImageReady = 0;
				TERMINAL_PRINTF("\r\n");
				TERMINAL_PRINTF("L:");

				TERMINAL_PRINTF("case 3: \r\n");
				if (t == 0)
					t = 3;
				else
					t--;

				TFC_SetBatteryLED_Level(t);

				//the average value of the array
				/*uint16_t averageValue = 0;
				int i;
				for (i = 0; i < 128; i++) {
					averageValue += LineScanImage0[i];
				}

				averageValue = averageValue / 128;
				TERMINAL_PRINTF("AVERAGE0: ");
				TERMINAL_PRINTF("%d", averageValue);*/

				
				
				float Position=0.0;
				TFC_HBRIDGE_ENABLE;
				
				if(LineScanImage0[0] > 2100 && LineScanImage0[0] < 2900){
					Position=0;
					TFC_SetMotorPWM(0,0);
				}else if(LineScanImage0[0]>2900){
					Position=0.7;
					//TFC_SetMotorPWM(0,0.3);
				}
				else if(LineScanImage0[0]<2100){
					Position=-0.5;
					//TFC_SetMotorPWM(0.3,0);
				}
				
				int middle=64;
				int blackLine=blackLineLocation(centralDifference(LineScanImage0));
				TERMINAL_PRINTF("Black Line Location: %d",blackLine);

				//TFC_SetServo(0, Position);
				
				/*for (i = 0; i < 128; i++) {
				 TERMINAL_PRINTF("%d,", LineScanImage0[i]);
				 TERMINAL_PRINTF("TEST");
				 }

				 for (i = 0; i < 128; i++) {
				 TERMINAL_PRINTF("%d", LineScanImage1[i]);
				 if (i == 127)
				 TERMINAL_PRINTF("\r\n");
				 else
				 TERMINAL_PRINTF(",");
				 }*/
				

			}

			break;
		}
	}

	return 0;
}


uint16_t * centralDifference(uint16_t * input){
	static uint16_t output[128];
	
	int i;
	int j=0;//back index
	int k=0;//forward index
	
	for(i=0;i<128;i++){
		j=i-1;
		k=i+1;
		
		if(j<0){
			j=0; 
		}
		
		if(k>127){
			k=127;
		}
		
		output[i]=(input[k]-input[j])/2;
	}
	
	return output;
}

int blackLineLocation(uint16_t * differentialResult){
	
	int i;
	
	uint16_t min=differentialResult[0];
	uint16_t max=differentialResult[0];
	
	int minPos=0;
	int maxPos=0;
	
	for(i=0;i<128;i++){
		if(differentialResult[i]>=max){
			max=differentialResult[i];
			maxPos=i;
		}
		else if(differentialResult[i]<min){
			min=differentialResult[i];
			minPos=i;
		}
		else{
			continue;
		}
	}
	
	int result = (minPos+maxPos)/2;
	
	return result;
}
















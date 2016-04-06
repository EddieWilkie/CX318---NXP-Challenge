#include "derivative.h" /* include peripheral declarations */
#include "TFC\TFC.h"
#include <stdio.h>
#include <stdlib.h>

int16_t * getCentralDifference(volatile uint16_t * lineScanCameraData){
	
static int16_t centralDifference[128];
	
	int i;
	
	for(i = 0; i < 128; i++){
		
		int j = i - 1;
		int k = i + 1;
		
		if(j < 0){
			j = 0;
		}
		if(k > 127)
			k = 127;
		
		centralDifference[i] = (lineScanCameraData[k] - lineScanCameraData[j])/2;

	}
	
	return centralDifference;
}

int getCentralPoint(int16 * centralDifferenceData){
	uint16_t centralPoint;
	
	//loop through and find the max and min values then take the average to find the central point.
	int i;
	int maxi = 0;
	int mini = 0;
	int16_t max = -5000;
	int16_t min = 5000;
	
	for(i = 0; i < 128; i++){
		if(centralDifferenceData[i] > max){
			max = centralDifferenceData[i];
			maxi = i;
		}
		if(centralDifferenceData[i] < min){
			min = centralDifferenceData[i];
			mini = i;
		}
	}
	
	centralPoint = (maxi + mini)/2;//min is a negative number so need to take the absolute value to achieve the average. 
	return centralPoint;
}

float getServoPosition(int centralPoint){
	float center = 64.0;
	float servoAngle = (centralPoint - center)/center;
	return servoAngle;
}

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
		case 0:
			//Demo mode 0 just tests the switches and LED's
			
			 if(TFC_PUSH_BUTTON_0_PRESSED)
			 TFC_BAT_LED0_ON;
			 else
			 TFC_BAT_LED0_OFF;
			 
			 if(TFC_PUSH_BUTTON_1_PRESSED)
			 TFC_BAT_LED3_ON;
			 else
			 TFC_BAT_LED3_OFF;
			 
			 
			 if(TFC_GetDIP_Switch()&0x01)
			 TFC_BAT_LED1_ON;
			 else
			 TFC_BAT_LED1_OFF;
			 
			 if(TFC_GetDIP_Switch()&0x08)
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
				TFC_SetServo(1, 1);
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
			TFC_SetMotorPWM(TFC_ReadPot(1),TFC_ReadPot(0));
			
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
			TFC_HBRIDGE_ENABLE;			
			//TFC_SetMotorPWM(TFC_ReadPot(0),TFC_ReadPot(0));
			
			if (TFC_Ticker[0] > 100 && LineScanImageReady == 1) {
				TFC_Ticker[0] = 0;
				LineScanImageReady = 0;
				TERMINAL_PRINTF("\r\n");
				TERMINAL_PRINTF("L:");

				if (t == 0)
					t = 3;
				else
					t--;

				TFC_SetBatteryLED_Level(t);

//				for (i = 0; i < 128; i++) {
//					TERMINAL_PRINTF("%d ", LineScanImage0[i]);
//				}
				
				TERMINAL_PRINTF("\n");
				
				int16_t * centralDifference = getCentralDifference(LineScanImage0);
				int centralPoint = getCentralPoint(centralDifference);
				TERMINAL_PRINTF("CentralPoint: %d\n", centralPoint);
				float servoPosition = getServoPosition(centralPoint);
				TERMINAL_PRINTF("Position: %d", (int) (servoPosition*100));
			
				TFC_SetServo(0, servoPosition);			
				TFC_SetMotorPWM(0.1,0.1);
						
			}

			break;
		}
	}

	return 0;
}

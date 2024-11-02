/*
 Name:		AVR_Bug_1.ino
 Created:	12/12/2018 
 Author:	XNRQ8224
*/

#include <Arduino_FreeRTOS.h>
#include <event_groups.h>

#define PIN_LED_R 9
#define PIN_LED_G 10
#define PIN_LED_B 6
#define PIN_LED_EYE1 3
#define PIN_LED_EYE2 5
#define PIN_SOUND A2

#define BIT_TOUCHED1	( 1 << 0 )
#define BIT_NOSE_FLASHING	( 2 << 0 )
#define BIT_EYES_FLASHING	( 3 << 0 )
#define BIT_SOUND	( 4 << 0 )

#define IDLE_BRIGHTNESS 0.25

EventGroupHandle_t xEventGroup;

void TaskBlinkEyes(void *pvParameters);
void TaskLightNose(void *pvParameters);
void TaskCheckButtons(void *pvParameters);
void TaskSound(void *pvParameters);



void setup() {
	pinMode(PIN_LED_R, OUTPUT);
	pinMode(PIN_LED_G, OUTPUT);
	pinMode(PIN_LED_B, OUTPUT);
	pinMode(PIN_LED_EYE1, OUTPUT);
	pinMode(PIN_LED_EYE2, OUTPUT);
	pinMode(PIN_SOUND, OUTPUT);
	digitalWrite(PIN_SOUND, HIGH);

	randomSeed(analogRead(A1));

	xEventGroup = xEventGroupCreate();
	/* Was the event group created successfully? */
	if (xEventGroup == NULL)
	{
		digitalWrite(PIN_LED_R, HIGH);
		while (true); //infinite loop here with RED led on to show a default
	}
	
	xTaskCreate(
		TaskBlinkEyes
		, (const portCHAR *)"BlinkEyes"   // A name just for humans
		, 128  // This stack size can be checked & adjusted by reading the Stack Highwater
		, NULL
		, 2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		, NULL);

	xTaskCreate(
		TaskLightNose
		, (const portCHAR *)"Lightnose"   // A name just for humans
		, 128  // This stack size can be checked & adjusted by reading the Stack Highwater
		, NULL
		, 2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		, NULL);

	xTaskCreate(
		TaskCheckButtons
		, (const portCHAR *)"CheckButtons"   // A name just for humans
		, 64  // This stack size can be checked & adjusted by reading the Stack Highwater
		, NULL
		, 2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		, NULL);

	xTaskCreate(
		TaskSound
		, (const portCHAR *)"Sound"   // A name just for humans
		, 64  // This stack size can be checked & adjusted by reading the Stack Highwater
		, NULL
		, 3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		, NULL);

}

void loop() {
}

void setEventGroupBits()
{
    //set state bits to trigger actions on other tasks
    xEventGroupSetBits(xEventGroup, BIT_TOUCHED1 | BIT_NOSE_FLASHING | BIT_EYES_FLASHING | BIT_SOUND);			
}


void TaskCheckButtons(void *pvParameters)  // This is a task.
{
	(void)pvParameters;
	int oldTouch = -1;
	uint8_t forceAction = false;

	while (true) {
		int touch = analogRead(A0);
		if (random(0, 500) == 1) {
			forceAction == true;
		}
		//if (touch < 122) {
		if (oldTouch != -1 && (oldTouch - touch) > 40
			|| forceAction) {			
			setEventGroupBits();
		}
		else {
			xEventGroupClearBits(xEventGroup, BIT_TOUCHED1);
		}
		oldTouch = touch;
		vTaskDelay(25 / portTICK_PERIOD_MS);
	}
}

void TaskBlinkEyes(void *pvParameters)  // This is a task.
{
	(void)pvParameters;

	uint8_t maxEyesBrightness = 120;
	
	for (;;) // A Task shall never return or exit.
	{
				
		for (int b = 1; b < maxEyesBrightness; b++) {			
			uint8_t brightness = b;
			if (xEventGroupGetBits(xEventGroup) && BIT_EYES_FLASHING) {
				b = random(0, 255);
			}
			analogWrite(PIN_LED_EYE1, b);
			analogWrite(PIN_LED_EYE2, b);
			vTaskDelay(30 / portTICK_PERIOD_MS); 
		}
		for (float b = maxEyesBrightness; b >= 1; b-=0.5) {
			uint8_t brightness = (int)b;
			if (xEventGroupGetBits(xEventGroup) && BIT_EYES_FLASHING) {
				b = random(0, 255);
			}
			analogWrite(PIN_LED_EYE1, b);
			analogWrite(PIN_LED_EYE2, b);
			vTaskDelay(40 / portTICK_PERIOD_MS);
		}		
		
		vTaskDelay(2300 / portTICK_PERIOD_MS);
		xEventGroupClearBits(xEventGroup, BIT_EYES_FLASHING);
	}
}

void TaskSound(void *pvParameters)  // This is a task.
{
	(void)pvParameters;
	const TickType_t xTicksToWait = 65535;
	EventBits_t uxBits;

	for (;;) // A Task shall never return or exit.
	{

		uxBits = xEventGroupWaitBits(
			xEventGroup,   /* The event group being tested. */
			BIT_SOUND, /* The bits within the event group to wait for. */
			pdTRUE,        /* BIT should be cleared before returning. */
			pdFALSE,       /* Don't wait for all bits, either bit will do. */
			xTicksToWait);/* Wait a maximum of 100ms for either bit to be set. */
		if (uxBits & BIT_SOUND)
		{
			uint8_t max = (uint8_t)random(8, 15);
			for (uint8_t i = 0; i < max; i++)
			{
				digitalWrite(PIN_SOUND, LOW);
				vTaskDelay((uint8_t)random(10 / portTICK_PERIOD_MS));
				digitalWrite(PIN_SOUND, HIGH);
				vTaskDelay((uint8_t)random(100 / portTICK_PERIOD_MS));
			}
		}
	}
}


void Wheel(byte WheelPos, byte* r, byte* g, byte* b)
{
	WheelPos = 255 - WheelPos;
	if (WheelPos < 85)
	{
		*r = 255 - WheelPos * 3;
		*g = 0;
		*b = WheelPos * 3;
	}
	else if (WheelPos < 170)
	{
		WheelPos -= 85;
		*r = 0;
		*g = WheelPos * 3;
		*b = 255 - WheelPos * 3;
	}
	else
	{
		WheelPos -= 170;
		*r = WheelPos * 3;
		*g = 255 - WheelPos * 3;
		*b = 0;
	}
}

void TaskLightNose(void *pvParameters)  // This is a task.
{
	(void)pvParameters;

	byte r, g, b;
	byte pos = 0;
	float brightness = IDLE_BRIGHTNESS;
	unsigned long t1 = millis();
	unsigned long t2 = t1;
	uint16_t nextChange = t1 + random(2000, 8000);
	uint16_t delay = 100;
	while (true) {	
		if (xEventGroupGetBits(xEventGroup) && BIT_NOSE_FLASHING) { //if touched then flash nose
			xEventGroupClearBits(xEventGroup, BIT_NOSE_FLASHING); //clear flash nose bit
			for (int i = 0; i < 20; i++) { //flashing nose
				digitalWrite(PIN_LED_R, HIGH);
				digitalWrite(PIN_LED_G, HIGH);
				digitalWrite(PIN_LED_B, HIGH);
				vTaskDelay(random(30,35) / portTICK_PERIOD_MS);
				digitalWrite(PIN_LED_R, LOW);
				digitalWrite(PIN_LED_G, LOW);
				digitalWrite(PIN_LED_B, LOW);
				vTaskDelay(random(30,40) / portTICK_PERIOD_MS);
			}
		}
		// slow color wheel movement with randomness in speed and brighness changes
		Wheel(pos, &r, &g, &b);
		analogWrite(PIN_LED_R, (int)(r * brightness));
		analogWrite(PIN_LED_G, (int)(g * brightness));
		analogWrite(PIN_LED_B, (int)(b * brightness));
		
		pos += 1;

		t2 = millis();
		if (t2 - t1 > nextChange) {
			brightness = (float)IDLE_BRIGHTNESS * random(1, 4);
			nextChange = t2 + random(2000, 8000);
			delay = random(25, 400);
			t1 = t2;
		}
		

		vTaskDelay(delay / portTICK_PERIOD_MS);
	}	
}





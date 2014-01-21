#include <Keypad.h>
#include <Metro.h>
#include <SoftwareSerial.h>
#include "chaine.h"

const char pwd_admin_content[] = {'3', '7', '2', '4'};
const char pwd_user_content[]  = {'1', '1', '1', '1'};
const char pwd_menu_content[]  = {'0', '0', '0', '0'};

//relay
const int pinout_relay = 12;
//led
const int pinout_led = 11;
//sound
const int pinout_buzzer = 10;
const int buz_ok_note = 1760;
const int buz_ok_duration = 300;
const int buz_wrong_note = 440;
const int buz_wrong_duration = 800;
const int buz_reset_note = 440;
const int buz_reset_duration = 300;


//keypad
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
		{'1','2','3'},
		{'4','5','6'},
		{'7','8','9'},
		{'*','0','#'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3};
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//prog state
typedef enum{ MODIFPWD, NORMAL } progState;
progState stateProgram;
typedef enum{ DEBUT, WAIT_OLD_PWD, WAIT_NEW_PWD_1, WAIT_NEW_PWD_2, NA } modifPwdState;
modifPwdState statePwd;

// tempo inactivite
const int DELAY_INACTIF = 7 * 1000;
Metro delayInactif = Metro(DELAY_INACTIF); 

char idle_state = true;
char idle_state_old = true;

bool relayState = false;

chaine4 pwd_admin; 	// never erased, constant
chaine4 pwd_menu; 	// to change user pwd
chaine4 pwd_user;		// user pwd, can be changed
chaine4 pwd_buffer;
chaine4 pwd_buffer2;

void setup(){
	Serial.begin(9600);
	chaine4Init(&pwd_admin);
	chaine4Init(&pwd_user);
	chaine4Init(&pwd_menu);
	chaine4Init(&pwd_buffer);
	chaine4Init(&pwd_buffer2);
	stateProgram = NORMAL;
	if (!chaineSet(&pwd_admin, (char*) pwd_admin_content, 4)) {Serial.println("echec init"); }
	if (!chaineSet(&pwd_user, (char*) pwd_user_content, 4)) {Serial.println("echec init"); }
	if (!chaineSet(&pwd_menu, (char*) pwd_menu_content, 4)) {Serial.println("echec init"); }
	//init output pin
	pinMode(pinout_buzzer, OUTPUT); 
	pinMode(pinout_led, OUTPUT); 
	pinMode(pinout_relay, OUTPUT); 
	relayOFF(); //relay and led OFF
	digitalWrite(pinout_buzzer, LOW);
	idle_state = true;
}

void loop(){
	if (delayInactif.check()) {
		//sound reset si inactivite
		if (idle_state == false){
			if((stateProgram == MODIFPWD) || (pwd_buffer.index > 0) || (pwd_buffer2.index > 0)) {
				sound_reset(); 
				Serial.println("raz");
			}
		}
		gotoNormalProcess();
		idle_state = true;
	}
	char key = keypad.getKey();
	if (key != NO_KEY){
		idle_state = false;
		delayInactif.reset();
		delay(100); 
		switch (key){
			case 'A': 
			case 'B': 
			case 'C':
			case 'D':
				break; 
			case '#': 
				// reset pwd
				if (stateProgram == NORMAL) {
					chaineReset(&pwd_buffer);
					sound_reset();
				}
				else { modifyPwd(key); }
				break;
			case '*': 
				// check pwd
				if (stateProgram == NORMAL) {
					checkPwd();
					chaineReset(&pwd_buffer);
				}
				else { modifyPwd(key); }
				break;
			default: 
				//append to buffer
				if (stateProgram == NORMAL) {
					chaineAppend(key, &pwd_buffer);
					chainePrint(pwd_buffer);
				}
				else { modifyPwd(key); }
			// end of switch
		}
	}
}

void checkPwd(){
	if (chaineCompare(pwd_buffer, pwd_user) || chaineCompare(pwd_buffer, pwd_admin)) { 
		sound_ok();
		toggleRelay();
		gotoNormalProcess();
	}
	else if (chaineCompare(pwd_buffer, pwd_menu)) { 
		sound_ok();
		stateProgram = MODIFPWD;
		statePwd = DEBUT;
		clearBuffers();
		modifyPwd('0');
	}
	else { 
		sound_wrong();
		Serial.println("pwd non reconnu");
		gotoNormalProcess();
	}
}

void modifyPwd(char key) {
	switch (statePwd) {
		case DEBUT: 
			Serial.println("modif pwd");
			delay(1000);
			Serial.println("code actuel?");		
			statePwd = WAIT_OLD_PWD;
		break; 
		case WAIT_OLD_PWD: 
			if (key == '*') {
				if (chaineCompare(pwd_buffer, pwd_user)) {
					chaineReset(&pwd_buffer);
					Serial.println("new code ?");
					statePwd = WAIT_NEW_PWD_1;
					sound_ok();
				}
				else {
					Serial.println("echec");
					gotoNormalProcess();
					sound_wrong();
				}
			}
			else if (key == '#') { 
				gotoNormalProcess(); 
				sound_reset();
			}
			else { 
				chaineAppend(key, &pwd_buffer); 
				chainePrint(pwd_buffer);
			}		
			break; 
		case WAIT_NEW_PWD_1:
			switch (key) {
				case '#': 
					gotoNormalProcess();
					sound_reset();
					break;
				case '*': 
					if (pwd_buffer.index == 4) {
						Serial.println("repeat ?");
						statePwd = WAIT_NEW_PWD_2;
						sound_ok();
					}
					else {
						Serial.println("trop court");
						gotoNormalProcess();
						sound_wrong();
					}
					break; 
				default: 
					chaineAppend(key, &pwd_buffer); 
					chainePrint(pwd_buffer);
			}
			break; 
		case WAIT_NEW_PWD_2: 
			switch (key) {
				case '#': 
					gotoNormalProcess();
					sound_reset();
					break;
				case '*': 
					if (chaineCompare(pwd_buffer, pwd_buffer2)) {
						Serial.println("OK");
						chaineSet(&pwd_user, pwd_buffer.content, pwd_buffer.max_len);
						gotoNormalProcess();
						sound_ok();
					}
					else {
						Serial.println("echec");
						gotoNormalProcess();
						sound_wrong();
					}
					break; 
				default: 
					chaineAppend(key, &pwd_buffer2);
					chainePrint(pwd_buffer2);
			}
			break; 
	}
}

void gotoNormalProcess() {
	stateProgram = NORMAL;
	clearBuffers();
	delay(100);
}

void clearBuffers() {
	chaineReset(&pwd_buffer);
	chaineReset(&pwd_buffer2);
}

void toggleRelay() {
	if (relayState == false) {
		relayState = true;
		relayON();
	}
	else { 
		relayState = false; 
		relayOFF();
	}
}

void relayON() {
	digitalWrite(pinout_led, HIGH);
	digitalWrite(pinout_relay, HIGH);
}
void relayOFF() {
	digitalWrite(pinout_led, LOW);
	digitalWrite(pinout_relay, LOW);
}

void sound_reset() {
	tone(pinout_buzzer, buz_reset_note, buz_reset_duration);
}
void sound_wrong() {
	tone(pinout_buzzer, buz_wrong_note, buz_wrong_duration);
}
void sound_ok() {
	tone(pinout_buzzer, buz_ok_note, buz_ok_duration);
}

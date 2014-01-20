#include "Arduino.h"
void setup();
void loop();
#include <Keypad.h>
#include <Password.h>
#include <Metro.h>
#include <SoftwareSerial.h>
#include "chaine.h"

const char pwd_admin_content[] = {'3', '7', '2', '4'};
const char pwd_user_content[]  = {'1', '1', '1', '1'};
const char pwd_menu_content[]  = {'0', '0', '0', '0'};

//keypad
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
		{'1','2','3','A'},
		{'4','5','6','B'},
		{'7','8','9','C'},
		{'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//prog state
typedef enum{ MODIFPWD, NORMAL } progState;
progState stateProgram;
typedef enum{ DEBUT, WAIT_OLD_PWD, WAIT_NEW_PWD_1, WAIT_NEW_PWD_2, NA } modifPwdState;
modifPwdState statePwd;

// tempo inactivite
const int DELAY_INACTIF = 7 * 1000;
Metro delayInactif = Metro(DELAY_INACTIF); 

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

	delay(2000);
	Serial.print("chaineCompare(pwd_menu, pwd_admin) : ");
	Serial.println(strcmp(pwd_menu.content, pwd_admin.content));
	Serial.print("strcmp(pwd_menu, pwd_menu) : ");
	Serial.println(strcmp(pwd_menu.content, pwd_menu.content));
}

void loop(){
	if (delayInactif.check()) {
		stateProgram = NORMAL;
		clearBuffers();
		Serial.println("raz");
	}
	char key = keypad.getKey();
	if (key != NO_KEY){
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
	if (chaineCompare(pwd_buffer, pwd_user) == true) { 
		toggleRelay(); 
		gotoNormalProcess();
	}
	else if (chaineCompare(pwd_buffer, pwd_menu) == true) { 
		stateProgram = MODIFPWD;
		statePwd = DEBUT;
		clearBuffers();
	}
	else { 
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
				}
				else {
					Serial.println("echec");
					gotoNormalProcess();
				}
			}
			else if (key == '#') { chaineReset(&pwd_buffer); }
			else { chaineAppend(key, &pwd_buffer); }		
			break; 
		case WAIT_NEW_PWD_1:
			switch (key) {
				case '#': 
					gotoNormalProcess();
					break;
				case '*': 
					if (pwd_buffer.index == 4) {
						Serial.println("repeat ?");
						statePwd = WAIT_NEW_PWD_2;
					}
					else {
						Serial.println("trop court");
						gotoNormalProcess();
					}
					break; 
				default: 
					chaineAppend(key, &pwd_buffer);
			}
			break; 
		case WAIT_NEW_PWD_2: 
			switch (key) {
				case '#': 
					gotoNormalProcess();
					break;
				case '*': 
					if (chaineCompare(pwd_buffer, pwd_buffer)) {
						Serial.println("OK");
					}
					else {
						Serial.println("echec");
						gotoNormalProcess();
					}
					break; 
				default: 
					chaineAppend(key, &pwd_buffer2);
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
	Serial.println("toggleRelay()");
}

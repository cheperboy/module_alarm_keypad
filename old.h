#include <Keypad.h>
#include <Password.h>
#include <Metro.h>
#include <SoftwareSerial.h>

#define STRING_TERMINATOR '\0'

String newPasswordString; //hold the new password
char newPassword[6]; //charater string of newPasswordString

byte max_code_len = 4;

char[4] code_temp_1;
int code_temp_1_len = 0;
 
//initialize password to 1234
//you can use password.set(newPassword) to overwrite it
Password password = Password( "1234" );

 
byte maxPasswordLength = 6; 
byte currentPasswordLength = 0;
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns
 
//Define the keymap
char keys[ROWS][COLS] = {
{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
 
// Create the Keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// tempo inactivite
const int DELAY_INACTIF = 10 * 1000;
Metro delayInactif = Metro(DELAY_INACTIF); 

bool alarmState = false;

typedef enum{ MODIFPWD, NORMAL } progState;
progState stateProgram;

typedef enum{ DEBUT, WAIT_OLD_PWD, WAIT_NEW_PWD_1, WAIT_NEW_PWD_2, NA } modifPwdState;
modifPwdState statePwd;

void setup(){
	stateProgram = NORMAL;
	delayInactif.interval(DELAY_INACTIF);
	Serial.begin(9600);
}
 
void loop(){
	if (delayInactif.check()) {
		stateProgram = NORMAL;
		clearPassword();
		Serial.println("raz");
	}
	char key = keypad.getKey();
	if (key != NO_KEY){
		delayInactif.reset();
		delay(60); 
		switch (key){
		case 'A': break; 
		case 'B': break; 
		case 'C': break; 
		case 'D': //modifier mdp
			stateProgram = MODIFPWD;
			statePwd = WAIT_OLD_PWD;
			clearPassword();
			Serial.println("ancien code?");
		break; 
		case '#': 
			if (stateProgram == MODIFPWD) { modifyPwd(key); } //procedure chgmt code
			else { checkPassword(); } //fonctionnement nominal : OK
		break;
		case '*': 
			if (stateProgram == MODIFPWD) { modifyPwd(key); } //procedure chgmt code
			else { clearPassword(); } //fonctionnement nominal ANNULER
		break;
		default: 
			if (stateProgram == MODIFPWD) { modifyPwd(key); } //procedure chgmt code
			else { processNumberKey(key); } //fonctionnement nominal : DIGIT
		}
	}
}

void modifyPwd(char key) {
	switch (statePwd){
		case NA: 
			Serial.println("bug");
			while(1);
		break; 
		case DEBUT: 
			// Serial.println("DEBUT");
			statePwd = WAIT_OLD_PWD;
		break; 
		case WAIT_OLD_PWD: 
			if (key == '#') {
				if (password.evaluate()) {
					Serial.println("new code?");
					initNewCode(); 
					statePwd = WAIT_NEW_PWD_1;
				}
				else {
					Serial.println("echec");
					gotoNormalProcess();
				}
			}
			else { processNumberKey(key); }
		break; 
		case WAIT_NEW_PWD_1:
			Serial.println("WAIT_NEW_PWD_1");
			switch (key){
			case '#': 
				if (code_temp_1_len > 3) {
					code_temp_1[code_temp_1_len] = '\0';
					statePwd = WAIT_NEW_PWD_2;
					Serial.println("repeat?");
				}
				else { 
					Serial.println("trop petit");
					gotoNormalProcess();
				}
			break; 
			case '*': 
				Serial.println("retour");
				gotoNormalProcess();
			break; 
			default: 
				code_temp_1[code_temp_1_len] = key;
				code_temp_1[code_temp_1_len+1] = '\0';
				code_temp_1_len++;
				printString(code_temp_1);
			}
		break; 
		case WAIT_NEW_PWD_2: 
			Serial.println("WAIT_NEW_PWD_2");
			switch (key){
				case '#': break; 
					code_temp_2[code_temp_2_length] = '\0';
					if (compare(code_temp_1, code_temp_2)) {
						changePassword(code_temp_1);
						Serial.println("ok");
						gotoNormalProcess();
					}
					else { 
						Serial.println("different");
						gotoNormalProcess();
					}
				case '*': 
					Serial.println("retour");
					gotoNormalProcess();
				break; 
				default: 
					code_temp_2[code_temp_2_length] = key;
					code_temp_2_length++;
			}
		break;
	}
}
void gotoNormalProcess() {
	stateProgram = NORMAL;
	clearPassword();
	delay(1500);
	//changer l'affichage
}

void processNumberKey(char key) {
   // Serial.print(key);
   currentPasswordLength++;
   password.append(key);
   Serial.println(password.getPassword());
}

void checkPassword() {
   if (password.evaluate()){
			toggleAlarmState();
      Serial.println(" OK.");
   } else {
      Serial.println(" Wrong passwowrd!");
   } 
   clearPassword();
}

void clearPassword() {
   password.reset(); 
   currentPasswordLength = 0; 
}

void initNewCode() {
	code_temp_1_len = 0;
	code_temp_2_length = 0;
}

void changePassword(char* newPassword) {
	password.set(newPassword);
	clearPassword();
}
void changePassword_() {
	newPasswordString = "123";
	newPasswordString.toCharArray(newPassword, newPasswordString.length()+1); //convert string to char array
	password.set(newPassword);
	clearPassword();
	Serial.print("Password changed to ");
	Serial.println(newPasswordString);
}

// char* code_temp_1 = "";
// char* code_temp_2 = "";
// int code_temp_1_len = 0;
// int code_temp_2_length = 0;

boolean compare(char* code_1, char* code_2) {
	for (char i=0; i<maxPasswordLength; i++){
		if ((code_1 == STRING_TERMINATOR) && (code_2 == STRING_TERMINATOR)) {
			return true;
		}
		else if (code_1[i] != code_2[i]) {
			return false;
		}
	}
	return false;
}

boolean printString(char* code) {
	char i=0;
	while (code[i] != '\0'){
		Serial.print(code[i], DEC);
			i++;
		}
		Serial.println("");
}

void toggleAlarmState() {
	if (alarmState == true) {alarmState = false;}
	else { alarmState = true; }
	Serial.print("alarmState ");
	Serial.println(alarmState);
}
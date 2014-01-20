#define CHAINE_END '\0'

typedef struct structchaine
{
	char content[5];
	byte max_len;
	byte index;
} chaine4;

void chaine4Init(struct structchaine *machaine) {
	machaine->content[0] = CHAINE_END;
	machaine->index = 0;
	machaine->max_len = 4;
}
boolean chaineSet(struct structchaine *machaine, char value[], byte len) {
	if (len > machaine->max_len) { return (false); }
	char i = 0;
	while ((i < len) && (value[i] != '\0')) {
		machaine->content[i] = value[i];
		i++;
	}
	machaine->content[i] = CHAINE_END;
	machaine->index = i;
	return (true);
}
void chaineReset(struct structchaine *machaine) {
	machaine->content[0] = CHAINE_END;
	machaine->index = 0;
}
void chaineAppend(char key, struct structchaine *machaine) {
	if ((machaine->index) == machaine->max_len) { 
		Serial.println("max");
	}
	else {
		machaine->content[machaine->index++] = key;
		machaine->content[machaine->index] = CHAINE_END;
	}
}
void chaineDebug(struct structchaine machaine) {
	char i = 0;
	Serial.print("index : ");
	Serial.println(machaine.index);
	Serial.print("max_len : ");
	Serial.println(machaine.max_len);
	Serial.print("content <");
	while ((machaine.content[i] != CHAINE_END) && (i < machaine.max_len)){
		Serial.print(machaine.content[i]);
		i++;
	}
	Serial.println(">");
}
void chainePrint(struct structchaine machaine) {
	char i = 0;
	Serial.print("<");
	while ((machaine.content[i] != CHAINE_END) && (i < machaine.max_len)){
		Serial.print(machaine.content[i]);
		i++;
	}
	Serial.println(">");
}
void chaineErase(struct structchaine *machaine) {
	char i = 0;
	while (i < machaine->max_len){
		machaine->content[i] = '-';
		i++;
	}
	machaine->content[machaine->max_len] = CHAINE_END;
	machaine->index = machaine->max_len;
}
boolean chaineCompare(struct structchaine chaine1, struct structchaine chaine2) {
	if (strcmp(chaine1.content, chaine2.content) == 0) { return (true); }
	else { return (false); }
}

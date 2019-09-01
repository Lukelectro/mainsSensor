#include "show.h"
#include <util/delay.h>
#include <string.h>

void readdevnames(void){ // read devnames from EEPROM (on bootup)
    uint8_t size = sizeof(devnames[0]); // they are all the same size (18 bytes, but still use sizeoff in case it changes)    
    for(unsigned int i = 0;i<numdevs;i++){
        eeprom_read_block(&devnames[i], (void*) (i*size) ,size); // TODO: test this!
    }
}

void storedevnames(void){ // store devnames in EEPROM (on modification) (NOTE: EEPROM on ATMEGA328p is 1024 bytes) 
// use EEPROM update, so when data is the same, nothing gets rewritten (saves wear);
    uint8_t size = sizeof(devnames[0]); // they are all the same size (18 bytes, but still use sizeoff in case it changes)    
    for(unsigned int i = 0;i<numdevs;i++){
        eeprom_update_block(&devnames[i], (void*) (i*size) ,size); // TODO: test this!
    }
}

char* IDtoName(uint16_t ID){
    for(int i=0;i<numdevs;i++){
        if(devnames[i].ID==ID) return &devnames[i].name[0];
    }
    return NULL;
}


// TODO: serial input of new device names for each ID? setname 0x8234 slartibartfast 
// TODO: add a way to erase ID/name combo's with unused or no longer used ID's (or wrongly entered)
// maybe just set unused names to " ", and make it re-use those spots for new ID/name combinations?
void readnewname(DNS* names){
    unsigned int data_in;
    static char input[32]; // setname 0x1234 a16charalongname+0    
    static uint8_t i = 0;
    unsigned long IDL = 0; // long ID, for strtoul 
    uint16_t newID;
    char newname[16], buff=0, buffer[7];   
    char* startofname;
    bool exists = false;

    /* per time this function gets called, one character gets read form buffer, so call periodically */

    data_in = uart_getc(); // read a character
    if( 0==(data_in & 0xFF00)){ // higher byte is status, 0 is good
        buff = data_in & 0x00FF; // lower byte is data
        input[i] = buff;        
        uart_putc(buff); // echo what's input        
        if(i<31) i++;
    }     
    
    if((i==31) || (13==buff) ){ // On 32 characters input, or CR (use buff because i++) (If you want to add LF: || (10==buff))
        if(i==31) input[i]=0; else input[i-1]=0; // correctly terminate string for further processing        
        i=0;
        buff = 0; // so it does not keep looping this
        if(0==strncmp(input,"setname",7)){ // if correctly formed command (use strncmp or strstr?)
            IDL = strtoul((void*)(input+7),NULL,0); // begin 7 leters into input string, so skip the "setname" and start at the ID ( hex as 0x... or decimal)        
        }
        else
        {
         uart_puts_P("\nIncorrect command. Try setname [ID] [name (max 16)]\n");
        }
        if(IDL!=0){ // if indeed an ID is input
            newID = IDL; // copy ID, for later search
            
            // display ID and name just set.
            
            itoa(IDL,buffer,16); //convert ID to hex-formeted text for later display
            startofname = strrchr(input,' ') + 1; // name starts after last ' '.
            if( (startofname-input+1) >= 16 ){// prevent name longer than sizeof(newname), 
            // because input is 32 chars so if it starts at or after position 16 it is short enough
            // though that ruins the posibility of decimal input for ID's... Those are given in .hex anyway so not a problem            
                strcpy(newname,startofname); 
            }
            else
            {
                uart_puts_P("\nError: new name is too long or ID too short \n"); // if new name is longer then sizeof(newname)
                strcpy(newname,"too long");                          // set it to something shorter. 
            }
            uart_puts_P("\nName set: ");            
            uart_puts(buffer);            
            uart_puts_P(", ");
            uart_puts(newname);
            uart_puts_P("\n");                      

            // the store-in-eeprom part.
            // for i in names find the unused/255 one and put this ID/name combo in, then update EEPROM 
        
            exists=false;
            for(int i=0;i<numdevs;i++){
                if(devnames[i].ID == newID){ // If this ID allready has a name, change its name
                    strcpy(devnames[i].name,newname);
                    exists = true;
                    break;
                }
            }
            if(exists == false){ //if ID is not found
                for(int i=0;i<numdevs;i++){
                    if((devnames[i].name[0]==255)||(devnames[i].name[0]==' ')||(devnames[i].name[0]=='\n')){ 
                    // names that start with 255 (empty EEPROM), " " or newline are empty spots
                    devnames[i].ID=newID;
                    strcpy(devnames[i].name,newname); 
                    break; // don't fill the entire EEPROM with 1 ID/name combo!!
                    }
                }
            }          
            storedevnames(); //update_EEPROM (only writes data if it is indeed changed)
        }
        else
        {
         uart_puts_P("\nIncorrect data. Try setname [ID] [name (max 16)]\n");
        }
    }
}


// Display number of devices still on (either on a LED if numon>0, or on 7segment displays, or on a LCD)
// Display device ID's still on.
// TODO: Maybe even display device names for those ID's, but that would require a lookup table. That would need te be editable or known at compile time. Both difficult.
void LCD_refresh(void){
char buffer[7];
uint8_t num;
//static unsigned int whichnameareweat = 0; // Maybe later scroll the names

_delay_ms(10); // so it can finish serial transmission
uart_release();// lcd/uart share pins
lcd_cls();     // clear lcd so devices that are now OFF are not still displayed

// turn backlite on if there is still devices ON, and off otherwise.
// also, display names of devices still ON if there are devices still ON, otherwise display a message saying all is OFF.
    if(numOn>0){ 
        
        PORTC |= (1<<PORTC3); 
        itoa(numOn,buffer,10);
        lcd_puts(buffer);
        if(numOn==1) lcd_puts(" ding nog aan"); else lcd_puts(" dingen nog aan");
        lcd_goto(1,0);
// Display NAMES of devices still on:
        num = 1;
        for(unsigned int i =0;i<numdevs;i++){
            if(devices[i].state==ON){
                char* name = IDtoName(devices[i].ID);
                if(name!=NULL){ 
                    lcd_puts(name);
                    num++;
                }
                lcd_goto(num,0);
                if(num>3) break; // can display at maximum 3 names...
            }
        }
    }
    else{
        PORTC &= ~(1<<PORTC3);  // Backlite OFF
        lcd_puts("Alles met een sensor");
        lcd_goto(1,0);
        lcd_puts("is uit");
    }
    reenableuart();
}

void SerialRefresh(void){
// Transmit over serial: numon (number of devices still ON) and ID of all devices still on / OFF/Presumed off.

char buffer[7];
unsigned int count=0;

uart_puts_P("NumOn: ");
itoa(numOn,buffer,10);
uart_puts(buffer);
uart_putc('\n');
uart_puts_P("ID's still ON:");
    for(unsigned int pntr=0;pntr<numdevs;pntr++){
        if(devices[pntr].state==ON){
            itoa(devices[pntr].ID,buffer,16); //hex
            uart_puts(buffer);
            uart_puts_P(",");
            count++;      
            }
    }
    if(count) uart_putc('\n'); else uart_puts_P("NONE \n");
count = 0;
uart_puts_P("ID's not seen a while, presumed OFF:");
    for(unsigned int pntr=0;pntr<numdevs;pntr++){
        if(devices[pntr].state==PRESUMED_OFF){
            itoa(devices[pntr].ID,buffer,16); //hex
            uart_puts(buffer);
            uart_puts_P(","); 
            count++;        
            }
    }
    if(count) uart_putc('\n'); else uart_puts_P("NONE \n");
count = 0;
uart_puts_P("ID's that sent goodbye's, known OFF:");
    for(unsigned int pntr=0;pntr<numdevs;pntr++){
        if(devices[pntr].state==OFF){
            itoa(devices[pntr].ID,buffer,16); //hex
            uart_puts(buffer);
            uart_puts_P(",");      
            count++;                
            }
        }
    if(count) uart_putc('\n'); else uart_puts_P("NONE \n");

#ifdef LOG_ALL /* only print if indeed logged */
count = 0;
uart_puts_P("ID's that sent garbled MSG's:"); 
    for(unsigned int pntr=0;pntr<numdevs;pntr++){
        if(devices[pntr].state==GARBLE){
            itoa(devices[pntr].ID,buffer,16); //hex
            uart_puts(buffer);
            uart_puts_P(",");      
            count++;                
            }
        }
    if(count) uart_putc('\n'); else uart_puts_P("NONE \n");
/* when logging all/showing all, also show ON_1st*/
count = 0;
uart_puts_P("ID's that so far only sent 1 ON msg (ON_1st):"); 
    for(unsigned int pntr=0;pntr<numdevs;pntr++){
        if(devices[pntr].state==ON_1st){
            itoa(devices[pntr].ID,buffer,16); //hex
            uart_puts(buffer);
            uart_puts_P(",");      
            count++;                
            }
        }
    if(count) uart_putc('\n'); else uart_puts_P("NONE \n");
#endif
}



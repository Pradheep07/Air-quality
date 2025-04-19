#include<xc.h>

// Configuration bits - you might need these depending on your compiler
// #pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
// #pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
// #pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
// #pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
// #pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit
// #pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit
// #pragma config WRT = OFF        // Flash Program Memory Write Enable bits
// #pragma config CP = OFF         // Flash Program Memory Code Protection bit

// Define pins for LEDs and buzzer
#define LED_SAFE    RB0    // Safe level LED (Green)
#define LED_WARNING RB1    // Warning level LED (Yellow)
#define LED_DANGER  RB2    // Danger level LED (Red)
#define BUZZER      RB3    // Buzzer output

// Define PPM thresholds
#define SAFE_THRESHOLD    40.0   // Below this is safe
#define WARNING_THRESHOLD 80.0   // Below this is warning, above is danger

int flag = 0;
unsigned int adc_value = 0;
unsigned int prev_adc = 0;
float ppm_value = 0;

void delay_ms(int ms);
void delay_us(int us);
void Data(int Value);
void Cmd(int Value);
void Send2Lcd(const char Adr, const char *Lcd);
void LCD_Init();
void ADC_Init();
unsigned int ADC_Read(unsigned char channel);
void display_ppm(float ppm);
void LCD_Clear(void);
void update_indicators(float ppm);

void __interrupt() adcint() {
    GIE = 0;
    if(ADIF) {
        adc_value = (ADRESH << 8) + ADRESL;
        ADIF = 0;
    }
    GIE = 1;
}

void main() {
    // Initialize ports
    TRISC = 0x00;    // PORTC as output for LCD control pins
    TRISD = 0x00;    // PORTD as output for LCD data pins
    TRISA5 = 1;      // RA5 as input for MQ135 sensor
    TRISB = 0x00;    // PORTB as output for LEDs and buzzer

    PORTB = 0x00;    // Clear PORTB
    PORTC = 0x00;    // Clear PORTC
    PORTD = 0x00;    // Clear PORTD

    // Initialize peripherals
    delay_ms(500);   // Power-on delay for LCD and sensor
    LCD_Init();      // Initialize LCD
    ADC_Init();      // Initialize ADC

    // Display initial message
    LCD_Clear();
    Send2Lcd(0x80, "AIR QUALITY");
    Send2Lcd(0xC0, "Initializing...");
    delay_ms(1000);

    // Filter initialization
    prev_adc = ADC_Read(4);  // Initial reading (AN4/RA5)
    delay_ms(100);

    while(1) {
        // Read sensor with simple filtering
        unsigned int new_adc = ADC_Read(4);
        adc_value = (prev_adc * 3 + new_adc) / 4;  // Weighted averaging (75% old, 25% new)
        prev_adc = adc_value;

        // Convert to PPM (calibrate according to your sensor)
        ppm_value = ((float)adc_value * 5.0 / 1023.0) * 100.0;

        // Display PPM value
        display_ppm(ppm_value);

        // Update LED indicators and buzzer
        update_indicators(ppm_value);

        delay_ms(500);  // Update every half second
    }
}

void LCD_Init() {
    // Power on delay
    delay_ms(100);

    // Initial commands in 8-bit mode
    Cmd(0x30);    // 8-bit mode
    delay_ms(5);
    Cmd(0x30);    // 8-bit mode
    delay_ms(5);
    Cmd(0x30);    // 8-bit mode
    delay_ms(5);

    // Configure LCD
    Cmd(0x38);    // 8-bit, 2 line, 5x7 dots
    delay_ms(5);
    Cmd(0x08);    // Display off
    delay_ms(5);
    Cmd(0x01);    // Clear display
    delay_ms(5);
    Cmd(0x06);    // Entry mode - cursor increment, no display shift
    delay_ms(5);
    Cmd(0x0C);    // Display on, cursor off, blink off
    delay_ms(5);
}

void LCD_Clear(void) {
    Cmd(0x01);    // Clear display command
    delay_ms(5);  // Clear display needs longer delay
}

void Cmd(int Value) {
    PORTD = Value;
    RC1 = 0;      // RS = 0 (Command)
    RC0 = 0;      // RW = 0 (Write)
    delay_us(10);
    RC2 = 1;      // EN = 1
    delay_us(10);
    RC2 = 0;      // EN = 0
    delay_ms(2);  // Command execution time
}

void Data(int Value) {
    PORTD = Value;
    RC1 = 1;      // RS = 1 (Data)
    RC0 = 0;      // RW = 0 (Write)
    delay_us(10);
    RC2 = 1;      // EN = 1
    delay_us(10);
    RC2 = 0;      // EN = 0
    delay_ms(1);  // Data settling time
}

void Send2Lcd(const char Adr, const char *Lcd) {
    Cmd(Adr);     // Set DDRAM address
    delay_us(100);

    while(*Lcd != '\0') {
        Data(*Lcd);
        Lcd++;
    }
}

void delay_ms(int ms) {
    int i, count;
    for(i=1; i<=ms; i++) {
        count = 1000;
        while(count != 1) {
            count--;
        }
    }
}

void delay_us(int us) {
    us = us >> 1;
    while(us != 1) us--;
}

void ADC_Init() {
    // Configure ADC
    ADCON0 = 0x81;  // ADC ON, Clock = Fosc/32, Channel 0
    ADCON1 = 0x80;  // Right justified, All pins as analog
    delay_ms(20);   // Allow ADC reference to stabilize
}

unsigned int ADC_Read(unsigned char channel) {
    // Configure channel
    ADCON0 &= 0xC5;           // Clear channel bits
    ADCON0 |= (channel << 3); // Set channel

    delay_us(20);             // Acquisition time

    GO_nDONE = 1;             // Start conversion
    while(GO_nDONE);          // Wait for completion

    return ((ADRESH << 8) + ADRESL);  // Return result
}

void display_ppm(float ppm) {
    int whole = (int)ppm;
    int frac = (int)((ppm - whole) * 10);

    char buffer[16];
    buffer[0] = 'P';
    buffer[1] = 'P';
    buffer[2] = 'M';
    buffer[3] = ':';
    buffer[4] = ' ';

    // Convert whole part to characters
    if(whole >= 1000) {
        buffer[5] = '0' + (whole / 1000);
        buffer[6] = '0' + ((whole / 100) % 10);
        buffer[7] = '0' + ((whole / 10) % 10);
        buffer[8] = '0' + (whole % 10);
        buffer[9] = '.';
        buffer[10] = '0' + frac;
        buffer[11] = ' ';
        buffer[12] = '\0';
    }
    else if(whole >= 100) {
        buffer[5] = '0' + (whole / 100);
        buffer[6] = '0' + ((whole / 10) % 10);
        buffer[7] = '0' + (whole % 10);
        buffer[8] = '.';
        buffer[9] = '0' + frac;
        buffer[10] = ' ';
        buffer[11] = ' ';
        buffer[12] = '\0';
    }
    else if(whole >= 10) {
        buffer[5] = '0' + (whole / 10);
        buffer[6] = '0' + (whole % 10);
        buffer[7] = '.';
        buffer[8] = '0' + frac;
        buffer[9] = ' ';
        buffer[10] = ' ';
        buffer[11] = ' ';
        buffer[12] = '\0';
    }
    else {
        buffer[5] = '0' + whole;
        buffer[6] = '.';
        buffer[7] = '0' + frac;
        buffer[8] = ' ';
        buffer[9] = ' ';
        buffer[10] = ' ';
        buffer[11] = ' ';
        buffer[12] = '\0';
    }

    // Display on second line
    Send2Lcd(0xC0, buffer);
}

// Update LED indicators and buzzer based on PPM level
void update_indicators(float ppm) {
    // Reset all indicators
    LED_SAFE = 0;
    LED_WARNING = 0;
    LED_DANGER = 0;
    BUZZER = 0;

    if(ppm < SAFE_THRESHOLD) {
        // Safe level - Green LED only
        LED_SAFE = 1;
        Send2Lcd(0x80, "AIR QUALITY:SAFE");
    }
    else if(ppm < WARNING_THRESHOLD) {
        // Warning level - Yellow LED and buzzer
        LED_WARNING = 1;
        BUZZER = 1;
        Send2Lcd(0x80, "AIR QUAL:WARNING");

        // Pulse the buzzer
        delay_ms(100);
        BUZZER = 0;
        delay_ms(100);
        BUZZER = 1;
        delay_ms(100);
        BUZZER = 0;
    }
    else {
        // Danger level - Red LED
        LED_DANGER = 1;
        Send2Lcd(0x80, "AIR QUAL:DANGER ");

        // Rapid buzzer pulses for danger alert
        for(int i = 0; i < 3; i++) {
            BUZZER = 1;
            delay_ms(50);
            BUZZER = 0;
            delay_ms(50);
        }
    }
}

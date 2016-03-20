#include "application.h"
uint16_t upstream = 0;
uint16_t current_upstream = 0;
uint16_t downstream = 0;
uint16_t current_downstream = 0;
uint32_t last_set = 0;
//int LED = D6;
int LED = A4;
int led_state = HIGH;
bool moved = false;

int setMeter( String command )
{
    last_set = millis();
    Serial.print( "setMeter received command: " );
    Serial.println( command );

    uint16_t comma_at = command.indexOf( ',' );
    String downstream_str = command.substring( 0, comma_at );
    String upstream_str = command.substring( comma_at + 1, command.length() );
    
    Serial.print( "Down/Up percentage: " );
    Serial.print( downstream_str );
    Serial.print( "% / " );
    Serial.print( upstream_str );
    Serial.println( "%" );
    
    // 4095 == 3.3V, but the meters I have are 3V, so 0 -> 3722 is the range we can use
    upstream = ( upstream_str.toInt() * 3722 ) / 100;
    downstream = ( downstream_str.toInt() * 3722 ) / 100;
    Serial.print( "Down/Up DAC value: " );
    Serial.print( downstream );
    Serial.print( " / " );
    Serial.println( upstream );

    return 1;
}

// Move the meter towards a target taking reasonable steps on the way
uint16_t moveTowards( int pin, uint16_t current, uint16_t target ){
    int diff_direction = target - current;
    int diff = (int)( ( target - current ) / 100 );

    // Ensure that we aproach the end target with at least 10 steps per cycle
    if( diff_direction > 0 ){
        if( diff < 10 ){
            if( diff_direction > 10 ){
                diff = 10;
            }else{
                diff = diff_direction;
            }
        }
    }else{
        if( diff > -10 ){
            if( diff_direction < -10 ){
                diff = -10;
            }else{
                diff = diff_direction;
            }
        }
    }

    Serial.print( "Adjusting pin " );
    Serial.print( pin );
    Serial.print( " current/target/diff : " );
    Serial.print( current );
    Serial.print( "/" );
    Serial.print( target );
    Serial.print( "/" );
    Serial.println( diff );
    current += diff;
    analogWrite( pin, current );
    return current;
}

void setup() {
    Serial.begin(9600);
    pinMode(DAC1, OUTPUT);
    pinMode(DAC2, OUTPUT);
    pinMode(LED, OUTPUT);  
    digitalWrite(LED, HIGH);
    Particle.function( "set", setMeter );
}

void loop() {
    // There should be an update every 10 seconds - light LED if nothing for 20 seconds to 
    // warn that the display is stale
    if( millis() - last_set > 20000 ){
        if( led_state == LOW ){
            Serial.println( "Setting LED on" );
            led_state = HIGH;
        }
    }else{
        if( led_state == HIGH ){
            Serial.println( "Setting LED off" );
            led_state = LOW;
        }
    }
    digitalWrite(LED, led_state);
        
    moved = false;
    if( current_upstream != upstream ){
        current_upstream = moveTowards( DAC2, current_upstream, upstream );
        moved = true;
    }

    if( current_downstream != downstream ){
        current_downstream = moveTowards( DAC1, current_downstream, downstream );
        moved = true;
    }
    // delay a bit if something moved to allow smooth moves
    if( moved == true ){
        delay( 10 );
    }
}


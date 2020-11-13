#ifndef encoder_h
#define encoder_h

class Encoder
{
    private:
    //Motors are 3300 RPM at no load
    //each rotation is 12 encoder counts
    //int has a range of +-2,147,483,647, and potentially could last 54229 minutes at full speed
    //short is too short, and lasts around .8 minutes at full speed

    int counterUnit;
    
    public:

    Encoder(int pin1, int pin2, int counterunit);

    int count();
    
};
#endif

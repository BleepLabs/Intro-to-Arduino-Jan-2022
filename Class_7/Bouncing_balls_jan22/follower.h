#ifndef analyze_follower_h_
#define analyze_follower_h_

#include "Arduino.h"
#include "AudioStream.h"

class AudioAnalyzeFollower : public AudioStream
{
  public:
    AudioAnalyzeFollower(void) : AudioStream(0, NULL) {}

    void trig() {
      mode = 1;
      slope = 10;
    }

    int32_t current() {
      return slope;
    }

    virtual void update(void);
    
  private:
    float slope;
    byte mode;
    float rise_rate= 1.1;
    float fall_rate= 0.9995;
    uint32_t fixed_slope;
    uint32_t fixed_rise;
    uint32_t fixed_fall;
    
};

#endif



#include <wonderful.h>
#include <ws.h>
#include <wse/memory.h>

#include "waveforms.h"

#define WS_SOUND_WAVETABLE_CH1 0x00
#define WS_SOUND_WAVETABLE_CH2 0x01
#define WS_SOUND_WAVETABLE_CH3 0x02
#define WS_SOUND_WAVETABLE_CH4 0x03
#define WS_SOUND_VOLUME(left, right) (((left & 0xF) << 4) | (right & 0xF))

static void load_waveform(const uint8_t __wf_rom* packed_data, uint8_t channel)
{
    memcpy(wse_wavetable1.wave[channel].data, packed_data, 16);
}

void main(void)
{
    uint16_t last_keys = 0;
    uint16_t curr_keys = 0;
    uint16_t pressed_keys = curr_keys & ~last_keys;
    uint16_t released_keys = ~curr_keys & last_keys;
    uint8_t sound_volume = 0xFF;
    uint8_t sound_enabled = 0xFF;
    uint16_t test_frequency = 0x5000u; // in hz

    // enable sound channel
    ws_sound_set_wavetable_address(&wse_wavetable1);

    // set sound output to not distord the speaker
    outportb(WS_SOUND_OUT_CTRL_PORT, (WS_SOUND_OUT_CTRL_SPEAKER_VOLUME_200 << WS_SOUND_OUT_CTRL_SPEAKER_VOLUME_SHIFT) | 
                                     WS_SOUND_OUT_CTRL_SPEAKER_ENABLE); 

    // write waveform to channel's wavetable ram
    load_waveform(square_wave, WS_SOUND_WAVETABLE_CH1);

    // calculate the frequency to play and write it to channel's frequency register
    outportw(WS_SOUND_FREQ_CH1_PORT, WS_SOUND_UPDATE_HZ_TO_FREQ(test_frequency));

    // set channel valume to enable and disable it
    outportb(WS_SOUND_VOL_CH1_PORT, WS_SOUND_VOLUME(sound_enabled & 0x7, sound_enabled & 0x7)); 
    outportb(WS_SOUND_CH_CTRL_PORT, WS_SOUND_CH_CTRL_CH1_ENABLE);

    // test loop handles input on vblank
    ws_int_set_default_handler_key();
    ws_int_set_default_handler_vblank();
	ws_int_enable(WS_INT_ENABLE_VBLANK | WS_INT_ENABLE_KEY_SCAN);

    while(1)
    {
        ia16_halt(); // will pass on interrrupt(vblank)
        last_keys = curr_keys;
        curr_keys = ws_keypad_scan();

        pressed_keys = curr_keys & ~last_keys;
        released_keys = ~curr_keys & last_keys;

        if(released_keys & WS_KEY_X4)
        {
            load_waveform(sine_wave, WS_SOUND_WAVETABLE_CH1);
        }
        if(released_keys & WS_KEY_X3)
        {
            load_waveform(triangle_wave, WS_SOUND_WAVETABLE_CH1);
        }
        if(released_keys & WS_KEY_X2)
        {
            load_waveform(sawtooth_wave, WS_SOUND_WAVETABLE_CH1);
        }
        if(released_keys & WS_KEY_X1)
        {
            load_waveform(square_wave, WS_SOUND_WAVETABLE_CH1);
        }
        if(pressed_keys & WS_KEY_B)
        {
            sound_volume = ~sound_volume;
            outportb(WS_SOUND_VOL_CH1_PORT, WS_SOUND_VOLUME(sound_volume & 0x7, sound_volume & 0x7)); 
        }
        if(pressed_keys & WS_KEY_A)
        {
            sound_enabled = ~sound_enabled;
            outportb(WS_SOUND_CH_CTRL_PORT, sound_enabled & WS_SOUND_CH_CTRL_CH1_ENABLE);
        }
        if(curr_keys & WS_KEY_Y2 )
        {
            if(test_frequency < 0xFFFF)
            {
                ++test_frequency;
                outportw(WS_SOUND_FREQ_CH1_PORT, WS_SOUND_UPDATE_HZ_TO_FREQ(test_frequency));
            }
        }
        if(curr_keys & WS_KEY_Y4 )
        {
            if(test_frequency > 0x01)
            {
                --test_frequency;
                outportw(WS_SOUND_FREQ_CH1_PORT, WS_SOUND_UPDATE_HZ_TO_FREQ(test_frequency));
            }
        }
    }
}

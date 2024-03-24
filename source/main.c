#if defined(WIN32)
#pragma comment(lib, "winmm.lib")
#include <Windows.h>
#include <mmeapi.h>
#elif defined(__APPLE__)
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

#define RGFWDEF
#define RGFW_BUFFER
#include "RGFW.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "PureDOOM.h"

#include "stb_image_resize2.h"

#include "miniaudio.h"


/* Resolution DOOM renders at */
#define WIDTH 320
#define HEIGHT 200

doom_key_t RGFW_keycode_to_doom_key(u32 keycode) {
    switch (keycode) {
        case RGFW_Tab: return DOOM_KEY_TAB;
        case RGFW_Return: return DOOM_KEY_ENTER;
        case RGFW_Escape: return DOOM_KEY_ESCAPE;
        case RGFW_Space: return DOOM_KEY_SPACE;
        case RGFW_Quote: return DOOM_KEY_APOSTROPHE;
        case RGFW_Multiply: return DOOM_KEY_MULTIPLY;
        case RGFW_Comma: return DOOM_KEY_COMMA;
        case RGFW_Minus: return DOOM_KEY_MINUS;
        case RGFW_Period: return DOOM_KEY_PERIOD;
        case RGFW_Slash: return DOOM_KEY_SLASH;
        case RGFW_0: return DOOM_KEY_0;
        case RGFW_1: return DOOM_KEY_1;
        case RGFW_2: return DOOM_KEY_2;
        case RGFW_3: return DOOM_KEY_3;
        case RGFW_4: return DOOM_KEY_4;
        case RGFW_5: return DOOM_KEY_5;
        case RGFW_6: return DOOM_KEY_6;
        case RGFW_7: return DOOM_KEY_7;
        case RGFW_8: return DOOM_KEY_8;
        case RGFW_9: return DOOM_KEY_9;
        case RGFW_Semicolon: return DOOM_KEY_SEMICOLON;
        case RGFW_Equals: return DOOM_KEY_EQUALS;
        case RGFW_Bracket: return DOOM_KEY_LEFT_BRACKET;
        case RGFW_CloseBracket: return DOOM_KEY_RIGHT_BRACKET;
        case RGFW_A: return DOOM_KEY_A;
        case RGFW_B: return DOOM_KEY_B;
        case RGFW_C: return DOOM_KEY_C;
        case RGFW_D: return DOOM_KEY_D;
        case RGFW_E: return DOOM_KEY_E;
        case RGFW_F: return DOOM_KEY_F;
        case RGFW_G: return DOOM_KEY_G;
        case RGFW_H: return DOOM_KEY_H;
        case RGFW_I: return DOOM_KEY_I;
        case RGFW_J: return DOOM_KEY_J;
        case RGFW_K: return DOOM_KEY_K;
        case RGFW_L: return DOOM_KEY_L;
        case RGFW_M: return DOOM_KEY_M;
        case RGFW_N: return DOOM_KEY_N;
        case RGFW_O: return DOOM_KEY_O;
        case RGFW_P: return DOOM_KEY_P;
        case RGFW_Q: return DOOM_KEY_Q;
        case RGFW_R: return DOOM_KEY_R;
        case RGFW_S: return DOOM_KEY_S;
        case RGFW_T: return DOOM_KEY_T;
        case RGFW_U: return DOOM_KEY_U;
        case RGFW_V: return DOOM_KEY_V;
        case RGFW_W: return DOOM_KEY_W;
        case RGFW_X: return DOOM_KEY_X;
        case RGFW_Y: return DOOM_KEY_Y;
        case RGFW_Z: return DOOM_KEY_Z;
        case RGFW_BackSpace: return DOOM_KEY_BACKSPACE;
        
        #ifndef RGFW_WINDOWS
        case RGFW_ControlL:
        #endif

        case RGFW_ControlR: return DOOM_KEY_CTRL;
        case RGFW_Left: return DOOM_KEY_LEFT_ARROW;
        case RGFW_Up: return DOOM_KEY_UP_ARROW;
        case RGFW_Right: return DOOM_KEY_RIGHT_ARROW;
        case RGFW_Down: return DOOM_KEY_DOWN_ARROW;
        case RGFW_ShiftL:
        case RGFW_ShiftR: return DOOM_KEY_SHIFT;
        case RGFW_AltL:
        case RGFW_AltR: return DOOM_KEY_ALT;
        case RGFW_F1: return DOOM_KEY_F1;
        case RGFW_F2: return DOOM_KEY_F2;
        case RGFW_F3: return DOOM_KEY_F3;
        case RGFW_F4: return DOOM_KEY_F4;
        case RGFW_F5: return DOOM_KEY_F5;
        case RGFW_F6: return DOOM_KEY_F6;
        case RGFW_F7: return DOOM_KEY_F7;
        case RGFW_F8: return DOOM_KEY_F8;
        case RGFW_F9: return DOOM_KEY_F9;
        case RGFW_F10: return DOOM_KEY_F10;
        case RGFW_F11: return DOOM_KEY_F11;
        case RGFW_F12: return DOOM_KEY_F12;
        default: return DOOM_KEY_UNKNOWN;
    }

    return DOOM_KEY_UNKNOWN;
}


doom_button_t RGFW_button_to_doom_button(u8 RGFW_button) {
    switch (RGFW_button) {
        case RGFW_mouseLeft: return DOOM_LEFT_BUTTON;
        case RGFW_mouseRight: return DOOM_RIGHT_BUTTON;
        case RGFW_mouseMiddle: return DOOM_MIDDLE_BUTTON;
    }
    return (doom_button_t)3;
}

ma_bool32 g_isPaused = MA_TRUE;

void audio_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {   
    int16_t* buffer = doom_get_sound_buffer();
    
    if (g_isPaused)
        return;

    memcpy(pOutput, buffer, frameCount * pDevice->playback.channels * sizeof(ma_uint16));
}


/* this can also be used to convert BGR to RGB */
void bitmap_rgbToBgr(u8* bitmap, RGFW_area a) {
    /* loop through eacfh *pixel* (not channel) of the buffer */
    u32 x, y;
    for (y = 0; y < a.h; y++) {
        for (x = 0; x < a.w; x++) {
            u32 index = (y * 4 * a.w) + x * 4;

            u8 red = bitmap[index];
            bitmap[index] = bitmap[index + 2];
            bitmap[index + 2] = red;
        }
    }    
}

#if defined(WIN32)
static HMIDIOUT midi_out_handle = 0;
void send_midi_msg(uint32_t midi_msg)
{
    if (midi_out_handle)
    {
        while ((midi_msg = doom_tick_midi()) != 0)
            midiOutShortMsg(midi_out_handle, midi_msg);
    }
}
#elif defined(__APPLE__)
AudioUnit audio_unit = 0;
void send_midi_msg(uint32_t midi_msg)
{
    if (audio_unit)
    {
        MusicDeviceMIDIEvent(audio_unit, 
            (midi_msg) & 0xFF,
            (midi_msg >> 8) & 0xFF,
            (midi_msg >> 16) & 0xFF,
            0);
    }
}
#else
void send_midi_msg(uint32_t midi_msg) {}
#endif

u8 tick_midi(u32 interval, void *param) {
    uint32_t midi_msg;

    while ((midi_msg = doom_tick_midi()) != 0) send_midi_msg(midi_msg);

#if defined(__APPLE__)
    return 1000 / DOOM_MIDI_RATE - 1; // Weirdly, on Apple music is too slow
#else
    return 1000 / DOOM_MIDI_RATE;
#endif
}

time_t start_time;
void thread(void*) {
    while (1) {
        u8 midi_time = tick_midi(0, 0); // Call the MIDI tick function
        
     /*   struct timespec time;
        time.tv_sec = midi_time;
        time.tv_nsec = midi_time * 600;


        nanosleep(&time , NULL);
        */
    }
}

int main() {
    RGFW_window* window = RGFW_createWindow("RGFW DOOM", RGFW_RECT(0, 0, 500, 500), RGFW_CENTER);

    RGFW_area screenSize = RGFW_getScreenSize();

    u8* texture = malloc(screenSize.w * screenSize.h * 4);

    // SDL Audio thread
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format   = ma_format_s16;   // Set to ma_format_unknown to use the device's native format.
    config.playback.channels = 2;               // Set to 0 to use the device's native channel count.
    config.sampleRate        = DOOM_SAMPLERATE;           // Set to 0 to use the device's native sample rate.
    config.dataCallback      = audio_callback;   // This function will be called when miniaudio needs more data.
    //audio_spec.samples = 512;

    ma_device device;
    if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
        printf("Failed to init miniaudio device\n");
        return 1;
    }

    ma_device_start(&device);

    // Capture mouse
    RGFW_window_showMouse(window, 0);
    RGFW_window_mouseHold(window);

    //-----------------------------------------------------------------------
    // Setup DOOM
    //-----------------------------------------------------------------------

    // Change default bindings to modern
    doom_set_default_int("key_up", DOOM_KEY_W);
    doom_set_default_int("key_down", DOOM_KEY_S);
    doom_set_default_int("key_strafeleft", DOOM_KEY_A);
    doom_set_default_int("key_straferight", DOOM_KEY_D);
    doom_set_default_int("key_use", DOOM_KEY_E);
    
    doom_set_default_int("mouse_move", 0); // Mouse will not move forward
   
    // Setup resolution
    doom_set_resolution(WIDTH, HEIGHT);

    #ifdef RGFW_WINDOWS
    // Setup MIDI for songs
    if (midiOutGetNumDevs() != 0)
        midiOutOpen(&midi_out_handle, 0, 0, 0, 0);
    #endif

    // Initialize doom
    doom_init(NULL, 0, DOOM_FLAG_MENU_DARKEN_BG);

    // Main loop
    g_isPaused = MA_FALSE;

    int done = 0;

    int active_mouse = 1; // Dev allow us to take mouse out of window

    u32 lastTime = RGFW_getTimeNS();
    u8 midi_time = 1000 / DOOM_MIDI_RATE;

    RGFW_createThread(tick_midi, NULL);

    while (!done) {
        RGFW_vector mouse = RGFW_VECTOR(0, 0);

       // if (RGFW_getTimeNS() - lastTime) {
            //midi_time = tick_midi(0, 0);
           // lastTime = RGFW_getTimeNS();
        //}

        while (RGFW_window_checkEvent(window)) {
            switch (window->event.type) {
                case RGFW_quit:
                    done = 1;
                    break;

                case RGFW_keyPressed:
                    if (window->event.keyCode == RGFW_End || window->event.keyCode == RGFW_Escape)
                    {
                        RGFW_window_showMouse(window, active_mouse);
                        if (active_mouse)
                            RGFW_window_mouseUnhold(window);
                        else
                            RGFW_window_mouseHold(window);
                        
                        active_mouse = !active_mouse;
                    }

                    doom_key_down(RGFW_keycode_to_doom_key(window->event.keyCode));
                    break;

                case RGFW_keyReleased:
                    doom_key_up(RGFW_keycode_to_doom_key(window->event.keyCode));
                    break;

                case RGFW_mouseButtonPressed:
                    if (active_mouse) doom_button_down(RGFW_button_to_doom_button(window->event.button));
                    break;

                case RGFW_mouseButtonReleased:
                    if (active_mouse) doom_button_up(RGFW_button_to_doom_button(window->event.button));
                    break;

                case RGFW_mousePosChanged:
                    if (active_mouse)
                    {
                        mouse.x += window->event.point.x - (window->r.w / 2.0);
                        mouse.y += window->event.point.y - (window->r.h / 2.0);
                    }
                    break;
            }
            if (done) break;
        }
        if (done) break;

        if (mouse.x || mouse.y)
            doom_mouse_move(mouse.x * 6, mouse.y * 6);

        doom_update();
        
        const unsigned char* src = doom_get_framebuffer(4);
        
        bitmap_rgbToBgr(src, RGFW_AREA(WIDTH, HEIGHT)); 

        stbir_resize_uint8_srgb(src, WIDTH, HEIGHT, 0, texture, window->r.w, window->r.h, 0, STBIR_RGBA);

        u32 y;
        for (y = 0; y < window->r.h; y++) {
            u32 index = y * (4 * screenSize.w);
            memcpy(window->buffer + index, texture + (4 * window->r.w * y), window->r.w * 4);
        }

        RGFW_window_swapBuffers(window);
    }
    
#if defined(WIN32)
    if (midi_out_handle) midiOutClose(midi_out_handle);
#endif*/

    free(texture);

    ma_device_uninit(&device);
    RGFW_window_close(window);
    return 0;
}
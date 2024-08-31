#if defined(__APPLE__)
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

#define RSGLDEF
#define RGFWDEF

#define RGFW_BUFFER
#define RGFW_OPENGL
#include "RSGL.h"

#include "deps/RGFW.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <dirent.h>

#ifdef _WIN32
#include <wchar.h>
#include <_mingw_stat64.h>
#endif

#ifndef __APPLE__
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif

#define DOOM_IMPLEMENT_PRINT 
#define DOOM_IMPLEMENT_FILE_IO 
#define DOOM_IMPLEMENT_MALLOC 
#define DOOM_IMPLEMENT_GETTIME 
#define DOOM_IMPLEMENT_EXIT 
#define DOOM_IMPLEMENT_GETENV 
#define DOOM_IMPLEMENTATION 
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
        case RGFW_a: return DOOM_KEY_A;
        case RGFW_b: return DOOM_KEY_B;
        case RGFW_c: return DOOM_KEY_C;
        case RGFW_d: return DOOM_KEY_D;
        case RGFW_e: return DOOM_KEY_E;
        case RGFW_f: return DOOM_KEY_F;
        case RGFW_g: return DOOM_KEY_G;
        case RGFW_h: return DOOM_KEY_H;
        case RGFW_i: return DOOM_KEY_I;
        case RGFW_j: return DOOM_KEY_J;
        case RGFW_k: return DOOM_KEY_K;
        case RGFW_l: return DOOM_KEY_L;
        case RGFW_m: return DOOM_KEY_M;
        case RGFW_n: return DOOM_KEY_N;
        case RGFW_o: return DOOM_KEY_O;
        case RGFW_p: return DOOM_KEY_P;
        case RGFW_q: return DOOM_KEY_Q;
        case RGFW_r: return DOOM_KEY_R;
        case RGFW_s: return DOOM_KEY_S;
        case RGFW_t: return DOOM_KEY_T;
        case RGFW_u: return DOOM_KEY_U;
        case RGFW_v: return DOOM_KEY_V;
        case RGFW_w: return DOOM_KEY_W;
        case RGFW_x: return DOOM_KEY_X;
        case RGFW_y: return DOOM_KEY_Y;
        case RGFW_z: return DOOM_KEY_Z;
        case RGFW_BackSpace: return DOOM_KEY_BACKSPACE; 
		case RGFW_AltL:
        case RGFW_AltR: return DOOM_KEY_ALT;
        case RGFW_ControlL: 
		case RGFW_ControlR: return DOOM_KEY_CTRL;
        case RGFW_Left: return DOOM_KEY_LEFT_ARROW;
        case RGFW_Up: return DOOM_KEY_UP_ARROW;
        case RGFW_Right: return DOOM_KEY_RIGHT_ARROW;
        case RGFW_Down: return DOOM_KEY_DOWN_ARROW;        
        case RGFW_ShiftL:
		case RGFW_ShiftR: return DOOM_KEY_SHIFT;
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

doom_button_t RSGL_button_to_doom_button(u8 RSGL_button) {
    switch (RSGL_button) {
        case RGFW_mouseLeft: return DOOM_LEFT_BUTTON;
        case RGFW_mouseRight: return DOOM_RIGHT_BUTTON;
        case RGFW_mouseMiddle: return DOOM_MIDDLE_BUTTON;
    }
    return (doom_button_t)3;
}

/* this can also be used to convert BGR to RGB */
void bitmap_rgbToBgr(u8* bitmap, RSGL_area a) {
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


u32 tick_midi(u32 interval, void *param)
{
    u32 midi_msg;

    while ((midi_msg = doom_tick_midi()) != 0) 
        send_midi_msg(midi_msg);

#if defined(__APPLE__)
    return (DOOM_MIDI_RATE - 1) * 5e+4; // Weirdly, on Apple music is too slow
#else
    return DOOM_MIDI_RATE * 5e+4;
#endif
}

ma_bool8 g_isPaused = MA_TRUE;
void audio_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {   
    if (g_isPaused)
        return;

    i16* buffer = doom_get_sound_buffer();
    memcpy(pOutput, buffer, frameCount * pDevice->playback.channels * sizeof(ma_uint16));
}


void* thread(void* args) {
    static u32 midi_time = 0;
    static u32 lastTime = 0;

    while (1) {
        if (RGFW_getTimeNS() - lastTime >= midi_time) {
            midi_time = tick_midi(0, 0);
            lastTime = RGFW_getTimeNS();
        }
    }
}

int main (int argc, char** argv) {
    doom_set_pitch_shift(false);

    u32 i;
    char** foundWads = malloc(sizeof(char*) * 50);
    for (i = 0; i < 50; i++) 
        foundWads[i] = malloc(sizeof(char) * 1000);

    char* wadFolders[] = {"./"};

    struct dirent *dp;
    DIR *dfd;

    size_t wadCount = 0;

    for (i = 0; i < sizeof(wadFolders) / sizeof(char*); i++) {
        if ((dfd = opendir(wadFolders[i])) == NULL) {
            fprintf(stderr, "Can't open %s\n", wadFolders[i]);
            continue;
        }

        size_t folderLen = strlen(wadFolders[i]);

        char filename[255];
        while ((dp = readdir(dfd)) != NULL) {
            size_t len = strlen(dp->d_name);
            
            if (len >= 4 && (strncmp(dp->d_name + len - 4, ".wad", 4) == 0 || 
                            strncmp(dp->d_name + len - 4, ".WAD", 4) == 0)) 
            {
                snprintf(foundWads[wadCount], 300, "%s%s", wadFolders[i], dp->d_name);
                wadCount++;
            }
        }
    }

    if (wadCount == 0) {
        printf("Failed to find any .WAD files\n");
        return 0;
    }

    RGFW_window* window = RGFW_createWindow("RSGL DOOM", RGFW_RECT(0, 0, 500, 500), RGFW_CENTER);
    
	RSGL_init(RSGL_AREA(window->r.w, window->r.h), RGFW_getProcAddress);

	RSGL_setFont(RSGL_loadFont("Super Easy.ttf"));    

    RGFW_area screenSize = RGFW_getScreenSize();
    size_t buffer_stride = screenSize.w * 4;
    size_t doom_stride = WIDTH * 4;

    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format   = ma_format_s16;
    config.playback.channels = 2;
    config.sampleRate        = DOOM_SAMPLERATE;
    config.dataCallback      = audio_callback;
    config.periodSizeInFrames = 512;

    ma_device device, midi_device;
    if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
        printf("Failed to init miniaudio device\n");
        return 1;
    }

    ma_device_start(&device);

    //-----------------------------------------------------------------------
    // Setup DOOM
    //-----------------------------------------------------------------------

    // Change default bindings to modern
    doom_set_default_int("key_up", DOOM_KEY_W);
    doom_set_default_int("key_down", DOOM_KEY_S);
    doom_set_default_int("key_strafeleft", DOOM_KEY_A);
    doom_set_default_int("key_straferight", DOOM_KEY_D);
    doom_set_default_int("key_use", DOOM_KEY_E);
    doom_set_default_int("key_jump", DOOM_KEY_SPACE);
    
    doom_set_default_int("mouse_move", 0); // Mouse will not move forward
   
    // Setup resolution
    doom_set_resolution(WIDTH, HEIGHT);

    #ifdef RGFW_WINDOWS
    // Setup MIDI for songs
    if (midiOutGetNumDevs() != 0)
        midiOutOpen(&midi_out_handle, 0, 0, 0, 0);

    #elif defined(__APPLE__)
	AUGraph graph;
	AUNode outputNode, mixerNode, dlsNode;
	NewAUGraph(&graph);
	AudioComponentDescription output = {'auou','ahal','appl',0,0};
	AUGraphAddNode(graph, &output, &outputNode);
	AUGraphOpen(graph);
	AUGraphInitialize(graph);
	AUGraphStart(graph);
	AudioComponentDescription dls = {'aumu','dls ','appl',0,0};
	AUGraphAddNode(graph, &dls, &dlsNode);
	AUGraphNodeInfo(graph, dlsNode, NULL, &audio_unit);
	AudioComponentDescription mixer = {'aumx','smxr','appl',0,0};
	AUGraphAddNode(graph, &mixer, &mixerNode);
	AUGraphConnectNodeInput(graph,mixerNode,0,outputNode,0);
	AUGraphConnectNodeInput(graph,dlsNode,0,mixerNode,0);
	AUGraphUpdate(graph,NULL);
    #endif

    // Main loop
    g_isPaused = MA_FALSE;

    int done = 0;

    int active_mouse = 1; // Dev allow us to take mouse out of window

    RGFW_createThread(thread, NULL);

    STBIR_RESIZE resize;

    stbir_resize_init(&resize, NULL, WIDTH, HEIGHT, doom_stride, window->buffer, 0, 0, buffer_stride, 
                            STBIR_RGBA_NO_AW, STBIR_TYPE_UINT8_SRGB);
    
    resize.fast_alpha = 1;
    resize.horizontal_filter = STBIR_FILTER_BOX;
    resize.vertical_filter = STBIR_FILTER_BOX;

    bool doom_start = false;
    u32 texture = RSGL_renderCreateTexture(NULL, RSGL_AREA(WIDTH, HEIGHT), 4);
	
	u32 fps = 0;

	u8 radio_select = 0,
		radio_select2 = 0,
		engineVersion = 0;

	size_t selected_wad = 0;

	b8 pitchShift = 0, combo_open = 0, renderChoice = 0;

    i32 j = 0;
    while (!done) { 
        while (RSGL_checkEvent(window)) {
            switch (window->event.type) {
                case RGFW_quit:
                    done = 1;
                    break;

                case RGFW_keyPressed:
                    if (doom_start == false)
                        break;
                    
					/* I don't know a better way to do this without it causing
					 * bugs or having to modify PureDOOM
					*/
					if (window->event.keyCode == RGFW_End || (window->event.keyCode == RGFW_ShiftL && RGFW_isPressed(window, RGFW_Escape)))
                    {
						RGFW_window_showMouse(window, active_mouse);
                        RGFW_window_mouseUnhold(window);   
						active_mouse = 0;
                    }

                    doom_key_down(RGFW_keycode_to_doom_key(window->event.keyCode));
                    break;

                case RGFW_keyReleased:
                    doom_key_up(RGFW_keycode_to_doom_key(window->event.keyCode));
                    break;

                case RGFW_mouseButtonPressed:
                    if (active_mouse) doom_button_down(RSGL_button_to_doom_button(window->event.button));
					else {
						active_mouse = 1;
						RGFW_window_showMouse(window, 0);
                        RGFW_window_mouseHold(window, RGFW_AREA(0, 0));   
					}
                    break;

                case RGFW_mouseButtonReleased:
                    if (active_mouse) doom_button_up(RSGL_button_to_doom_button(window->event.button));
                    break;

                case RGFW_mousePosChanged:
                    if (active_mouse) doom_mouse_move(window->event.point.x * 10, window->event.point.y * 10);
                    break;
            }
            if (done) break;
        }
        if (done) break;

        if (doom_start) {
            doom_update();
            
            resize.input_pixels = doom_get_framebuffer(4);
    
            if (renderChoice == 0) {
                resize.output_w = resize.output_subw= window->r.w;
                resize.output_h = resize.output_subh = window->r.h; 
                stbir_resize_extended(&resize);
            
                RGFW_window_setCPURender(window, 1);
                RGFW_window_setGPURender(window, 0);

            } else {
                glBindTexture(GL_TEXTURE_2D, texture);    
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, resize.input_pixels);
                
                RSGL_setTexture(texture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, resize.input_pixels);

                RSGL_drawRect(RSGL_RECT(0, 0, window->r.w, window->r.h), RSGL_RGB(255, 255, 255));
            
                RGFW_window_setCPURender(window, 0);
                RGFW_window_setGPURender(window, 1);
            }
        }

        else if (doom_start == false) {
			RSGL_openBlankContainer(RSGL_RECT(0, 0, window->r.w, window->r.h));

			RSGL_widgetAlign(RSGL_ALIGN_LEFT | RSGL_ALIGN_MIDDLE);
			RSGL_combobox(RSGL_RECTF(30, 30, 200, 25), RSGL_STYLE_DARK, foundWads, wadCount, &combo_open, &selected_wad);

			RSGL_widgetAlign(RSGL_ALIGN_LEFT | RSGL_ALIGN_UP);
			RSGL_label("Select a .WAD", RSGL_RECTF(30, 5, 200, 25));

			RSGL_label("Engine Version", RSGL_RECTF(250, 5, 15, 15));
            RSGL_label("DOOM 1", RSGL_RECTF(250, 35, 15, 15));
			RSGL_label("DOOM 2", RSGL_RECTF(250, 50, 15, 15));
			RSGL_radioButtons(RSGL_RECTF(400, 35, 15, 15), 2, RSGL_STYLE_DARK, &radio_select, &engineVersion);

			RSGL_label("Renderer", RSGL_RECTF(250, 123, 15, 15));
            RSGL_label("CPU Buffer", RSGL_RECTF(250, 153, 15, 15));
            RSGL_label("OpenGL Buffer", RSGL_RECTF(250, 173, 15, 25));  
			RSGL_radioButtons(RSGL_RECTF(400, 153, 15, 15), 2, RSGL_STYLE_DARK, &radio_select2, &renderChoice);

			RSGL_label("Pitch Shift", RSGL_RECTF(250, 200, 40, 20));
			RSGL_toggleButton(RSGL_RECTF(400, 210, 40, 20), RSGL_STYLE_ROUND | RSGL_STYLE_DARK, &pitchShift);

			RSGL_widgetAlign(RSGL_ALIGN_CENTER | RSGL_ALIGN_MIDDLE);
			if (RSGL_labeledButton("Play Game!", RSGL_RECTF(30, 400, 100, 25), RSGL_STYLE_DARK | RSGL_STYLE_ROUND) == RSGL_PRESSED) {
                    doom_start = true;
                    // Capture mouse
                    RGFW_window_showMouse(window, 0);
                    RGFW_window_mouseHold(window, RGFW_AREA(0, 0));
					
                    u32 gameMode = registered; 
                    if (engineVersion - 1)
                        gameMode = commercial;

                    doom_set_pitch_shift(pitchShift);

                    // Initialize doom
					doom_init(argc, argv, DOOM_FLAG_MENU_DARKEN_BG, foundWads[selected_wad], gameMode);
					
					for (i = 0; i < 50; i++) 
                        free(foundWads[i]);
                    free(foundWads);
			}



			RGFW_window_setGPURender(window, 1);
			RGFW_window_setCPURender(window, 0);
        }
        
        RSGL_clear(RSGL_RGB(30, 30, 40));
		RGFW_window_swapBuffers(window);
		fps = RGFW_window_checkFPS(window, 0);
	}
    
#if defined(WIN32)
    if (midi_out_handle) midiOutClose(midi_out_handle);
#endif

    RSGL_renderDeleteTexture(texture);

    ma_device_uninit(&device);

	RSGL_free();

	RGFW_window_close(window);
    return 0;
}

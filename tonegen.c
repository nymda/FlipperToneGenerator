#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>

float freq = 500.f;
int volume = 100;

static void draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_clear(canvas);
    canvas_draw_frame(canvas, 0, 0, 128, 64);

    char txtBufferTone[16];
    snprintf(txtBufferTone, 16, "Tone: %0.2fHz", (double)freq);

    char txtBufferVol[16];
    snprintf(txtBufferVol, 16, "Volume: %i%%", volume);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 7, 15, "Tone generator");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 7, 30, txtBufferTone);
    canvas_draw_str(canvas, 7, 40, txtBufferVol);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t tonegen_app(void* p) { 
    UNUSED(p);

    InputEvent event;
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, NULL);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    void* speaker = (void*)furi_hal_speaker_acquire(1000);

    //furi_hal_subghz_reset();
    //furi_hal_subghz_load_preset(FuriHalSubGhzPreset2FSKDev476Async);

    //uint32_t frequency = 433920000;
    //frequency = furi_hal_subghz_set_frequency_and_path(frequency);

    while(true) {
        furi_check(furi_message_queue_get(event_queue, &event, FuriWaitForever) == FuriStatusOk);

        if(event.type == InputTypePress){
            if(event.key == InputKeyBack) {
                break;
            }
            if(event.key == InputKeyUp) {
                if(freq < 2500.f){
                    freq += 25.f;
                }
            }
            if(event.key == InputKeyDown) {
                if(freq > 100.f){
                    freq -= 25.f;
                }
            }
            if(event.key == InputKeyRight) {
                if(volume < 100){
                    volume += 5;
                }
            }
            if(event.key == InputKeyLeft) {
                if(volume > 5){
                    volume -= 5;
                }
            }
            if(event.key == InputKeyOk) {
                //furi_hal_subghz_tx();
                furi_hal_speaker_start(freq, (float)(volume / 100.f));
            }
        } 
        else if(event.type == InputTypeRelease) {
            //furi_hal_subghz_idle(); //end carrier
            furi_hal_speaker_stop(); //end tone
        }
    }

    furi_hal_speaker_release(speaker);
    furi_message_queue_free(event_queue);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);

    return 0;
}
#include <furi.h>
#include <dialogs/dialogs.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/empty_screen.h>
#include <m-string.h>
#include <furi_hal_version.h>

// For the tunes
#include "../music_player/music_player_worker.h"

#define GUI_DISPLAY_WIDTH 128

typedef DialogMessageButton (*TuesdayDialogScreen)(DialogsApp* dialogs, DialogMessage* message);

static DialogMessageButton banner_screen(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;

    dialog_message_set_icon(message, &I_BANNER_128x50, 0, 6);

    const char* screen_banner = "HACK TUESDAY\n   NIGHT\n";
    dialog_message_set_header(message, screen_banner, 0, 0, AlignLeft, AlignTop);
    // dialog_message_set_text(message, screen_text, 0, 26, AlignLeft, AlignTop);
    result = dialog_message_show(dialogs, message);
    dialog_message_set_header(message, NULL, 0, 0, AlignLeft, AlignTop);
    dialog_message_set_icon(message, NULL, 0, 0);

    return result;
}

static DialogMessageButton greetz_screen(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;

    const char* screen_banner = ".:~ gReEtZ ~:.";

    const char* screen_text = "llama - linse - puck\n"
                              "s4y - Hackerm0m - cel\n"
                              "naltroc - maxd - mustafa\n"
                              "and those ive missed <3";

    
    dialog_message_set_header(message, screen_banner, GUI_DISPLAY_WIDTH / 2, 0, AlignCenter, AlignTop);
    dialog_message_set_text(message, screen_text, GUI_DISPLAY_WIDTH / 2, 14, AlignCenter, AlignTop);
    result = dialog_message_show(dialogs, message);
    dialog_message_set_header(message, NULL, 0, 0, AlignCenter, AlignTop);
    dialog_message_set_text(message, NULL, 0, 0, AlignCenter, AlignTop);

    return result;
}

static DialogMessageButton tunez_screen(DialogsApp* dialogs, DialogMessage* message) {
    DialogMessageButton result;

    const char* screen_banner = ".:~ tunez? ~:.";
    
    dialog_message_set_header(message, screen_banner, GUI_DISPLAY_WIDTH / 2, 0, AlignCenter, AlignTop);
    result = dialog_message_show(dialogs, message);
    dialog_message_set_header(message, NULL, 0, 0, AlignCenter, AlignTop);

    return result;
}

const TuesdayDialogScreen tuesday_screens[] = {
    banner_screen,
    greetz_screen,
    tunez_screen
};

const size_t tuesday_screens_count = sizeof(tuesday_screens) / sizeof(TuesdayDialogScreen);

int32_t tuesday_app(void* p) {
    UNUSED(p);
    DialogsApp* dialogs = furi_record_open("dialogs");
    DialogMessage* message = dialog_message_alloc();

    MusicPlayerWorker* music_player_worker = music_player_worker_alloc();

    Gui* gui = furi_record_open("gui");
    ViewDispatcher* view_dispatcher = view_dispatcher_alloc();
    EmptyScreen* empty_screen = empty_screen_alloc();
    const uint32_t empty_screen_index = 0;

    size_t screen_index = 0;
    DialogMessageButton screen_result;

    // draw empty screen to prevent menu flickering
    view_dispatcher_add_view(
        view_dispatcher, empty_screen_index, empty_screen_get_view(empty_screen));
    view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_switch_to_view(view_dispatcher, empty_screen_index);

    printf("song is being defined\r\n");

    const char* song = "/any/music_player/AirWolf.fmf";

    printf("Starting main loop\r\n");

    while(1) {
        if(screen_index >= tuesday_screens_count - 1) {
            dialog_message_set_buttons(message, "", NULL, NULL);
        } else {
            dialog_message_set_buttons(message, "", NULL, "");
        }

        screen_result = tuesday_screens[screen_index](dialogs, message);

        if(screen_result == DialogMessageButtonLeft) {
            if(screen_index <= 0) {
                break;
            } else {
                screen_index--;
            }
        } else if(screen_result == DialogMessageButtonRight) {
            if(screen_index < tuesday_screens_count) {
                screen_index++;
            } else if (screen_index == 2) {
                printf("loading %s\r\n", song);

                if(!music_player_worker_load_fmf_from_file(music_player_worker, song)) {
                    printf("Failed to open file %s\r\n", song);
                }

                printf("setting volume\r\n");

                music_player_worker_set_volume(music_player_worker, 1.0f);
                printf("Starting worker\r\n");
                music_player_worker_start(music_player_worker);
                while(1) { osDelay(50); }
                music_player_worker_stop(music_player_worker);
                music_player_worker_free(music_player_worker);
            }
        } else if(screen_result == DialogMessageButtonBack) {
            break;
        }
    }

    dialog_message_free(message);
    furi_record_close("dialogs");

    view_dispatcher_remove_view(view_dispatcher, empty_screen_index);
    view_dispatcher_free(view_dispatcher);
    empty_screen_free(empty_screen);
    furi_record_close("gui");

    return 0;
}

#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <flipper_format/flipper_format_i.h>

#include <lib/toolbox/path.h>
#include <notification/notification_messages.h>

#include <lib/subghz/receiver.h>
#include <lib/subghz/transmitter.h>
#include <lib/subghz/subghz_file_encoder_worker.h>

#include <assets_icons.h>

#define TAG "UniRF Remix"

typedef struct {
    osMutexId_t* model_mutex;

    osMessageQueueId_t input_queue;

    ViewPort* view_port;
    Gui* gui;

    string_t up_file;
    string_t down_file;
    string_t left_file;
    string_t right_file;
    string_t ok_file;
    string_t empty;

    string_t up_l;
    string_t left_l;
    string_t right_l;
    string_t down_l;
    string_t ok_l;

    char* up_label;
    char* down_label;
    char* left_label;
    char* right_label;
    char* ok_label;

    int up_enabled;
    int down_enabled;
    int left_enabled;
    int right_enabled;
    int ok_enabled;

    char* send_status;
    int send_status_c;
    int processing;
    int repeat;
    int button;

    int file_result;
    int file_blank;

    string_t signal;

} UniRFRemix;

static char* char_to_str(char* str, int i) {
    char* converted = malloc(sizeof(char) * i + 1);
    memcpy(converted, str, i);

    converted[i] = '\0';

    return converted;
}

static const char* int_to_char(int number) {
    switch(number) {
    case 0:
        return "0";
    case 1:
        return "1";
    case 2:
        return "2";
    case 3:
        return "3";
    case 4:
        return "4";
    case 5:
        return "5";
    case 6:
        return "6";
    case 7:
        return "7";
    case 8:
        return "8";
    case 9:
        return "9";
    default:
        return "0";
    }
}

/*Decided not to use this
//check name for special characters and length
static char* check_special(char* filename)
{
	char stripped[11];
	
	//grab length of string
	int len = strlen(filename);

	int c = 0;
	int i;

	//remove special characters
	for (i = 0; i < len; i++)
	{
		if (isalnum((unsigned)filename[i]))
		{
			if(c < 11)
			{
				stripped[c] = filename[i];
				c++;
			}
		}
	}
	
	stripped[c] = '\0';
	
	return char_to_str(stripped, 10);
}
*/

//get filename without path
static char* extract_filename(const char* name, int len) {
    string_t tmp;
    string_init(tmp);

    //remove path
    path_extract_filename_no_ext(name, tmp);

    return char_to_str((char*)string_get_cstr(tmp), len);
}

/*
*check that map file exists
*assign variables to values within map file
*set missing filenames to N/A
*set filename as label if label definitions are missing
*set error flag if all buttons are N/A
*set error flag if missing map file
*/

void unirfremix_cfg_set_check(UniRFRemix* app) {
    string_t file_name;
    string_init(file_name);
    string_set(file_name, "/any/subghz/assets/universal_rf_map");

    Storage* storage = furi_record_open("storage");
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

    app->file_result = 0;
    app->file_blank = 0;

    app->up_enabled = 1;
    app->down_enabled = 1;
    app->left_enabled = 1;
    app->right_enabled = 1;
    app->ok_enabled = 1;

    int label_len = 12;

    //check that map file exists
    if(!flipper_format_file_open_existing(fff_data_file, string_get_cstr(file_name))) {
        FURI_LOG_I(TAG, "Could not open MAP file %s", string_get_cstr(file_name));
        app->file_result = 1;
    }
	else
	{
        //Filename Assignment/Check Start		

        //assign variables to values within map file
        //set missing filenames to N/A
        if(!flipper_format_read_string(fff_data_file, "UP", app->up_file))
		{
            FURI_LOG_I(TAG, "Could not read UP string");

            //increment file_blank for processing later
            app->file_blank++;

            //set label to "N/A"
            app->up_label = "N/A";

            //disable the ability to process the signal on button press
            app->up_enabled = 0;

            FURI_LOG_I(TAG, "Up_Enabled: %d", app->up_enabled);
        }
		else
		{
			//check name length for proper screen fit
			//then set filename as label. Might be replaced with defined label later on below.
			app->up_label = extract_filename(string_get_cstr(app->up_file), label_len);

			FURI_LOG_I(TAG, "UP file: %s", string_get_cstr(app->up_file));
		}	

        //Repeat process for Down
        if(!flipper_format_read_string(fff_data_file, "DOWN", app->down_file))
		{
            FURI_LOG_I(TAG, "Could not read DOWN string");

            app->file_blank++;
            app->down_label = "N/A";
            app->down_enabled = 0;

            FURI_LOG_I(TAG, "Down_Enabled: %d", app->down_enabled);
        }
		else
		{
            app->down_label = extract_filename(string_get_cstr(app->down_file), label_len);

            FURI_LOG_I(TAG, "DOWN file: %s", string_get_cstr(app->down_file));
        }

        //Repeat process for Left
        if(!flipper_format_read_string(fff_data_file, "LEFT", app->left_file))
		{
            FURI_LOG_I(TAG, "Could not read LEFT string");

            app->file_blank++;
            app->left_label = "N/A";
            app->left_enabled = 0;

            FURI_LOG_I(TAG, "Left_Enabled: %d", app->left_enabled);
        }
		else
		{
            app->left_label = extract_filename(string_get_cstr(app->left_file), label_len);

            FURI_LOG_I(TAG, "LEFT file: %s", string_get_cstr(app->left_file));
        }

        //Repeat process for Right
        if(!flipper_format_read_string(fff_data_file, "RIGHT", app->right_file))
		{
            FURI_LOG_I(TAG, "Could not read RIGHT string");

            app->file_blank++;
            app->right_label = "N/A";
            app->right_enabled = 0;

            FURI_LOG_I(TAG, "Right_Enabled: %d", app->right_enabled);
        }
		else
		{
            app->right_label = extract_filename(string_get_cstr(app->right_file), label_len);

            FURI_LOG_I(TAG, "RIGHT file: %s", string_get_cstr(app->right_file));
        }

        //Repeat process for Ok
        if(!flipper_format_read_string(fff_data_file, "OK", app->ok_file))
		{
            FURI_LOG_I(TAG, "Could not read OK string");

            app->file_blank++;
            app->ok_label = "N/A";
            app->ok_enabled = 0;

            FURI_LOG_I(TAG, "Ok_Enabled: %d", app->ok_enabled);
        }
		else
		{
            app->ok_label = extract_filename(string_get_cstr(app->ok_file), label_len);

            FURI_LOG_I(TAG, "OK file: %s", string_get_cstr(app->ok_file));
        }
		
		//File definitions are done.
		//File checks will follow after label assignment in order to close the universal_rf_map file without the need to reopen it again.

        //File definitions are done.
        //File checks will follow after label assignment in order to close the universal_rf_map file without the need to reopen it again.

        //Label Assignment/Check Start

        //assign variables to values within map file
        if(!flipper_format_read_string(fff_data_file, "ULABEL", app->up_l)) {
            FURI_LOG_I(TAG, "Could not read ULABEL string");

            //if Up button is disabled, set the label to "N/A";
            if(app->up_enabled == 0) {
                app->up_label = "N/A";
            }
        } else {
            //check if button is disabled, and set label to "N/A" from missing map definition above
            if(app->up_enabled == 0) {
                app->up_label = "N/A";
            } else {
                //set label from map to variable and shrink to fit screen
                app->up_label = char_to_str((char*)string_get_cstr(app->up_l), label_len);
            }

            FURI_LOG_I(TAG, "UP label: %s", app->up_label);
        }

        if(!flipper_format_read_string(fff_data_file, "DLABEL", app->down_l)) {
            FURI_LOG_I(TAG, "Could not read DLABEL string");

            if(app->down_enabled == 0) {
                app->down_label = "N/A";
            }
        } else {
            if(app->down_enabled == 0) {
                app->down_label = "N/A";
            } else {
                app->down_label = char_to_str((char*)string_get_cstr(app->down_l), label_len);
            }

            FURI_LOG_I(TAG, "DOWN label: %s", app->down_label);
        }

        if(!flipper_format_read_string(fff_data_file, "LLABEL", app->left_l)) {
            FURI_LOG_I(TAG, "Could not read LLABEL string");

            if(app->left_enabled == 0) {
                app->left_label = "N/A";
            }
        } else {
            if(app->left_enabled == 0) {
                app->left_label = "N/A";
            } else {
                app->left_label = char_to_str((char*)string_get_cstr(app->left_l), label_len);
            }

            FURI_LOG_I(TAG, "LEFT label: %s", app->left_label);
        }

        if(!flipper_format_read_string(fff_data_file, "RLABEL", app->right_l)) {
            FURI_LOG_I(TAG, "Could not read RLABEL string");

            if(app->right_enabled == 0) {
                app->right_label = "N/A";
            }
        } else {
            if(app->right_enabled == 0) {
                app->right_label = "N/A";
            } else {
                app->right_label = char_to_str((char*)string_get_cstr(app->right_l), label_len);
            }

            FURI_LOG_I(TAG, "RIGHT label: %s", app->right_label);
        }

        if(!flipper_format_read_string(fff_data_file, "OKLABEL", app->ok_l)) {
            FURI_LOG_I(TAG, "Could not read OKLABEL string");

            if(app->ok_enabled == 0) {
                app->ok_label = "N/A";
            }
        } else {
            if(app->ok_enabled == 0) {
                app->ok_label = "N/A";
            } else {
                app->ok_label = char_to_str((char*)string_get_cstr(app->ok_l), label_len);
            }

            FURI_LOG_I(TAG, "OK label: %s", app->ok_label);
        }
    }

	//Close universal_rf_map
    flipper_format_free(fff_data_file);
    furi_record_close("storage");
	
	//File Existence Check
	//Check each file definition if not already set to "N/A"
	
	//determine if files exist.
	//determine whether or not to continue to launch app with missing variables
	//if 5 files are missing, throw error

    if(app->file_blank == 5)
	{
		//trigger invalid file error screen
		app->file_result = 2;
	}
	else
	{
		//check all files
		//reset app->file_blank to redetermine if error needs to be thrown
		app->file_blank = 0;
		
		//if button is still enabled, check that file exists
		if(app->up_enabled == 1)
		{
			string_set(file_name, app->up_file);
			Storage* storage = furi_record_open("storage");
			FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
			
			if(!flipper_format_file_open_existing(fff_data_file, string_get_cstr(file_name)))
			{
				FURI_LOG_I(TAG, "Could not open UP file %s", string_get_cstr(file_name));
				
				//disable button, and set label to "N/A"
				app->up_enabled = 0;
				app->up_label = "N/A";
				app->file_blank++;
			}
			
			//close the file
			flipper_format_free(fff_data_file);
			furi_record_close("storage");
		}
		
		if(app->down_enabled == 1)
		{
			string_set(file_name, app->down_file);
			Storage* storage = furi_record_open("storage");
			FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
			
			if(!flipper_format_file_open_existing(fff_data_file, string_get_cstr(file_name)))
			{
				FURI_LOG_I(TAG, "Could not open DOWN file %s", string_get_cstr(file_name));
				
				app->down_enabled = 0;
				app->down_label = "N/A";
				app->file_blank++;
			}
			
			flipper_format_free(fff_data_file);
			furi_record_close("storage");
		}
		
		if(app->left_enabled == 1)
		{
			string_set(file_name, app->left_file);
			Storage* storage = furi_record_open("storage");
			FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
			
			if(!flipper_format_file_open_existing(fff_data_file, string_get_cstr(file_name)))
			{
				FURI_LOG_I(TAG, "Could not open LEFT file %s", string_get_cstr(file_name));
				
				app->left_enabled = 0;
				app->left_label = "N/A";
				app->file_blank++;
			}
			
			flipper_format_free(fff_data_file);
			furi_record_close("storage");
		}
		
		if(app->right_enabled == 1)
		{
			string_set(file_name, app->right_file);
			Storage* storage = furi_record_open("storage");
			FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
			
			if(!flipper_format_file_open_existing(fff_data_file, string_get_cstr(file_name)))
			{
				FURI_LOG_I(TAG, "Could not open RIGHT file %s", string_get_cstr(file_name));
				
				app->right_enabled = 0;
				app->right_label = "N/A";
				app->file_blank++;
			}
			
			flipper_format_free(fff_data_file);
			furi_record_close("storage");
		}
		
		if(app->ok_enabled == 1)
		{
			string_set(file_name, app->ok_file);
			Storage* storage = furi_record_open("storage");
			FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
			
			if(!flipper_format_file_open_existing(fff_data_file, string_get_cstr(file_name)))
			{
				FURI_LOG_I(TAG, "Could not open OK file %s", string_get_cstr(file_name));
				
				app->ok_enabled = 0;
				app->ok_label = "N/A";
				app->file_blank++;
			}
			
			flipper_format_free(fff_data_file);
			furi_record_close("storage");
		}
		
		if(app->file_blank == 5)
		{
			app->file_result = 2;
		}
		else
		{
			app->file_result = 0;
		}
	}
}

static void unirfremix_end_send(UniRFRemix* app) {
    app->processing = 0;
}

static void unirfremix_send_signal(
    UniRFRemix* app,
    uint32_t frequency,
    string_t signal,
    string_t protocol) {
    for(int x = 1; x <= app->repeat; x++) {
        frequency = frequency ? frequency : 433920000;
        FURI_LOG_E(TAG, "file to send: %s", string_get_cstr(signal));

        string_t flipper_format_string;

        if(strcmp(string_get_cstr(protocol), "RAW") == 0) {
            string_init_printf(flipper_format_string, "File_name: %s", string_get_cstr(signal));
        } else {
            unirfremix_end_send(app);
        }

        NotificationApp* notification = furi_record_open("notification");

        FlipperFormat* flipper_format = flipper_format_string_alloc();
        Stream* stream = flipper_format_get_raw_stream(flipper_format);
        stream_clean(stream);
        stream_write_cstring(stream, string_get_cstr(flipper_format_string));

        SubGhzEnvironment* environment = subghz_environment_alloc();
        SubGhzTransmitter* transmitter =
            subghz_transmitter_alloc_init(environment, string_get_cstr(protocol));
        subghz_transmitter_deserialize(transmitter, flipper_format);

        furi_hal_subghz_reset();
        furi_hal_subghz_load_preset(FuriHalSubGhzPresetOok270Async);
        furi_hal_subghz_set_frequency_and_path(frequency);

        furi_hal_subghz_reset();
        furi_hal_subghz_load_preset(FuriHalSubGhzPresetOok270Async);
        furi_hal_subghz_set_frequency_and_path(frequency);

        printf("Transmitting at %lu, repeat %d.\r\n", frequency, x);

        furi_hal_power_suppress_charge_enter();
        furi_hal_subghz_start_async_tx(subghz_transmitter_yield, transmitter);

        while(!(furi_hal_subghz_is_async_tx_complete())) {
            notification_message(notification, &sequence_blink_magenta_10);
            printf("Sending...");
            fflush(stdout);
            osDelay(333);
        }

        furi_record_close("notification");

        furi_hal_subghz_stop_async_tx();
        furi_hal_subghz_sleep();

        furi_hal_power_suppress_charge_exit();

        flipper_format_free(flipper_format);
        subghz_transmitter_free(transmitter);
        subghz_environment_free(environment);
    }

    unirfremix_end_send(app);
}

static void unirfremix_process_signal(UniRFRemix* app, string_t signal) {
    osMutexRelease(app->model_mutex);
    view_port_update(app->view_port);

    FURI_LOG_I(TAG, "signal = %s", string_get_cstr(signal));

    if(strlen(string_get_cstr(signal)) > 12) {
        string_t file_name;
        string_init(file_name);

        string_t protocol;
        string_init(protocol);

        uint32_t frequency_str;

        string_set(file_name, string_get_cstr(signal));

        Storage* storage = furi_record_open("storage");
        FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

        flipper_format_file_open_existing(fff_data_file, string_get_cstr(file_name));

        flipper_format_read_uint32(fff_data_file, "Frequency", (uint32_t*)&frequency_str, 1);

        if(!flipper_format_read_string(fff_data_file, "Protocol", protocol)) {
            FURI_LOG_I(TAG, "Could not read Protocol");
            string_set(protocol, "RAW");
        }

        flipper_format_free(fff_data_file);
        furi_record_close("storage");

        FURI_LOG_I(TAG, "%lu", frequency_str);

        unirfremix_send_signal(app, frequency_str, signal, protocol);
    } else if(strlen(string_get_cstr(signal)) < 10) {
        unirfremix_end_send(app);
    }
}

static void render_callback(Canvas* canvas, void* ctx) {
    UniRFRemix* app = ctx;
    furi_check(osMutexAcquire(app->model_mutex, osWaitForever) == osOK);

    //setup different canvas settings
    if(app->file_result == 1) {
        //If map is missing
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 62, 5, AlignCenter, AlignTop, "Config file missing.");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 62, 25, AlignCenter, AlignTop, "Please configure");
        canvas_draw_str_aligned(canvas, 62, 35, AlignCenter, AlignTop, "universal_rf_map");
        canvas_draw_str_aligned(canvas, 62, 60, AlignCenter, AlignBottom, "Hold Back to Exit.");
    } else if(app->file_result == 2) {
        //if map has no valid filenames defined
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 62, 5, AlignCenter, AlignTop, "Config is incorrect.");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 62, 25, AlignCenter, AlignTop, "Please configure");
        canvas_draw_str_aligned(canvas, 62, 35, AlignCenter, AlignTop, "universal_rf_map");
        canvas_draw_str_aligned(canvas, 62, 60, AlignCenter, AlignBottom, "Hold Back to Exit.");
    } else {
        //map found, draw all the things
        canvas_clear(canvas);

        //canvas_set_font(canvas, FontPrimary);
        //canvas_draw_str(canvas, 0, 10, "U: ");
        //canvas_draw_str(canvas, 0, 20, "L: ");
        //canvas_draw_str(canvas, 0, 30, "R: ");
        //canvas_draw_str(canvas, 0, 40, "D: ");
        //canvas_draw_str(canvas, 0, 50, "Ok: ");

        //PNGs are located in assets/icons/UniRFRemix before compiliation

        //Icons for Labels
        canvas_draw_icon(canvas, 0, 0, &I_UniRFRemix_LeftAlignedButtons_9x64);

        //Labels
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 12, 10, app->up_label);
        canvas_draw_str(canvas, 12, 20, app->down_label);
        canvas_draw_str(canvas, 12, 30, app->left_label);
        canvas_draw_str(canvas, 12, 40, app->right_label);
        canvas_draw_str(canvas, 12, 50, app->ok_label);

        canvas_draw_str_aligned(canvas, 12, 62, AlignLeft, AlignBottom, "Repeat # - Hold to exit");

        //Status text and indicator
        canvas_draw_str_aligned(canvas, 125, 10, AlignRight, AlignBottom, app->send_status);

        switch(app->send_status_c) {
        case 0:
            canvas_draw_icon(canvas, 110, 15, &I_UniRFRemix_Outline_14x14);
            break;
        case 1:
            canvas_draw_icon(canvas, 110, 15, &I_UniRFRemix_Left_14x14);
            break;
        case 2:
            canvas_draw_icon(canvas, 110, 15, &I_UniRFRemix_Right_14x14);
            break;
        case 3:
            canvas_draw_icon(canvas, 110, 15, &I_UniRFRemix_Up_14x14);
            break;
        case 4:
            canvas_draw_icon(canvas, 110, 15, &I_UniRFRemix_Down_14x14);
            break;
        case 5:
            canvas_draw_icon(canvas, 110, 15, &I_UniRFRemix_Center_14x14);
            break;
        }

        //Repeat indicator
        //canvas_draw_str_aligned(canvas, 125, 40, AlignRight, AlignBottom, "Repeat:");
        canvas_draw_icon(canvas, 115, 39, &I_UniRFRemix_Repeat_12x14);
        canvas_draw_str_aligned(
            canvas, 125, 62, AlignRight, AlignBottom, int_to_char(app->repeat));
    }

    osMutexRelease(app->model_mutex);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    UniRFRemix* app = ctx;

    osMessageQueuePut(app->input_queue, input_event, 0, osWaitForever);
}

UniRFRemix* unirfremix_alloc() {
    UniRFRemix* app = malloc(sizeof(UniRFRemix));

    app->model_mutex = osMutexNew(NULL);

    app->input_queue = osMessageQueueNew(32, sizeof(InputEvent), NULL);

    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, render_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app);

    // Open GUI and register view_port
    app->gui = furi_record_open("gui");
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    return app;
}

void unirfremix_free(UniRFRemix* app) {
    gui_remove_view_port(app->gui, app->view_port);
    furi_record_close("gui");
    view_port_free(app->view_port);

    osMessageQueueDelete(app->input_queue);

    osMutexDelete(app->model_mutex);

    free(app);
}

int32_t unirfremix_app(void* p) {
    UNUSED(p);
    UniRFRemix* app = unirfremix_alloc();

    //setup variables before population
    string_init(app->up_file);
    string_init(app->down_file);
    string_init(app->left_file);
    string_init(app->right_file);
    string_init(app->ok_file);
    string_init(app->empty);

    string_init(app->up_l);
    string_init(app->down_l);
    string_init(app->left_l);
    string_init(app->right_l);
    string_init(app->ok_l);

    //check map and population variables
    unirfremix_cfg_set_check(app);

    bool exit_loop = false;

    if(app->file_result == 0) {
        FURI_LOG_I(
            TAG,
            "U: %s - D: %s - L: %s - R: %s - O: %s ",
            string_get_cstr(app->up_file),
            string_get_cstr(app->down_file),
            string_get_cstr(app->left_file),
            string_get_cstr(app->right_file),
            string_get_cstr(app->ok_file));

        //variables to control multiple button presses and status updates
        app->send_status = "Idle";
        app->send_status_c = 0;
        app->processing = 0;
        app->repeat = 1;
        app->button = 0;

        //refresh screen to update variables before processing main screen or error screens
        osMutexRelease(app->model_mutex);
        view_port_update(app->view_port);

        //input detect loop start
        InputEvent input;
        while(1) {
            furi_check(osMessageQueueGet(app->input_queue, &input, NULL, osWaitForever) == osOK);
            FURI_LOG_I(
                TAG,
                "key: %s type: %s",
                input_get_key_name(input.key),
                input_get_type_name(input.type));

            switch(input.key) {
            case InputKeyUp:
                if(input.type == InputTypeShort) {
                    if(app->up_enabled) {
                        if(app->processing == 0) {
                            *app->signal = *app->empty;
                            *app->signal = *app->up_file;
                            app->button = 3;
                            app->processing = 1;
                        }
                    }
                }
                break;

            case InputKeyDown:
                if(input.type == InputTypeShort) {
                    if(app->down_enabled) {
                        if(app->processing == 0) {
                            *app->signal = *app->empty;
                            *app->signal = *app->down_file;
                            app->button = 4;
                            app->processing = 1;
                        }
                    }
                }
                break;

            case InputKeyRight:
                if(input.type == InputTypeShort) {
                    if(app->right_enabled) {
                        if(app->processing == 0) {
                            *app->signal = *app->empty;
                            *app->signal = *app->right_file;
                            app->button = 1;
                            app->processing = 1;
                        }
                    }
                }
                break;

            case InputKeyLeft:
                if(input.type == InputTypeShort) {
                    if(app->left_enabled) {
                        if(app->processing == 0) {
                            *app->signal = *app->empty;
                            *app->signal = *app->left_file;
                            app->button = 2;
                            app->processing = 1;
                        }
                    }
                }
                break;

            case InputKeyOk:
                if(input.type == InputTypeShort) {
                    if(app->ok_enabled) {
                        if(app->processing == 0) {
                            *app->signal = *app->empty;
                            *app->signal = *app->ok_file;
                            app->button = 5;
                            app->processing = 1;
                        }
                    }
                }
                break;

            case InputKeyBack:
                if(input.type == InputTypeShort) {
                    if(app->processing == 0) {
                        if(app->repeat < 5) {
                            app->repeat++;
                        } else {
                            app->repeat = 1;
                        }
                    }
                } else if(input.type == InputTypeLong) {
                    exit_loop = true;
                }
                break;
            }

            if(app->processing == 0) {
                FURI_LOG_I(TAG, "processing 0");
                app->send_status = "Idle";
                app->send_status_c = 0;
                app->button = 0;
            } else if(app->processing == 1) {
                FURI_LOG_I(TAG, "processing 1");

                app->send_status = "Sending";

                switch(app->button) {
                case 1:
                    app->send_status_c = 1;
                    break;
                case 2:
                    app->send_status_c = 2;
                    break;
                case 3:
                    app->send_status_c = 3;
                    break;
                case 4:
                    app->send_status_c = 4;
                    break;
                case 5:
                    app->send_status_c = 5;
                    break;
                }

                app->processing = 2;

                unirfremix_process_signal(app, app->signal);
            }

            if(exit_loop == true) {
                osMutexRelease(app->model_mutex);
                break;
            }

            osMutexRelease(app->model_mutex);
            view_port_update(app->view_port);
        }
    } else {
        //refresh screen to update variables before processing main screen or error screens
        view_port_update(app->view_port);

        InputEvent input;
        while(1) {
            furi_check(osMessageQueueGet(app->input_queue, &input, NULL, osWaitForever) == osOK);
            FURI_LOG_I(
                TAG,
                "key: %s type: %s",
                input_get_key_name(input.key),
                input_get_type_name(input.type));

            switch(input.key) {
            case InputKeyRight:
                break;
            case InputKeyLeft:
                break;
            case InputKeyUp:
                break;
            case InputKeyDown:
                break;
            case InputKeyOk:
                break;
            case InputKeyBack:
                if(input.type == InputTypeLong) {
                    exit_loop = true;
                }
                break;
            }

            if(exit_loop == true) {
                osMutexRelease(app->model_mutex);
                break;
            }

            osMutexRelease(app->model_mutex);
            view_port_update(app->view_port);
        }
    }

    // remove & free all stuff created by app
    unirfremix_free(app);

    return 0;
}

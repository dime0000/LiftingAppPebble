/*
 * main.c
 * Constructs a Window housing an output TextLayer to show data from 
 * either modes of operation of the accelerometer.
 */

// PRESENTATION: pre-emptive - talk briefly about pebble, what it offers (cost), what smartwatch does, developer utils
// emulator, pebble cloud
// demo app first

// PRESENTATION: PEBBLE.H - requirement for all things
#include <pebble.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

// keys being passed between phone and watch
//
#define EXERCISE_NAME 0
#define KEY_EXERCISE_VALUE 1

// TODO - replace with array length
// which means array needs to be global
// service *should* be returning 20 at a time


#define EXERCISE_NAME_MAX_LENGTH 100

int LIST_MESSAGE_WINDOW_NUM_ROWS  = 20;

// no idea if i need this. experiment
int LIST_MESSAGE_WINDOW_CELL_HEIGHT = 40;

// no idea about this either
int LIST_MESSAGE_WINDOW_MENU_HEIGHT = 600;


int repCount = 0;
int curlUpX = 0;
int curlDownX = 0;

int sampleToLog = 0;

bool isUp = true;
bool isDown = false;

static char  exercise_names[20][100];
static char selected_exercise[EXERCISE_NAME_MAX_LENGTH];
  

static Window *s_main_window;
static Window *s_exercise_window;
static Window *s_tracking_window;


static MenuLayer *s_exercise_menu_layer;
  
static TextLayer *s_output_layer;
static TextLayer *s_tracking_layer;
static TextLayer *s_reps_layer;

static TextLayer *s_exercise_layer;

// a supposed strtok supported for pebble

// PRESENTATION: C and JS, but not ALL C.. some things are not supported, like strtok

char *
strtok(s, delim)
	register char *s;
	register const char *delim;
{
	register char *spanp;
	register int c, sc;
	char *tok;
	static char *last;


	if (s == NULL && (s = last) == NULL)
		return (NULL);

	/*
	 * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
	 */
cont:
	c = *s++;
	for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
		if (c == sc)
			goto cont;
	}

	if (c == 0) {		/* no non-delimiter characters */
		last = NULL;
		return (NULL);
	}
	tok = s - 1;

	/*
	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (;;) {
		c = *s++;
		spanp = (char *)delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				last = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}

static void send(int key, int value) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_int(iter, key, &value, sizeof(int), true);

  app_message_outbox_send();
}

static void sendString(int key, const char *value) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_cstring(iter, key, value);

  app_message_outbox_send();
}
//PRESENTATION: Sends data to android
static void send_to_droid_click_handler(ClickRecognizerRef recognizer, void *context) {
  
    char new_str[strlen(text_layer_get_text(s_tracking_layer))+ strlen(" ") + 
                         strlen(text_layer_get_text(s_reps_layer))+1];
  
    new_str[0] = '\0';   // ensures the memory is an empty string
    strcat(new_str,text_layer_get_text(s_tracking_layer));
    strcat(new_str," ");
    strcat(new_str,text_layer_get_text(s_reps_layer) );

    sendString(KEY_EXERCISE_VALUE, new_str);
    
}

// PRESENTATION: MenuLayer functions... maybe?
static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {

  return LIST_MESSAGE_WINDOW_NUM_ROWS;
}

// goes through for each of the number of rows. (int)cell_index->row to get the value in the index
// may have to be -1
static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *context) {
  
  static char s_buff[EXERCISE_NAME_MAX_LENGTH];
  
  strcpy(s_buff, exercise_names[(int)cell_index->row]);
  
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, s_buff, fonts_get_system_font(FONT_KEY_GOTHIC_18), layer_get_bounds(cell_layer), 
                     GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL );
  
  
  
 // menu_cell_basic_draw(ctx, cell_layer, s_buff, NULL, NULL);
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  
  return LIST_MESSAGE_WINDOW_CELL_HEIGHT;
}


// gets data from accelerometer
static void data_handler(AccelData *data, uint32_t num_samples) {
  // Long lived buffer
  
  
  static char s_buffer[128];

  // saple to log will make it so that its 1 log every second... 20 would be every 2.. etc.
  if (sampleToLog == 50) {
    // Compose string of all data
    snprintf(s_buffer, sizeof(s_buffer), 
      "N X,Y,Z\n0 %d,%d,%d\n1 %d,%d,%d\n2 %d,%d,%d", 
      data[0].x, data[0].y, data[0].z, 
      data[1].x, data[1].y, data[1].z, 
      data[2].x, data[2].y, data[2].z
    );
    APP_LOG(APP_LOG_LEVEL_INFO, "%s", s_buffer);
    sampleToLog = 0;
  } else {
    sampleToLog ++;
  }
  
  // TODO - need to change the value of curlUpX, curlDownX and/or use ddiferent data depending on exercise
  //if (strstr(s_buff, "http://wger.de/api/v2/exercise/?page=") != NULL) {
  //text_layer_get_text(s_tracking_layer);
    int counter=0;
 
    for (counter = 0; selected_exercise[counter] != '\0'; counter++)
      selected_exercise[counter] = tolower((unsigned char)selected_exercise[counter]);

    

  int dataToScan = data[0].x; 
  curlUpX = -2000;
  curlDownX = 2000;
  
  
  if (strstr(selected_exercise, "biceps curls") != NULL) {
    curlUpX = -850;
    curlDownX = 850;
    dataToScan = data[0].x;
  }
  
  else if (strstr(selected_exercise, "shoulder press") != NULL)  {
    curlUpX = -15;
    curlDownX = 140;
    dataToScan = data[0].z;
  }
  else if (strstr(selected_exercise, "bench press") != NULL || 
            strstr(selected_exercise, "benchpress") != NULL)  {
    curlUpX = 120;
    curlDownX = 150;
    dataToScan = data[0].z;
  }
  else if (strstr(selected_exercise, "squat") != NULL)  {
    curlUpX = -15;
    curlDownX = 140;
    dataToScan = data[0].z;
  }
  
/*
  down	up
< -575	> - 180
// requires reverse
  */

  if (dataToScan < curlUpX) {
    if (isDown) {
      repCount = repCount + 1;
      isUp = true;
      isDown = false;
      snprintf(s_buffer, sizeof(s_buffer), "REPS: %d", repCount);
      // need another layer
      text_layer_set_text(s_reps_layer, s_buffer);
      // could also buzz instead
      vibes_short_pulse();
      
     // APP_LOG(APP_LOG_LEVEL_DEBUG, "%s", s_buffer);

    }
    
  } else if (dataToScan > curlDownX) {
    isDown = true;
    isUp = false;
  }
  
  
  
  // buzz 3 times when complete and stop logging. 
  
  //TODO start with an exercise list / selection - call https://wger.de/en/software/api
  // demos API call using phone
  
  //Show the data
  //text_layer_set_text(s_output_layer, s_buffer);
  //use bits from here https://developer.getpebble.com/tutorials/watchface-tutorial/part3/
}


// PRESENTATION: Loads and unloads.. always have to make sure to clean memory per C...

static void tracking_window_load(Window *window) {
  repCount = 0;  
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  
  // Create output TextLayer
  s_tracking_layer = text_layer_create(GRect(5, 0, 139, 100));
  s_reps_layer = text_layer_create(GRect(5, 115, 139, 50));

  text_layer_set_font(s_tracking_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_font(s_reps_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));

  text_layer_set_text(s_tracking_layer, "No data yet.");
  text_layer_set_text(s_reps_layer, "Reps:");
  
  text_layer_set_overflow_mode(s_tracking_layer, GTextOverflowModeWordWrap);
  text_layer_set_overflow_mode(s_reps_layer, GTextOverflowModeWordWrap);
  
  layer_add_child(window_layer, text_layer_get_layer(s_tracking_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_reps_layer));
  
}

static void tracking_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_tracking_layer);
  text_layer_destroy(s_reps_layer);
  window_destroy(window);
  accel_data_service_unsubscribe();
}


void tracking_config_provider(Window *window) {
  window_single_click_subscribe(BUTTON_ID_SELECT, send_to_droid_click_handler);

}


// PRESENTATION: Callbacks in general...
void select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context)
{
   //cell_index->row use this to get the value in the array exercise_names and post the name to a new window..
  // just like the other select click.
  //Get which row
  // draw row maybe?

  strcpy(selected_exercise, exercise_names[(int)cell_index->row]);
  
  if (strstr(selected_exercise, "http://wger.de/api/v2/exercise/") != NULL) {
    LIST_MESSAGE_WINDOW_NUM_ROWS  = 20; // need to reset to max
    window_stack_pop(false);
    sendString(0,selected_exercise); // 2 is the page number.. need to figure this out.   
  }
  else {
  
    s_tracking_window = window_create();
    
    window_set_click_config_provider(s_tracking_window, (ClickConfigProvider) tracking_config_provider);
    
    
    window_set_window_handlers(s_tracking_window, (WindowHandlers) {
      .load = tracking_window_load,
      .unload = tracking_window_unload
    }); 
  
  // PRESENTATION: Push to navigation stack... back button automatically pulls from stack

    window_stack_push(s_tracking_window, false);
  
    char new_str2[strlen("Selected exercise: ")+strlen(selected_exercise)+1] ;
    
    new_str2[0] = '\0';   // ensures the memory is an empty string
    strcat(new_str2,"Selected exercise: ");
      
    strcat(new_str2,selected_exercise);
    
    text_layer_set_text(s_tracking_layer,  new_str2);
  
    int num_samples = 3;
    
  // PRESENTATION: Accelerometer setup.. pretty simple to use. Data_handler method handles accelerometer data 
  // as it comes in
    accel_data_service_subscribe(num_samples, data_handler);
  
    // Choose update rate
    accel_service_set_sampling_rate(ACCEL_SAMPLING_25HZ);
    
    // now convert to lower case

    //free(new_str2);
    
  }
}

void select_changed_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context)
{

   //menu_layer_reload_data(menu_layer);
}

static void exercise_window_load(Window *window) {
  
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
    
  s_exercise_menu_layer = menu_layer_create(window_bounds);
  
  menu_layer_set_click_config_onto_window(s_exercise_menu_layer, window);
  
  // PRESENTATION: sets up callbacks.. typical format for something like menu layer, windows..
  menu_layer_set_callbacks(s_exercise_menu_layer, NULL, (MenuLayerCallbacks) {
      .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback)get_num_rows_callback,
      .draw_row = (MenuLayerDrawRowCallback)draw_row_callback,
		  .get_cell_height =  get_cell_height_callback,
      .selection_changed = (MenuLayerSelectionChangedCallback )select_changed_callback,
      .select_click = (MenuLayerSelectCallback) select_click_callback,
      
  });
  
  layer_add_child(window_layer, menu_layer_get_layer(s_exercise_menu_layer));

  
}

static void exercise_window_unload(Window *window) {
  // Destroy output TextLayer
  
  menu_layer_destroy(s_exercise_menu_layer);
  window_destroy(window);
}

// call backs to get data from exercise API
// PRESENTATION: callbacks from phone is done here.. 
// actually calling the web using the phone.. returns a dictionary of data for us
// JS file handles external calls to phone
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // here
  // may have to reset the array every time.. 
  s_exercise_window = window_create();
  
  window_set_window_handlers(s_exercise_window, (WindowHandlers) {
    .load = exercise_window_load,
    .unload = exercise_window_unload
  }); 
  
  memset(exercise_names, 0, sizeof exercise_names);
  
  // PRESENTATION: C strings.. always a pain
  static char exercise_layer_buffer[1000];
  
  // Read tuples for data
  // PRESENTATION: Dictionary is actually a series of strings. Have to register "app keys" that both C file and JS recognize  
  // in this case, returning a delimited string that i ened to parse
  Tuple *name_tuple = dict_find(iterator, EXERCISE_NAME);

  // If all data is available, use it
  if(name_tuple) {
    snprintf(exercise_layer_buffer, sizeof(exercise_layer_buffer), "%s", name_tuple->value->cstring);
  }
  
  
  window_stack_push(s_exercise_window, false);
  
  //// PRESENTATION: Parsing and putting results in to an array for later

  static char new_exercise_layer[1000];
  
  snprintf(new_exercise_layer, sizeof(new_exercise_layer), "%s", exercise_layer_buffer);
  
  char * pch;
  int i = 0;
  pch = strtok (new_exercise_layer,"|");
  while (pch != NULL)
  {
    
    strcpy(exercise_names[i], pch);
    pch = strtok (NULL, "|");

    i++;
  }
  
  // dummy something up for now for page 2..
  /*
  strcpy(exercise_names[i], "NEXT -->");
  i++;
  */
  // can i redefine the length based on how many returned here?
  LIST_MESSAGE_WINDOW_NUM_ROWS = i;
  
  free(pch);
  
}



static void inbox_dropped_callback(AppMessageResult reason, void *context) {
   APP_LOG(APP_LOG_LEVEL_DEBUG, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
   APP_LOG(APP_LOG_LEVEL_DEBUG, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
   APP_LOG(APP_LOG_LEVEL_DEBUG, "Outbox send success!");
}

static void main_window_load(Window *window) {
  
//  APP_LOG(APP_LOG_LEVEL_DEBUG, "IN MAIN WINDOW LOAD");
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create output TextLayer
  s_output_layer = text_layer_create(GRect(5, 0, window_bounds.size.w - 10, window_bounds.size.h));
  text_layer_set_font(s_output_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(s_output_layer, "Hit select to load exercise options");
  text_layer_set_overflow_mode(s_output_layer, GTextOverflowModeWordWrap);

  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));
  
  
}

static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_output_layer);
  window_destroy(window);
}

// PRESENTATION: this handler sends a message to js file for handling
void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  // this is where we'd send 0,1 - 0,2 etc with the 2nd number being the page number..
  sendString(0,"http://wger.de/api/v2/exercise/?page=1");
  
}


void config_provider(Window *window) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);

}


// PRESENTATION: init sets app level callbacks and windows
static void init() {
  // Register callbacks
  
  
// PRESENTATION: callbacks to send/recieve messages to/from phone
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  s_main_window = window_create();
  
  window_set_click_config_provider(s_main_window, (ClickConfigProvider) config_provider);
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(s_main_window, true);

}

static void deinit() {
  //window_destroy(s_main_window);
}


// PRESENTATION: main method
int main(void) {
  init();
  app_event_loop();
  deinit();
}

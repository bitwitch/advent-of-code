#include <math.h>
#include "../common.c"

#define PIXELS_PER_METER 10
#define PI 3.141592653589793

// both in radians
#define GAUGE_OFFSET_ANGLE 2.67
#define GAUGE_RANGE        4.1

typedef struct {
	oc_vec2 start;
	oc_vec2 pos;
	f32 w, h;
	oc_image image;
	f32 speed;
	char dist_text[8]; // 000.00 m
} Boat;

typedef struct {
	f32 x, y, w, h;
	oc_color color;
} Box;

typedef struct {
	Box panel;
	Box text_box;
	f64 duration;
	f64 time;
	f32 font_size;
	char time_string[8]; // 9999.99
} Timer;

typedef enum {
	STATE_NONE,
	STATE_WAITING,
	STATE_HOLDING,
	STATE_RACING,
	STATE_DONE,
} State;

typedef struct {
	bool down, was_down, pressed, released;
} DigitalInput;

typedef struct {
	oc_rect rect;
	oc_image image;
	oc_vec2 text_pos;
	oc_color text_color;
	f32 font_size;
	oc_str8 text;
	DigitalInput state;
} DashButton; 

typedef struct {
	oc_rect rect;
	oc_image image;
	f32 gauge_len;
	f32 angle_radians;
} Speedometer;

typedef struct {
	f32 radius;
	oc_vec2 center;
	oc_color color, down_color;
	DigitalInput state;
} SpeedButton;

typedef struct {
    float x;
    float y;
    float deltaX;
    float deltaY;
	DigitalInput left, right;
} MouseInput;

typedef struct {
	DigitalInput r;
} Input;


static oc_surface surface;
static oc_canvas canvas;
static oc_font font;
static oc_vec2 view_size = {1280, 720};
static f64 dt, last_timestamp;
static State state;
static MouseInput mouse_input;
static Input input;

static oc_color color_silver = {203/255.0f, 219/255.0f, 252/255.0f, 1};

static oc_color bg_color;
static Timer timer;
static Box dashboard;
static SpeedButton button;
static Boat boat;
static DashButton reset_button;
static Speedometer speedometer;

static f32 vec2_dist(oc_vec2 v1, oc_vec2 v2) {
	f32 a = v2.x - v1.x;
	f32 b = v2.y - v1.y;
	return sqrtf(a*a + b*b);
}

static bool point_in_rect(oc_vec2 p, oc_rect rect) {
	return p.x >= rect.x && p.x < rect.x + rect.w &&
	       p.y >= rect.y && p.y < rect.y + rect.h;
}

static void update_timer_string(f64 seconds) {
	snprintf(timer.time_string, sizeof(timer.time_string), "%7.2f", seconds);
}

static void reset(void) {
    last_timestamp = oc_clock_time(OC_CLOCK_MONOTONIC);

	timer.time = timer.duration;
	update_timer_string(timer.time);

	boat.speed = 0;
	boat.pos = boat.start;

	state = STATE_WAITING;

	memset(&button.state, 0, sizeof(button.state));
	memset(&reset_button.state, 0, sizeof(reset_button.state));
	memset(&mouse_input.left, 0, sizeof(mouse_input.left));
	memset(&mouse_input.right, 0, sizeof(mouse_input.right));
	memset(&input, 0, sizeof(input));
}


static void update_waiting(void) {
	if (button.state.pressed) {
		state = STATE_HOLDING;
	}
}

static void update_holding(void) {
	timer.time -= dt;

	if (timer.time <= 0) {
		boat.speed += timer.time; // correct for going below 0 time
		timer.time = 0;
		update_timer_string(timer.time);
		state = STATE_DONE;
		return;
	}

	boat.speed += dt;

	update_timer_string(timer.time);

	if (!button.state.down) {
		state = STATE_RACING;
	}
}

static void update_racing(void) {

	timer.time -= dt;

	if (timer.time <= 0) {
		timer.time = 0;
		update_timer_string(timer.time);
		state = STATE_DONE;
		return;
	}

	update_timer_string(timer.time);

	f32 speed_factor = 0.5;
	boat.pos.x += boat.speed * speed_factor;

	f32 dist = (boat.pos.x - boat.start.x) / PIXELS_PER_METER;
	snprintf(boat.dist_text, sizeof(boat.dist_text), "%.2f m", dist);
}

static void update_done(void) {
}

static void update_speedometer(void) {
	f32 max_speed = timer.duration;
	f32 normalized = (max_speed - boat.speed) / max_speed;
	f32 angle = (1.0f - normalized) * GAUGE_RANGE;
	speedometer.angle_radians = angle + GAUGE_OFFSET_ANGLE;
}

static void digital_input_begin_frame(DigitalInput *di) {
	di->pressed = di->down && !di->was_down;
	di->released = !di->down && di->was_down;
}

static void digital_input_end_frame(DigitalInput *di) {
	di->was_down = di->down;
}

static void input_begin_frame(void) {
	digital_input_begin_frame(&mouse_input.left);
	digital_input_begin_frame(&mouse_input.right);
	digital_input_begin_frame(&input.r);

	// dashboard buttons
	if (mouse_input.left.pressed) {
		oc_vec2 mouse_pos = {mouse_input.x, mouse_input.y};
		if (vec2_dist(button.center, mouse_pos) < button.radius) {
			button.state.pressed = true;
			button.state.down = true;
		}
		if (point_in_rect(mouse_pos, reset_button.rect)) {
			reset_button.state.pressed = true;
			reset_button.state.down = true;
		}
	} else if (mouse_input.left.released) {
		button.state.down = false;
		button.state.released = button.state.was_down;
		reset_button.state.down = false;
		reset_button.state.released = reset_button.state.was_down;
	}
}

static void input_end_frame(void) {
	digital_input_end_frame(&mouse_input.left);
	digital_input_end_frame(&mouse_input.right);
	digital_input_end_frame(&input.r);
	digital_input_end_frame(&button.state);
	digital_input_end_frame(&reset_button.state);
}

static void draw_button(void) {
	if (button.state.down) {
		oc_set_color(button.down_color);
	} else {
		oc_set_color(button.color);
	}
	oc_circle_fill(button.center.x, button.center.y, button.radius);

	f32 border_width = 3;
	oc_set_color(button.down_color);
	oc_set_width(border_width);
	oc_circle_stroke(button.center.x, button.center.y, button.radius);

	oc_set_color(color_silver);
	border_width = 6;
	oc_set_width(border_width);
	oc_circle_stroke(button.center.x, button.center.y, button.radius + 0.5f*border_width);
}

static void draw_dashboard(void) {
	// draw dashboard panel
	oc_set_color(dashboard.color);
	oc_rectangle_fill(dashboard.x, dashboard.y, dashboard.w, dashboard.h);
	f32 trim_height = 7;
	oc_set_color_rgba(61/255.0f, 29/255.0f, 24/255.0f, 1);
	oc_rectangle_fill(dashboard.x, dashboard.y-trim_height, dashboard.w, trim_height);

	// reset button
	oc_image_draw(reset_button.image, reset_button.rect);
	oc_set_color(reset_button.text_color);
	oc_set_font_size(reset_button.font_size);
	oc_text_fill(reset_button.text_pos.x, reset_button.text_pos.y, reset_button.text);

	// speedometer
	f32 gauge_width = 4;
	oc_set_width(gauge_width);
	oc_image_draw(speedometer.image, speedometer.rect);
	f32 sx = speedometer.rect.x + 0.5f * speedometer.rect.w;
	f32 sy = speedometer.rect.y + 0.5f * (speedometer.rect.h + gauge_width); 
	f32 gauge_x = sx + speedometer.gauge_len * cosf(speedometer.angle_radians);
	f32 gauge_y = sy + speedometer.gauge_len * sinf(speedometer.angle_radians);
	oc_set_color_rgba(1,0,0,1);
	oc_move_to(sx, sy);
	oc_line_to(gauge_x, gauge_y);
	oc_close_path();
	oc_stroke();

	// speed button
	draw_button();

	// timer
	oc_set_color(timer.panel.color);
	oc_rectangle_fill(timer.panel.x, timer.panel.y, timer.panel.w, timer.panel.h);
	oc_set_color(color_silver);
	f32 border_width = 4;
	oc_set_width(border_width);
	oc_rectangle_stroke(
			timer.panel.x - 0.5f*border_width, 
			timer.panel.y - 0.5f*border_width, 
			timer.panel.w + border_width, 
			timer.panel.h + border_width);

	oc_set_color(timer.text_box.color);
	oc_set_font_size(timer.font_size);
	oc_text_fill(timer.text_box.x, timer.text_box.y, OC_STR8(timer.time_string));

}

static void draw(void) {
	oc_canvas_select(canvas);
	oc_surface_select(surface);
	oc_set_color(bg_color);
	oc_clear();

	draw_dashboard();

	// draw boat
	oc_rect boat_rect = { .x=boat.pos.x, .y=boat.pos.y, .w=boat.w, .h=boat.h };
	oc_image_draw(boat.image, boat_rect);

	// draw boat distance
	if (state == STATE_RACING || state == STATE_DONE) {
		oc_set_color_rgba(0,0,0,1);
		oc_set_font_size(16);
		oc_text_fill(
			boat.pos.x + boat.w + 5, 
			boat.pos.y + 0.5 * boat.h, 
			OC_STR8(boat.dist_text));
	}

    oc_render(canvas);
    oc_surface_present(surface);
}

ORCA_EXPORT void oc_on_resize(u32 width, u32 height) {
	view_size.x = width;
	view_size.y = height;
}

ORCA_EXPORT void oc_on_key_down(oc_scan_code scan, oc_key_code key) {
	switch (key) {
	case OC_KEY_R: input.r.down = true; break;
	default: break;
	}
}

ORCA_EXPORT void oc_on_key_up(oc_scan_code scan, oc_key_code key) {
	switch (key) {
	case OC_KEY_R: input.r.down = false; break;
	default: break;
	}
}

ORCA_EXPORT void oc_on_mouse_down(int button) {
	if (button == OC_MOUSE_LEFT) {
		mouse_input.left.down = true;
	} else if (button == OC_MOUSE_RIGHT) {
		mouse_input.right.down = true;
	}
}

ORCA_EXPORT void oc_on_mouse_up(int button) {
	if (button == OC_MOUSE_LEFT) {
		mouse_input.left.down = false;
	} else if (button == OC_MOUSE_RIGHT) {
		mouse_input.right.down = false;
	}
}

ORCA_EXPORT void oc_on_mouse_move(float x, float y, float dx, float dy) {
	mouse_input.x = x;
	mouse_input.y = y;
	mouse_input.deltaX = dx;
	mouse_input.deltaY = dy;
}

ORCA_EXPORT void oc_on_init(void) {
    oc_window_set_title(OC_STR8("Boat Race"));
	oc_window_set_size(view_size);

    surface = oc_surface_canvas();
    canvas = oc_canvas_create();

    oc_unicode_range ranges[5] = {
        OC_UNICODE_BASIC_LATIN,
        OC_UNICODE_C1_CONTROLS_AND_LATIN_1_SUPPLEMENT,
        OC_UNICODE_LATIN_EXTENDED_A,
        OC_UNICODE_LATIN_EXTENDED_B,
        OC_UNICODE_SPECIALS,
    };
	font = oc_font_create_from_path(OC_STR8("segoeui.ttf"), 5, ranges);
	oc_set_font(font);

	bg_color = (oc_color){0.19, .38, .51, 1};

	dashboard.h = 0.3f * view_size.y;
	dashboard.w = view_size.x;
	dashboard.x = 0;
	dashboard.y = view_size.y - dashboard.h;
	dashboard.color = (oc_color){130/255.0f, 75/255.0f, 51/255.0f, 1};

	button.radius = 0.25f * dashboard.h;
	button.color = (oc_color){0, 207/255.0f, 72/255.0f, 1};
	button.down_color = (oc_color){
		.r=button.color.r*0.9f, .g=button.color.g*0.9f, .b=button.color.b*0.9f, 1};
	button.center.x = 0.5f * dashboard.w;
	button.center.y = dashboard.y + 0.5f * dashboard.h;

	reset_button.image = oc_image_create_from_path(surface, OC_STR8("control_button.png"), false);
	oc_vec2 control_button_size = oc_image_size(reset_button.image);
	reset_button.rect.w = control_button_size.x;
	reset_button.rect.h = control_button_size.y;
	reset_button.rect.x = dashboard.x + 25;
	reset_button.rect.y = dashboard.y + dashboard.h - reset_button.rect.h - 25;
	reset_button.text_color = (oc_color){0,0,0,1};
	reset_button.text = OC_STR8("RESET");
	reset_button.font_size = reset_button.rect.h * 0.5f;
	oc_text_metrics metrics = oc_font_text_metrics(font, reset_button.font_size, reset_button.text);
	reset_button.text_pos.x = reset_button.rect.x - 1 + 0.5 * (reset_button.rect.w - metrics.ink.w);
	reset_button.text_pos.y = reset_button.rect.y + 0.5 * (reset_button.rect.h + metrics.ink.h);

	timer.panel.h = 0.43f * dashboard.h;
	timer.panel.w = 2.17f * timer.panel.h;
	timer.panel.x = dashboard.x + (0.75f * dashboard.w) - (0.5f * timer.panel.w);
	timer.panel.y = dashboard.y + (0.5f * dashboard.h) - (0.5f * timer.panel.h);
	timer.panel.color = (oc_color){0,0,0,1};

	timer.duration = 7.0;
	timer.time = timer.duration;
	update_timer_string(timer.time);
	oc_str8 timer_text = OC_STR8(timer.time_string);
	f32 pad = 15;
	timer.font_size = timer.panel.h - (2.0f * pad);
	metrics = oc_font_text_metrics(font, timer.font_size, timer_text);
	timer.text_box.color = (oc_color){1,1,1,1};
	timer.text_box.x = timer.panel.x + timer.panel.w - metrics.ink.w - pad;
	timer.text_box.y = timer.panel.y + 0.5f * (timer.panel.h + metrics.ink.h);

	speedometer.image = oc_image_create_from_path(surface, OC_STR8("speedometer.png"), false);
	oc_vec2 speedometer_size = oc_image_size(speedometer.image);
	speedometer.rect.x = dashboard.x + (0.25f * dashboard.w) - (0.5f * speedometer_size.x);
	speedometer.rect.y = dashboard.y + (0.5f * dashboard.h) - (0.5f * speedometer_size.y);
	speedometer.rect.w = speedometer_size.x;
	speedometer.rect.h = speedometer_size.y;
	speedometer.gauge_len = 0.39f * speedometer.rect.w;
	// speedometer.angle_radians = PI / 4;
	speedometer.angle_radians = 0;

	boat.image = oc_image_create_from_path(surface, OC_STR8("boat.png"), false);
	oc_vec2 boat_size = oc_image_size(boat.image);
	boat.start.x = 25;
	boat.start.y = 0.1f * view_size.y;
	boat.w = boat_size.x;
	boat.h = boat_size.y;
	boat.pos = boat.start;

	reset();
}


ORCA_EXPORT void oc_on_frame_refresh(void) {
    f64 timestamp = oc_clock_time(OC_CLOCK_MONOTONIC);
	dt = timestamp - last_timestamp;
	last_timestamp = timestamp;

	input_begin_frame();

	if (input.r.pressed || reset_button.state.pressed) {
		reset();
	}
	
	switch(state) {
		case STATE_WAITING: update_waiting(); break;
		case STATE_HOLDING: update_holding(); break;
		case STATE_RACING:  update_racing();  break;
		case STATE_DONE:    update_done();    break;
		default:            assert(0);        break;
	}

	// bob boat
	boat.pos.y = boat.start.y + 2.0f*sinf(3.0f*timestamp);

	update_speedometer();

	draw();

	input_end_frame();
}



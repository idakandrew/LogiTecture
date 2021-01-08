#include <stdio.h>
#include <math.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_ttf.h>
#include "canvas.h"
#include "utils.h"
#include "sim.h"
#include "ui.h"

int main(void) {
    fix_dir();
    al_init();
    al_init_primitives_addon();
    al_init_image_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    al_install_keyboard();
    al_install_mouse();

    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0);
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
    ALLEGRO_DISPLAY *disp = al_create_display(1920, 1080);

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    ALLEGRO_EVENT event;
    ALLEGRO_MOUSE_STATE state;
    ALLEGRO_KEYBOARD_STATE kstate;

    int scene = 1;
    bool done = false;
    bool redraw = true;

    al_start_timer(timer);

    start:

    if(scene == 1) {
        int mcbtn0 = 0, mcbtn1 = 0, mcbtn2 = 0;

        ALLEGRO_FONT *font = al_load_ttf_font("mont.otf", 32, 0);
        button mbtn0 = btn_build(480, 500, "Load Project", "new.png");
        button mbtn1 = btn_build(480, 650, "Controls", "new.png");
        button mbtn2 = btn_build(480, 800, "Exit", "new.png");
        ALLEGRO_BITMAP *logo = al_load_bitmap("logo.png");

        while(1) {
            al_wait_for_event(queue, &event);

            switch(event.type) {
                case ALLEGRO_EVENT_TIMER:
                    redraw = true;
                    break;
                case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                    btn_click(mbtn0, event.mouse, &mcbtn0);
                    btn_click(mbtn1, event.mouse, &mcbtn1);
                    btn_click(mbtn2, event.mouse, &mcbtn2);
                    break;
                case ALLEGRO_EVENT_DISPLAY_CLOSE:
                    done = true;
                    break;
            }

            if(mcbtn0 == 1) {
                scene = 2;
                break;
            } else if(done || mcbtn2 == 1) {
                scene = 0;
                break;
            }

            if(redraw && al_is_event_queue_empty(queue)) {
                al_clear_to_color(al_map_rgb(45, 45, 45));
                al_draw_filled_rectangle(130, 0, 830, 1080, al_map_rgb(15, 15, 15));
                al_draw_bitmap(logo, 480 - al_get_bitmap_width(logo) / 2, 150, 0);
                al_draw_text(font, al_map_rgb(200, 200, 200), 480, 1000, ALLEGRO_ALIGN_CENTER, "Version 0.1");
                btn_draw(mbtn0, font, &mcbtn0);
                btn_draw(mbtn1, font, &mcbtn1);
                btn_draw(mbtn2, font, &mcbtn2);
                al_flip_display();
                redraw = false;
            }
        }

        al_destroy_bitmap(logo);
        al_destroy_bitmap(mbtn0.bit);
        al_destroy_bitmap(mbtn1.bit);
        al_destroy_bitmap(mbtn2.bit);
        al_destroy_font(font);
        al_flush_event_queue(queue);

        goto start;

    } else if (scene == 2) {
        bool grid = false;
        int click = 0, wait = 0, lock = 0, x = 0, y = 0, lx = 0, ly = 0, dirx = 0, diry = 0;
        int cbtna = 0, cbtnn = 0, cbtnf = 0, cbtnl = 0, cbtnr = 0, cbtnx = 0, select = 0;
        int map[96][50];
        memset(map, 0, sizeof(map));

        ALLEGRO_FONT *font = al_load_ttf_font("mont.otf", 26, 0);
        button abtn = btn_build(660, 1040, "AND", "select.png");
        button nbtn = btn_build(810, 1040, "NOT", "select.png");
        button fbtn = btn_build(960, 1040, "Switch", "select.png");
        button lbtn = btn_build(1110, 1040, "Light", "select.png");
        button rbtn = btn_build(1260, 1040, "Crossing", "select.png");
        button xbtn = btn_build(1680, 1040, "Exit", "select.png");

        while(1) {
            al_wait_for_event(queue, &event);
            al_get_mouse_state(&state);
            al_get_keyboard_state(&kstate);

            switch (event.type) {
                case ALLEGRO_EVENT_TIMER:
                    wire_sim(map, 0, 0, 0);
                    chip_sim(map);
                    redraw = true;
                    break;
                case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                    if(m_range(state, 0, 1920, 0, 1000)) {
                        if(lock == 1) {lock_coords(&lock, &lx, &ly, state);}
                        click = 1;
                    } else if(btn_click(abtn, event.mouse, &cbtna)) {
                        select = (select == 1) ? 0 : 1;
                    } else if(btn_click(nbtn, event.mouse, &cbtnn)) {
                        select = (select == 2) ? 0 : 2;
                    } else if(btn_click(fbtn, event.mouse, &cbtnf)) {
                        select = (select == 3) ? 0 : 3;
                    } else if(btn_click(lbtn, event.mouse, &cbtnl)) {
                        select = (select == 4) ? 0 : 4;
                    } else if(btn_click(rbtn, event.mouse, &cbtnr)) {
                        select = (select == 5) ? 0 : 5;
                    }
                    btn_click(xbtn, event.mouse, &cbtnx);
                    break;
                case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
                    click = 0;
                    break;
                case ALLEGRO_EVENT_KEY_DOWN:
                    if(event.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                        done = true;
                    } else if(event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                        select = 0;
                    } else if(event.keyboard.keycode == ALLEGRO_KEY_LSHIFT) {
                        lock_coords(&lock, &lx, &ly, state);
                    } else if(event.keyboard.keycode == ALLEGRO_KEY_TAB) {
                        grid = !grid;
                    } else if(event.keyboard.keycode == ALLEGRO_KEY_BACKSPACE) {
                        memset(map, empty, sizeof(map));
                    }
                    break;
                case ALLEGRO_EVENT_KEY_UP:
                    if(event.keyboard.keycode == ALLEGRO_KEY_LSHIFT) {
                        lock = 0;
                        dirx = 0;
                        diry = 0;
                    }
                    break;
                case ALLEGRO_EVENT_DISPLAY_CLOSE:
                    done = true;
                    break;
            }

            if(done || cbtnx == 1) {
                scene = 1;
                break;
            }

            if(click) {
                x = r_lim(0, floor(state.x / 20), 95);
                y = r_lim(0, floor(state.y / 20), 49);
                
                lock_handler(&lock, lx, ly, &x, &y, &dirx, &diry);

                wait = r_lim(0, wait, 10);

                click_handler(map, state, x, y, select, &wait);
            }

            if(redraw && al_is_event_queue_empty(queue)) {
                al_clear_to_color(al_map_rgb(45, 45, 45));
            
                draw_map(false, grid, map, font);

                btn_draw(abtn, font, &cbtna);
                btn_draw(nbtn, font, &cbtnn);
                btn_draw(fbtn, font, &cbtnf);
                btn_draw(lbtn, font, &cbtnl);
                btn_draw(rbtn, font, &cbtnr);
                btn_draw(xbtn, font, &cbtnx);
                
                text_select(select, font);

                al_draw_filled_rectangle(475, 1000, 485, 1080, al_map_rgb(170, 30, 30));
                al_draw_filled_rectangle(1435, 1000, 1445, 1080, al_map_rgb(170, 30, 30));

                al_flip_display();
                redraw = false;
            }

            wait--;
        }

        al_destroy_font(font);
        al_destroy_bitmap(abtn.bit);
        al_destroy_bitmap(nbtn.bit);
        al_destroy_bitmap(fbtn.bit);
        al_destroy_bitmap(lbtn.bit);
        al_destroy_bitmap(rbtn.bit);
        al_destroy_bitmap(xbtn.bit);
        al_flush_event_queue(queue);

        goto start;
    }
}
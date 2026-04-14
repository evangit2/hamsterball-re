/* ui.c - UI/menu system matching original App_ShowMainMenu (0x46B630) */
#include "ui/ui.h"
#include <stdio.h>
#include <string.h>

static ui_title_state_t g_title_state = UI_TITLE_LOGO;
static int g_selection = 0;
static bool g_active = true;
static float g_logo_timer = 0;
static texture_t *g_title_texture = NULL;
static texture_t *g_miniclip_logo = NULL;
static texture_t *g_raptisoft_logo = NULL;

void ui_init(void) {
    g_title_state = UI_TITLE_LOGO;
    g_selection = 0;
    g_active = true;
    g_logo_timer = 0;
}

void ui_update(float dt) {
    if (!g_active) return;
    
    g_logo_timer += dt;
    
    /* Logo splash: 2 seconds per logo, then title screen */
    if (g_title_state == UI_TITLE_LOGO) {
        if (g_logo_timer > 4.0f) { /* 2s Miniclip + 2s Raptisoft */
            g_title_state = UI_TITLE_MAIN;
            g_logo_timer = 0;
        }
    }
}

void ui_render_title(int screen_w, int screen_h) {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screen_w, screen_h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    if (g_title_state == UI_TITLE_LOGO) {
        /* Dark background */
        float alpha = 1.0f;
        if (g_logo_timer < 0.5f) alpha = g_logo_timer * 2.0f;
        if (g_logo_timer > 1.5f && g_logo_timer < 2.0f) alpha = (2.0f - g_logo_timer) * 2.0f;
        if (g_logo_timer > 3.5f) alpha = (4.0f - g_logo_timer) * 2.0f;
        
        glColor4f(0, 0, 0, alpha);
        glBegin(GL_QUADS);
        glVertex2f(0, 0); glVertex2f(screen_w, 0);
        glVertex2f(screen_w, screen_h); glVertex2f(0, screen_h);
        glEnd();
        
        /* Logo text */
        if (g_logo_timer < 2.0f) {
            glColor4f(1, 1, 1, alpha);
            /* Show "Miniclip" text placeholder */
            const char *text = "MINICLIP";
            int cx = screen_w / 2 - 40;
            int cy = screen_h / 2;
            glRasterPos2f(cx, cy);
        } else {
            glColor4f(1, 1, 1, alpha);
            const char *text = "RAPTISOFT";
            int cx = screen_w / 2 - 50;
            int cy = screen_h / 2;
            glRasterPos2f(cx, cy);
        }
    } else if (g_title_state == UI_TITLE_MAIN) {
        /* Purple/blue gradient background (matching original title screen) */
        glBegin(GL_QUADS);
        glColor4f(0.1f, 0.0f, 0.3f, 1.0f); glVertex2f(0, 0);
        glColor4f(0.2f, 0.1f, 0.5f, 1.0f); glVertex2f(screen_w, 0);
        glColor4f(0.1f, 0.0f, 0.3f, 1.0f); glVertex2f(screen_w, screen_h);
        glColor4f(0.05f, 0.0f, 0.2f, 1.0f); glVertex2f(0, screen_h);
        glEnd();
        
        /* Title text */
        glColor4f(1, 1, 1, 1.0f);
        glRasterPos2f(screen_w / 2 - 60, screen_h / 4);
        
        /* Menu options */
        const char *items[] = {"RACE", "TOURNAMENT", "OPTIONS", "QUIT"};
        int item_count = 4;
        for (int i = 0; i < item_count; i++) {
            int y = screen_h / 2 + i * 40;
            if (i == g_selection) {
                glColor4f(1.0f, 0.85f, 0.0f, 1.0f); /* Yellow selected */
                /* Draw selection box */
                glBegin(GL_QUADS);
                glColor4f(1.0f, 0.85f, 0.0f, 0.3f);
                glVertex2f(screen_w/2 - 80, y - 10);
                glVertex2f(screen_w/2 + 80, y - 10);
                glVertex2f(screen_w/2 + 80, y + 20);
                glVertex2f(screen_w/2 - 80, y + 20);
                glEnd();
                glColor4f(1.0f, 0.85f, 0.0f, 1.0f);
            } else {
                glColor4f(0.7f, 0.7f, 0.7f, 1.0f); /* Gray unselected */
            }
            glRasterPos2f(screen_w / 2 - 30, y);
        }
        
        /* Version info */
        glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
        glRasterPos2f(10, screen_h - 20);
    }
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_FOG);
}

void ui_render_hud(int screen_w, int screen_h, float speed, int fps, int obj_count) {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screen_w, screen_h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    /* Speed bar background */
    glColor4f(0, 0, 0, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(8, 8); glVertex2f(218, 8);
    glVertex2f(218, 28); glVertex2f(8, 28);
    glEnd();
    
    /* Speed bar fill */
    float bar_w = speed * 0.5f;
    if (bar_w > 200.0f) bar_w = 200.0f;
    float speed_pct = speed / 5000.0f; /* vs MAX_VELOCITY */
    if (speed_pct < 0.5f) {
        glColor4f(0.2f + speed_pct, 0.8f, 0.2f, 0.8f);
    } else {
        glColor4f(0.8f, 0.8f - (speed_pct - 0.5f) * 1.6f, 0.2f, 0.8f);
    }
    glBegin(GL_QUADS);
    glVertex2f(10, 10); glVertex2f(10 + bar_w, 10);
    glVertex2f(10 + bar_w, 26); glVertex2f(10, 26);
    glEnd();
    
    /* Status text */
    glColor4f(1, 1, 1, 0.7f);
    char status[256];
    snprintf(status, sizeof(status), "FPS: %d  Obj: %d  Spd: %.0f", fps, obj_count, speed);
    glRasterPos2f(screen_w - 250, 20);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_FOG);
}

int ui_get_selection(void) { return g_selection; }
bool ui_is_active(void) { return g_active; }

void ui_shutdown(void) {
    if (g_title_texture) texture_release(g_title_texture);
    if (g_miniclip_logo) texture_release(g_miniclip_logo);
    if (g_raptisoft_logo) texture_release(g_raptisoft_logo);
}
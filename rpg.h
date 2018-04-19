/* rpg.h : */
#ifndef RPG_H
#define RPG_H

#ifdef NDEBUG
#  define DBG_LOG(...) /* disabled */
#else
#  include <stdio.h>
#  define DBG_LOG(f, ...) fprintf(stderr, f "\n", ## __VA_ARGS__)
// #define DBG_LOG(...) SDL_Log(__VA_ARGS__)
#endif

#define RPG_WINDOW_TITLE "RPG: The Adventure"
#define RPG_OUT_WIDTH 800
#define RPG_OUT_HEIGHT 600

typedef void engine_audio_callback_t(void *extra, uint8_t *stream, int len);

void engine_fini(void);
int engine_init(void);
int engine_loop(void);
int engine_texture_loadfile(const char *filename);
int engine_audio_start(engine_audio_callback_t *cb, void *extra);
void engine_audio_pause(int pause);
void engine_audio_stop(void);

void rpg_update(double elapsed);
int rpg_paint(void);
void rpg_fini(void);
int rpg_init(void);

#endif
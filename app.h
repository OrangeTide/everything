/* app - wraps up and handls applications. public domain */
#ifndef APP_H
#define APP_H
extern void app_initialize(int width, int height);
extern void app_terminate(void);
extern void app_paint(struct nk_context *ctx);
#endif

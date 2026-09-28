// Minimal DragonRuby 7.16 macOS demo; compile as C, not Objective-C.
#include <dragonruby.h>
#include <CoreGraphics/CoreGraphics.h>
#include <dispatch/dispatch.h>
#include <objc/message.h>
#include <objc/runtime.h>
#include <pthread.h>
#include <stdbool.h>

static drb_api_t *drb;
static id saved_window;
static CGRect saved_frame;
static unsigned long saved_style;
static bool configured, stopped;

static id object(id receiver, const char *name) {
  return ((id (*)(id, SEL))objc_msgSend)(receiver, sel_registerName(name));
}

static CGRect rect(id receiver, const char *name) {
#if defined(__x86_64__)
  // CGRect is a large struct: Intel uses the struct-return ABI.
  CGRect value;
  ((void (*)(CGRect *, id, SEL))objc_msgSend_stret)(
    &value, receiver, sel_registerName(name));
  return value;
#else
  return ((CGRect (*)(id, SEL))objc_msgSend)(receiver, sel_registerName(name));
#endif
}

static void style(id window, unsigned long value) {
  ((void (*)(id, SEL, unsigned long))objc_msgSend)(
    window, sel_registerName("setStyleMask:"), value);
}

static void frame(id window, CGRect value) {
  ((void (*)(id, SEL, CGRect, BOOL))objc_msgSend)(
    window, sel_registerName("setFrame:display:"), value, YES);
}

static void configure_on_main(void *context) {
  bool *ok = context;
  id app = object((id)objc_getClass("NSApplication"), "sharedApplication");
  id window = object(app, "keyWindow");
  if (!window) window = object(app, "mainWindow");
  // Deliberately limited to a single-window demo; never guess another window.
  if (!window) return;
  id screen = object(window, "screen");
  if (!screen) screen = object((id)objc_getClass("NSScreen"), "mainScreen");
  if (!screen) return;
  CGRect band = rect(screen, "visibleFrame");
  if (CGRectIsEmpty(band)) return;

  saved_window = object(window, "retain");
  saved_frame = rect(window, "frame");
  saved_style = ((unsigned long (*)(id, SEL))objc_msgSend)(
    window, sel_registerName("styleMask"));
  band.size.height /= 3.0; // Cocoa points; keep work-area origin and full width.
  style(window, 0UL);     // NSWindowStyleMaskBorderless
  frame(window, band);
  *ok = true;
}

static void restore_on_main(void *context) {
  (void)context;
  if (!saved_window) return;
  style(saved_window, saved_style);
  frame(saved_window, saved_frame);
  ((void (*)(id, SEL))objc_msgSend)(saved_window, sel_registerName("release"));
  saved_window = nil;
}

static void on_main(void *context, dispatch_function_t fn) {
  if (pthread_main_np()) fn(context);
  else dispatch_sync_f(dispatch_get_main_queue(), context, fn);
}

static mrb_value configure(mrb_state *mrb, mrb_value self) {
  (void)mrb; (void)self;
  // C state survives Ruby state resets caused by native window resizing.
  if (configured || stopped) return mrb_true_value();
  bool ok = false;
  on_main(&ok, configure_on_main);
  configured = ok;
  return ok ? mrb_true_value() : mrb_false_value();
}

static mrb_value restore(mrb_state *mrb, mrb_value self) {
  (void)mrb; (void)self;
  stopped = true; // Do not reapply after the restore triggers another resize.
  on_main(NULL, restore_on_main);
  configured = false;
  return mrb_true_value();
}

DRB_FFI_EXPORT
void drb_register_c_extensions_with_api(mrb_state *mrb, struct drb_api_t *api) {
  drb = api;
  struct RClass *mod = drb->mrb_define_module(mrb, "BottomBand");
  drb->mrb_define_module_function(mrb, mod, "configure", configure, MRB_ARGS_NONE());
  drb->mrb_define_module_function(mrb, mod, "restore", restore, MRB_ARGS_NONE());
}

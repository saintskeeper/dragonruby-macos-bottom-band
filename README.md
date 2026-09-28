# Minimal macOS bottom-band window — DragonRuby 7.16

I use a native extension to make DragonRuby's existing window borderless and size
it to the bottom third of the current display's usable desktop. This is a stripped
down demonstration, not my game's full adapter. It needs no art or game code.

## Build and run

Requires macOS, Xcode Command Line Tools, and DragonRuby 7.16 with C extension
support and `include/dragonruby.h` (Pro). Put these files in a new game directory:

```text
bottom-band-demo/
  bottom_band.c
  app/
    main.rb
  native/
    macos/              # compiled library goes here
```

From that directory, compile for the architecture of the DragonRuby process:

```sh
SDK="/absolute/path/to/dragonruby-macos-7.16"
ARCH=arm64 # use x86_64 for Intel or a DragonRuby process running under Rosetta
mkdir -p native/macos
clang -x c -std=c11 -fPIC -dynamiclib -arch "$ARCH" \
  -isysroot "$(xcrun --show-sdk-path)" -I "$SDK/include" \
  bottom_band.c -framework AppKit -framework Foundation -framework CoreGraphics \
  -o native/macos/bottom_band.dylib
"$SDK/dragonruby" "$PWD"
```

The window should become a borderless bottom band. R restores its original
style and frame; restart the process to apply the band again. Native state prevents
Ruby resize resets from repeatedly changing the window. A missing window/screen
raises an error rather than silently claiming success.

This intentionally leaves stacking level, opacity, shadow, and rendering behavior
unchanged. It does not make an always-on-top window or implement transparency.
The only render output is an opaque background after the native update.

## Which native objects supply the dimensions?

These are Objective-C/AppKit objects accessed from C through `objc_msgSend`, not
DragonRuby structs:

| Object / API | What I need it for |
| --- | --- |
| `NSApplication.sharedApplication` → `keyWindow` / `mainWindow` | Locate the existing game window in this single-window demo. |
| `NSWindow.screen` | Find the display containing that window; fall back to `NSScreen.mainScreen` if absent. |
| `NSScreen.visibleFrame` | Get usable desktop **origin and size**, accounting for the Dock and menu bar. |
| `CGRect.origin.x/y`, `CGRect.size.width/height` | Preserve the usable origin and full width; divide usable height by three. |
| `NSWindow.frame`, `styleMask` | Save the original frame and decorations for restoration. |
| `NSWindow.setFrame:display:`, `setStyleMask:` | Apply the new native frame and borderless style. |

The sizing formula is simply:

```text
band = screen.visibleFrame
band.height = band.height / 3
```

Cocoa screen/window frames are in **points**, with a bottom-left origin. Keeping
`visibleFrame.origin` matters on secondary displays and when the Dock occupies
part of the desktop. Do not assume `(0, 0)` is the usable area's origin.

`NSScreen.backingScaleFactor` is relevant when converting backing pixels to points;
I do **not** need it here because both the input and output rectangles are already
in points. DragonRuby logical canvas pixels and `grid.allscreen_*` are a different
coordinate space—not a substitute for the desktop work area.

AppKit calls execute on the main thread via `dispatch_sync_f`. The C message helper
also distinguishes Intel's `objc_msgSend_stret` convention for returning a `CGRect`
from Apple Silicon's normal `objc_msgSend` convention.

## Why native control?

The 7.16 SDK's `docs/api/runtime.md` documents `DR.set_window_size` and
`DR.set_window_position`, but explicitly marks them development/debugging-only,
not production features. My use case also needs native borderless styling and the
usable desktop bounds. The extension provides those together.

In my game, empty framebuffer areas appeared black rather than exposing the
desktop. A physically smaller window avoids depending on per-pixel transparency:
the desktop above the band is visible because no game window occupies it. This
example does not diagnose or change the renderer's alpha behavior.

## Native references

- [NSApplication](https://developer.apple.com/documentation/appkit/nsapplication)
- [NSWindow](https://developer.apple.com/documentation/appkit/nswindow)
- [NSScreen](https://developer.apple.com/documentation/appkit/nsscreen)
- [NSScreen.visibleFrame](https://developer.apple.com/documentation/appkit/nsscreen/visibleframe)
- [NSScreen.backingScaleFactor](https://developer.apple.com/documentation/appkit/nsscreen/backingscalefactor)
- [CGRect](https://developer.apple.com/documentation/corefoundation/cgrect)
- [Objective-C runtime](https://developer.apple.com/documentation/objectivec)

## Scope and validation

This assumes one ordinary game window, not fullscreen, multiple engine windows,
or ongoing display/Dock changes. It does not track window replacement or promise
focus/click-through behavior. It is a minimal reproduction, not a hardened adapter.
Compilation and Ruby syntax can be checked independently of live window behavior;
neither proves correct placement or R-key handling in a running engine.

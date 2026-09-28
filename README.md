# Minimal macOS bottom-band window — DragonRuby 7.16

I wanted my harbor game to sit along the bottom of the desktop. This is the
smallest example of the C extension I used: take the existing DragonRuby window,
remove the border, and size it to the bottom third of the usable screen.

## Quick start

Requires macOS, Xcode Command Line Tools, and DragonRuby 7.16 Pro with C
extension support (`include/dragonruby.h`).

```sh
git clone https://github.com/saintskeeper/dragonruby-macos-bottom-band.git
cd dragonruby-macos-bottom-band
make run SDK=/absolute/path/to/dragonruby-macos-7.16
```

`make build` creates `native/macos/bottom_band.dylib`; `make clean` removes it.
`SDK` defaults to `.sdk`, and `ARCH` defaults to `uname -m`. Override `ARCH`
when the engine process differs from the host, such as Rosetta:

```sh
make run SDK=/absolute/path/to/dragonruby-macos-7.16 ARCH=x86_64
```

The extension architecture must match DragonRuby. Press **R** to restore the
original frame and window style; restart DragonRuby to apply the band again.

## What the extension asks AppKit for

| Native object / API | Why it is needed |
| --- | --- |
| [`NSApplication`](https://developer.apple.com/documentation/appkit/nsapplication) `keyWindow` / `mainWindow` | Find the existing game window. |
| [`NSWindow`](https://developer.apple.com/documentation/appkit/nswindow) `screen`, `frame`, `setFrame:display:`, `styleMask`, `setStyleMask:` | Find its display, save the original frame and style, then resize it and remove the border. |
| [`NSScreen.visibleFrame`](https://developer.apple.com/documentation/appkit/nsscreen/visibleframe) | Get the usable work area—its origin and size already account for the Dock and menu bar. |
| [`CGRect`](https://developer.apple.com/documentation/corefoundation/cgrect) | Keep the work area's origin and width, then divide its height by three. |

The frame is AppKit/Cocoa **points** with a bottom-left origin, not DragonRuby
logical canvas pixels or backing pixels. `grid.allscreen_*` is therefore not a
desktop-coordinate substitute. `backingScaleFactor` matters only when
converting pixels to points; this example does not need that conversion.

AppKit work runs on the main thread. The C helper also uses the correct CGRect
return convention for Intel versus Apple Silicon.

## Why native control

DragonRuby 7.16 has `DR.set_window_size` and `DR.set_window_position`, but its
SDK runtime docs mark them development/debugging-only. This needs both
borderless styling and the display work area, so the extension uses AppKit.

I also tried leaving the top of the game transparent, but it showed up black.
So I made the actual window smaller instead. The desktop above it is uncovered,
not showing through the game. This example doesn't change the renderer.

## Limits and validation

This is a single ordinary window example: it does not track display, Dock, or
window replacement changes, or promise fullscreen, focus, and click-through
behavior. It was compiled and run on Apple Silicon; the geometry was verified
there and the demo looked good in playtest. R-key handling has not been live
verified.

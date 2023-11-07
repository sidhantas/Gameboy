#include "graphics.h"
#include <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>
#include <Foundation/Foundation.h>
#include <stdio.h>

@interface GameboyDisplay : NSView
@end

@implementation GameboyDisplay

- (void)drawRect:(NSRect)dirtyRect {
    NSRect rect = NSMakeRect(50, 50, 200,
                             100); // Specify the rectangle's position and size
    NSRectFill(rect);              // Fill the rectangle with the specified colo
}

@end

@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation AppDelegate {
    GameboyDisplay *display;
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    NSRect windowRect = NSMakeRect(100, 100, 400, 400);
    NSWindow *window =
        [[NSWindow alloc] initWithContentRect:windowRect
                                    styleMask:(NSWindowStyleMaskTitled |
                                               NSWindowStyleMaskClosable |
                                               NSWindowStyleMaskResizable)
                                      backing:NSBackingStoreBuffered
                                        defer:NO];

    [window setTitle:@"Gameboy"];
    [window orderFrontRegardless];
    self->display = [[GameboyDisplay alloc] initWithFrame:windowRect];
    [window setContentView:self->display];


}
@end

void *open_window() {
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        AppDelegate *delegate = [[AppDelegate alloc] init];
        [app setDelegate:delegate];
        [app run];
    }
    return NULL;
}

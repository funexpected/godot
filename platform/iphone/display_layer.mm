/*************************************************************************/
/*  display_layer.mm                                                     */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#import "display_layer.h"

#include "core/os/keyboard.h"
#include "core/project_settings.h"
#include "main/main.h"
#include "os_iphone.h"
#include "servers/audio_server.h"

#import <AudioToolbox/AudioServices.h>
#import <GameController/GameController.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>

int gl_view_base_fb;
bool gles3_available = true;

@implementation GodotOpenGLLayer {
	// The pixel dimensions of the backbuffer
	GLint backingWidth;
	GLint backingHeight;

	EAGLContext *context;
	GLuint viewRenderbuffer, viewFramebuffer;
	GLuint depthRenderbuffer;
}

- (void)initializeDisplayLayer {
	// Configure it so that it is opaque, does not retain the contents of the backbuffer when displayed, and uses RGBA8888 color.
	self.opaque = YES;
	self.drawableProperties = [NSDictionary
			dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:FALSE],
			kEAGLDrawablePropertyRetainedBacking,
			kEAGLColorFormatRGBA8,
			kEAGLDrawablePropertyColorFormat,
			nil];
	bool fallback_gl2 = false;
	// Create a GL ES 3 context based on the gl driver from project settings
	if (GLOBAL_GET("rendering/quality/driver/driver_name") == "GLES3") {
		context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
		NSLog(@"Setting up an OpenGL ES 3.0 context. Based on Project Settings \"rendering/quality/driver/driver_name\"");
		if (!context && GLOBAL_GET("rendering/quality/driver/fallback_to_gles2")) {
			gles3_available = false;
			fallback_gl2 = true;
			NSLog(@"Failed to create OpenGL ES 3.0 context. Falling back to OpenGL ES 2.0");
		}
	}

	// Create GL ES 2 context
	if (GLOBAL_GET("rendering/quality/driver/driver_name") == "GLES2" || fallback_gl2) {
		context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
		NSLog(@"Setting up an OpenGL ES 2.0 context.");
		if (!context) {
			NSLog(@"Failed to create OpenGL ES 2.0 context!");
			return;
		}
	}

	if (![EAGLContext setCurrentContext:context]) {
		NSLog(@"Failed to set EAGLContext!");
		return;
	}
	if (![self createFramebuffer]) {
		NSLog(@"Failed to create frame buffer!");
		return;
	}
}

- (void)layoutDisplayLayer {
	NSLog(@"[GODOT_ORIENTATION] DisplayLayer layoutDisplayLayer");
	NSLog(@"[GODOT_ORIENTATION] Display layer bounds: %@", NSStringFromCGRect(self.bounds));
	[EAGLContext setCurrentContext:context];
	[self destroyFramebuffer];
	[self createFramebuffer];
	NSLog(@"[GODOT_ORIENTATION] Framebuffer recreated");
}

- (void)startRenderDisplayLayer {
	[EAGLContext setCurrentContext:context];

	glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
}

- (void)stopRenderDisplayLayer {
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
	[context presentRenderbuffer:GL_RENDERBUFFER_OES];

#ifdef DEBUG_ENABLED
	GLenum err = glGetError();
	if (err) {
		NSLog(@"DrawView: %x error", err);
	}
#endif
}

- (void)dealloc {
	if ([EAGLContext currentContext] == context) {
		[EAGLContext setCurrentContext:nil];
	}

	if (context) {
		context = nil;
	}
}

- (BOOL)createFramebuffer {
	// Generate IDs for a framebuffer object and a color renderbuffer
	glGenFramebuffersOES(1, &viewFramebuffer);
	glGenRenderbuffersOES(1, &viewRenderbuffer);

	glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
	// This call associates the storage for the current render buffer with the EAGLDrawable (our CAEAGLLayer)
	// allowing us to draw into a buffer that will later be rendered to screen wherever the layer is (which corresponds with our view).
	[context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(id<EAGLDrawable>)self];
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, viewRenderbuffer);

	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
	
	NSLog(@"[GODOT_ORIENTATION] createFramebuffer: Got backing size from GL: %dx%d", backingWidth, backingHeight);

	// For this sample, we also need a depth buffer, so we'll create and attach one via another renderbuffer.
	glGenRenderbuffersOES(1, &depthRenderbuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
	glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, backingWidth, backingHeight);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);

	if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
		NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
		return NO;
	}

	if (OS::get_singleton()) {
		OS::VideoMode vm;
		vm.fullscreen = true;
		vm.width = backingWidth;
		vm.height = backingHeight;
		vm.resizable = false;
		NSLog(@"[GODOT_ORIENTATION] createFramebuffer: Setting VideoMode to %dx%d", vm.width, vm.height);
		
		// Store old size to detect change
		Size2 old_size = OS::get_singleton()->get_window_size();
		NSLog(@"[GODOT_ORIENTATION] createFramebuffer: Old window size: %dx%d", (int)old_size.width, (int)old_size.height);
		
		OS::get_singleton()->set_video_mode(vm);
		OSIPhone::get_singleton()->set_base_framebuffer(viewFramebuffer);
		
		Size2 new_size = OS::get_singleton()->get_window_size();
		NSLog(@"[GODOT_ORIENTATION] createFramebuffer: New window size: %dx%d", (int)new_size.width, (int)new_size.height);
		NSLog(@"[GODOT_ORIENTATION] createFramebuffer: VideoMode updated and framebuffer set");
		
		// CRITICAL: Force rendering system to update for new size
		// Schedule the update for the next frame to ensure proper timing
		dispatch_async(dispatch_get_main_queue(), ^{
			MainLoop *ml = OS::get_singleton()->get_main_loop();
			if (ml) {
				NSLog(@"[GODOT_ORIENTATION] createFramebuffer: Forcing scene tree update");
				
				// The SceneTree checks window size in its idle() method
				// Force an idle cycle to trigger screen_resized signal
				if (!Main::is_iterating()) {
					ml->idle(0.0);
				}
				
				Main::force_redraw();
				NSLog(@"[GODOT_ORIENTATION] createFramebuffer: Scene tree updated");
			}
		});
	} else {
		NSLog(@"[GODOT_ORIENTATION] createFramebuffer: WARNING - No OS singleton!");
	}

	gl_view_base_fb = viewFramebuffer;

	return YES;
}

// Clean up any buffers we have allocated.
- (void)destroyFramebuffer {
	glDeleteFramebuffersOES(1, &viewFramebuffer);
	viewFramebuffer = 0;
	glDeleteRenderbuffersOES(1, &viewRenderbuffer);
	viewRenderbuffer = 0;

	if (depthRenderbuffer) {
		glDeleteRenderbuffersOES(1, &depthRenderbuffer);
		depthRenderbuffer = 0;
	}
}

@end

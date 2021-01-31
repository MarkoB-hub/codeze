#include "modes.h"
#include "buffer.h"
#include "window.h"
#include "cursor.h"
#include "bind.h"


static void
on_event(Event& event) {

	if (event.type == KEY_PRESSED ||
		event.type == KEY_REPEAT) {
		handle_key((KeyCode)event.key, event.mods);
	}
}

static void
on_init() {
	
}

static void
update() {
	
}

static void
on_start() {
	
}

static void
on_end() {
	
}


const EditorModeOps NavigationModeOps = {
	on_event,
	update,
	on_init,
	on_start,
	on_end
};

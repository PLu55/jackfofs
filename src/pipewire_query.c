#ifdef HAS_PIPEWIRE

#include <pipewire/pipewire.h>

#include <pipewire/pipewire.h>
#include <stdio.h>

char* pipewire_query(char* query)
{
    // Initialize PipeWire
    pw_init(&argc, &argv);

    // Create a PipeWire core object
    struct pw_core *core = pw_core_new(NULL, NULL, 0);
    if (!core) {
        fprintf(stderr, "pipewire_query: Failed to create PipeWire core\n");
        return -1;
    }

    // Get the properties of the core
    const struct pw_properties *props = pw_core_get_properties(core);
    if (!props) {
        fprintf(stderr, "pipewire_query: Failed to get core properties\n");
        pw_core_destroy(core);
        return -1;
    }

    // Get the clock.max-quantum property
    const char *reply = pw_properties_get(props, query);

    // Clean up
    pw_core_destroy(core);
    pw_deinit();

    return reply;
}

#endif
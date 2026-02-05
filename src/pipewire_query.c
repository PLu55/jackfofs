#ifdef HAS_PIPEWIRE

#include <pipewire/pipewire.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *pipewire_query(const char *query)
{
    pw_init(NULL, NULL);

    struct pw_core *core = pw_core_new(NULL, NULL, 0);
    if (!core)
    {
        fprintf(stderr, "pipewire_query: Failed to create PipeWire core\n");
        pw_deinit();
        return NULL;
    }

    const struct pw_properties *props = pw_core_get_properties(core);
    if (!props)
    {
        fprintf(stderr, "pipewire_query: Failed to get core properties\n");
        pw_core_destroy(core);
        pw_deinit();
        return NULL;
    }

    const char *value = pw_properties_get(props, query);
    char *reply = value ? strdup(value) : NULL;

    pw_core_destroy(core);
    pw_deinit();

    return reply;
}

#else

typedef int pipewire_query_translation_unit_nonempty;

#endif
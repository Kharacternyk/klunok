#include "messages.h"
#include "constants.h"

#define BARE_VERSION_MESSAGE "Klunok " VERSION ", Lua " LUA_VERSION
#define USAGE "klunok [-h | -v | -c PATH | -d PATH | -w PATH | -e PATH]..."

#ifdef WATCH_NIX_STORE
#define VERSION_MESSAGE BARE_VERSION_MESSAGE ", compiled with Nix support"
#else
#define VERSION_MESSAGE BARE_VERSION_MESSAGE
#endif

const struct translation messages = {
    .trace =
        {
            .because_of = "because of",
            .which_is = "which is",
            .message_dropped = "message dropped",
        },
    .timestamp =
        {
            .overflow = "A version string exceeds the size limit",
        },
    .parents =
        {
            .cannot_remove_ancestor = "Cannot remove an ancestor directory",
            .cannot_create_ancestor = "Cannot create an ancestor directory",
        },
    .sync =
        {
            .source_is_not_regular_file =
                "Cannot copy a file that is not a regular file",
            .source_does_not_exist = "The file does not exist",
            .source_permission_denied = "Permission denied",
            .destination_already_exists = "The destination already exists",
        },
    .config =
        {
            .is_static = "Configuration files are not supported",
        },
    .linq =
        {
            .invalid_entry = "The queue contains an invalid entry",
        },
    .params =
        {
            .unknown_option = "An unknown option has been passed",
            .stray_option =
                "An option without a required value has been passed",
            .redefined_option = "An option has been passed more than once, but "
                                "it cannot have multiple values",
        },
    .handler =
        {
            .version =
                {
                    .has_slashes = "Slash characters found in a version string",
                },
            .config =
                {
                    .cannot_load = "Cannot load the configuration file",
                    .cannot_reload = "Cannot reload the configuration file",
                },
            .linq =
                {
                    .cannot_load = "Cannot load the debouncing link queue",
                    .cannot_reload = "Cannot reload the debouncing link queue",
                    .cannot_get_head =
                        "Cannot get the head of the debouncing link queue",
                    .cannot_push =
                        "Cannot push a file to the debouncing link queue",
                },
            .store =
                {
                    .cannot_copy = "Cannot copy a file to the store",
                },
            .journal = {.cannot_open = "Cannot open the journal",
                        .cannot_write_to = "Cannot write to the journal"},
        },
    .main =
        {
            .cannot_bootstrap = "Cannot bootstrap",
            .out_of_memory = "There is not enough memory",
            .cli_usage_violated =
                "Some of the command line arguments do not fit the usage",
            .cannot_parse_cli = "Cannot parse the command line arguments",
            .cannot_drop_privileges =
                "Cannot drop privileges by assuming identity of "
                "the owner of a path",
            .cannot_load_handler = "Cannot load the event handler",
            .cannot_handle_exec = "Cannot handle a file execution event",
            .cannot_handle_write = "Cannot handle a file write event",
            .cannot_handle_timeout = "Cannot handle the periodical tasks",
            .fanotify = {.cannot_init = "Cannot initialize fanotify",
                         .cannot_poll =
                             "Cannot poll the fanotify file descriptor",
                         .cannot_read_event = "Cannot read a fanotify event",
                         .version_mismatch =
                             "The version of a fanotify event is unsupported",
                         .queue_overflow =
                             "The fanotify event queue has overflowed"},
            .mount =
                {
                    .cannot_list = "Cannot list mount points",
                    .cannot_watch = "Cannot watch a mount point",
                },
            .info =
                {
                    .version = VERSION_MESSAGE,
                    .usage = USAGE,
                    .help = VERSION_MESSAGE "\nUsage: " USAGE,
                },
        },
};

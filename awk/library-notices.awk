BEGIN {
    for (i = 1; i < ARGC; i++) {
        split(ARGV[i], parts, "-")
        libraries = libraries (i > 1 ? ", " : "") parts[1]
    }
    release = "https://github.com/Kharacternyk/klunok/releases/download/v" release_version
    print "Klunok statically links these libraries: " libraries "."
    print "Build recipes and patches: https://github.com/NixOS/nixpkgs/archive/" nixpkgs_revision ".zip\n"
}
BEGINFILE {
    split(FILENAME, parts, "/")
    directory = parts[1]
    name = directory
    sub(/-.*/, "", name)
    version = substr(directory, length(name) + 2)
    archive = directory (name == "glibc" ? ".tar.xz" : ".tar.gz")
    print name " " version
}
name == "lua" && /^\*.*Copyright \(C\)/ { first = FNR }
name == "lua" && index($0, "*/") { first = 0 }
name == "lua" && first {
    sub(/^\* ?/, "")
    print
    if (FNR == first) {
        print "Source code: " release "/" archive
    }
}
name == "libseccomp" && sub(/^ \* Copyright /, "Copyright ") {
    print
}
name == "glibc" && FNR == 1 && sub(/^\/\* /, "") {
    print
}
ENDFILE {
    if (name != "lua") {
        later = name == "glibc" ? " or later" : ""
        print "Licensed under GNU Lesser General Public License, version 2.1" later " (see below)."
        print "Source code: " release "/" archive
    }
    print ""
}

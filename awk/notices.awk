BEGIN {
    source_url = "https://github.com/Kharacternyk/klunok"
    if (source_revision != "") {
        source_url = source_url "/tree/" source_revision
    }
    print "Klunok is Copyright (C) 2022-2026 Klunok authors"
    print "Licensed under GNU General Public License, version 3 (see below)."
    print "Source code: " source_url
}
FNR == 1 {
    print ""
}
{
    print
}

BEGIN {
    RS = "\n---\n"
    FS = "\n--\n"
}
{
    if (NF > 1) {
        if ($1 == "--post--") {
            printf "%s", $2 > post
        } else if ($1 == "--pre--") {
            printf "%s", $2 > pre
        } else {
            gsub("\n", " ", $2)
            gsub("\n", " ", $3)
            gsub("\n", " ", $4)

            printf "%s = %s\n", $1, $4 > pre
            printf "declare('%s', %s, %s)\n", $1, $2, $3 > post
            printf "\n--\n-- * Type: `%s`\n", $2 > doc
            if ($3 != "nil") {
                printf "-- * Reactive default: `%s`\n", $3 > doc
            } else {
                printf "-- * Default: `%s`\n", $4 > doc
            }
            printf "%s = nil\n", $1 > doc
        }
    } else {
        printf "%s", $0 > doc
    }
}
